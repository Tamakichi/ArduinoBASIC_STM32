#include "host.h"
#include "basic.h"

#include "src/lib/tscreenBase.h"  // コンソール基本
#include "src/lib/tTermscreen.h"  // シリアルコンソール

int16_t getNextLineNo(int16_t lineno);
char* getLineStr(int16_t lineno);
int16_t getPrevLineNo(int16_t lineno);
void mem_putch(uint8_t c);

#define SIZE_LINE MAXTEXTLEN    // コマンドライン入力バッファサイズ + NULL
void initScreenEnv();
tscreenBase* sc;   // 利用デバイススクリーン用ポインタ
tTermscreen sc1;   // ターミナルスクリーン
uint8_t* workarea = NULL;           // 画面用動的獲得メモリ
uint8_t  serialMode = DEF_SMODE;    // シリアルモード(0:USB、1:USART)
uint32_t defbaud = GPIO_S1_BAUD;    // シリアルボーレート

#if !defined (__STM32F1__)
#include <EEPROM.h>
#endif

//extern SSD1306ASCII oled;
//extern PS2Keyboard keyboard;
#if !defined (__STM32F1__)
extern EEPROMClass EEPROM;
#endif
//int timer1_counter;

// シリアルコンソール 制御キーコード
#define PS2_DELETE  0x7f
#define PS2_BS      0x08
#define PS2_ENTER   0x0d
#define PS2_ESC     0x1b
#define PS2_CTRL_C  0x03
#define VT100_CLS   "\x1b[2J"

// *** フラッシュメモリ管理 ***********

// フラッシュメモリ管理オブジェクト(プログラム保存、システム環境設定を管理）
tFlashMan FlashMan(FLASH_PAGE_NUM,FLASH_PAGE_SIZE, FLASH_SAVE_NUM, FLASH_PAGE_PAR_PRG); 

char tbuf[SIZE_LINE];          // テキスト表示用バッファ
int16_t tbuf_pos = 0;

// メモリへの文字出力
void mem_putch(uint8_t c) {
  if (tbuf_pos < SIZE_LINE) {
   tbuf[tbuf_pos] = c;
   tbuf_pos++;
  }
}

// メモリ書き込みポインタのクリア
void cleartbuf() {
  tbuf_pos=0;
  memset(tbuf,0,SIZE_LINE);
}

char* gettbuf() {
  return tbuf;
}

// 文字入出力
#define c_getch()       sc->get_ch()
#define c_kbhit( )      sc->isKeyIn()
#define c_cls()         sc->cls()

// 指定デバイスへの文字の出力
//  c     : 出力文字
//  devno : デバイス番号
void c_putch(uint8_t c, uint8_t devno=CDEV_SCREEN) {
  if (devno == CDEV_SCREEN )
    sc->putch(c); // メインスクリーンへの文字出力
  else if (devno == CDEV_MEMORY)
   mem_putch(c); // メモリーへの文字列出力
} 

// 改行
//  devno : デバイス番号
void c_newLine(uint8_t  devno=CDEV_SCREEN) {
 if (devno== CDEV_SCREEN )
   sc->newLine();        // メインスクリーンへの文字出力
  else if (devno == CDEV_MEMORY )
    mem_putch('\n');     // メモリーへの文字列出力   
}

char lineBuffer[MAXTEXTLEN];    // ラインバッファ
char inputMode = 0;
char inkeyChar = 0;
#if !defined (__STM32F1__)
char buzPin = 0;
#endif

const char bytesFreeStr[] PROGMEM = "bytes free";

// 文字列の右側の空白文字を削除する
char* tlimR(char* str) {
  uint16_t len = strlen(str);
  for (uint16_t i = len - 1; i>0 ; i--) {
    if (str[i] == ' ') {
      str[i] = 0;
    } else {
      break;
    }
  }
  return str;
}

// ホスト画面の初期化
void host_init(int buzzerPin) {

//  oled.clear();
#if !defined (__STM32F1__)
    buzPin = buzzerPin;
    if (buzPin)
        pinMode(buzPin, OUTPUT);
#endif
//  initTimer();

  workarea = (uint8_t*)malloc(5760); // SCREEN0で128x45まで
  sc = &sc1;
  ((tTermscreen*)sc)->init(TERM_W,TERM_H,SIZE_LINE, workarea); // スクリーン初期設定  
  sc->Serial_mode(serialMode, defbaud); // デバイススクリーンのシリアル出力の設定
}

// スリープ
void host_sleep(long ms) {
  delay(ms);
}

// デジタル出力
void host_digitalWrite(int pin,int state) {
  digitalWrite(pin, state ? HIGH : LOW);
}

// デジタル入力
int host_digitalRead(int pin) {
  return digitalRead(pin);
}

// アナログ入力
int host_analogRead(int pin) {
  return analogRead(pin);
}

// ピンモードの設定
void host_pinMode(int pin, WiringPinMode mode) {
  pinMode(pin, mode);
}

#if !defined (__STM32F1__)
void host_click() {
    if (!buzPin) return;
    digitalWrite(buzPin, HIGH);
    delay(1);
    digitalWrite(buzPin, LOW);
}

void host_startupTone() {
    if (!buzPin) return;
    for (int i=1; i<=2; i++) {
        for (int j=0; j<50*i; j++) {
            digitalWrite(buzPin, HIGH);
            delay(3-i);
            digitalWrite(buzPin, LOW);
            delay(3-i);
        }
        delay(100);
    }    
}
#endif

// スクリーンクリア
void host_cls() {
    c_cls();
    host_moveCursor(0,0);
//  curY = 0;
}

// カーソル移動
void host_moveCursor(int x, int y) {
 sc->locate((uint16_t)x, (uint16_t)y);
}

// 画面バッファに文字列を出力
void host_outputString(char *str, uint8_t devno) {
    while (*str) {
        c_putch(*str,devno);
        str++;
    }
}

// フラシュメモリ上の文字列を出力
void host_outputProgMemString(const char *p, uint8_t devno) {
    while (1) {
      unsigned char c = *p++;
      if (c == 0) break;
      host_outputChar(c,devno);
    }
}

// 文字を出力
void host_outputChar(char c, uint8_t devno) {
    c_putch(c,devno);
}

// 数値を出力
int host_outputInt(long num, uint8_t devno) {
  // returns len
  long i = num, xx = 1;
  int c = 0;
  do {
    c++;
    xx *= 10;
    i /= 10;
  } 
  while (i);

  for (int i=0; i<c; i++) {
    xx /= 10;
    char digit = ((num/xx) % 10) + '0';
    host_outputChar(digit,devno);
  }
  return c;
}

// フロート型を文字列変換
char *host_floatToStr(float f, char *buf) {
  // floats have approx 7 sig figs
  float a = fabs(f);
  if (f == 0.0f) {
    buf[0] = '0'; 
    buf[1] = 0;
  } else if (a<0.0001 || a>1000000) {
    // this will output -1.123456E99 = 13 characters max including trailing nul
#if defined(__AVR__) 
   dtostre(f, buf, 6, 0);
#else
   sprintf(buf, "%6.2e", f);
#endif
  } else {
    int decPos = 7 - (int)(floor(log10(a))+1.0f);
    dtostrf(f, 1, decPos, buf);
    if (decPos) {
      // remove trailing 0s
      char *p = buf;
      while (*p) p++;
      p--;
      while (*p == '0') {
          *p-- = 0;
      }
      if (*p == '.') *p = 0;
    }   
  }
  return buf;
}

// フロート型数値出力
void host_outputFloat(float f,uint8_t  devno) {
  char buf[16];
  host_outputString(host_floatToStr(f, buf),devno);
}

// 改行
void host_newLine(uint8_t  devno) {
  c_newLine(devno);
}

// 行入力
uint16_t host_input() {
    uint16_t rc = sc->editLine();
     sc->show_curs(true);
    return rc; 
}

// 行入力テキスト参照
char *host_getInputText() {
    return (char*)sc->getText(); // スクリーンバッファからテキスト取得 
}

// ライン入力
char *host_readLine() {
    uint8_t rc;        // 関数戻り値受け取り用
    char* textline;    // 入力行

    while (1) { //無限ループ
        rc = sc->edit();  // エディタ入力
        if (rc) {
            textline = (char*)sc->getText(); // スクリーンバッファからテキスト取得
            if (!strlen(textline) ) {
              // 改行のみ
              c_newLine();
              continue;
            }

            if (strlen(textline) >= SIZE_LINE) {
              // 入力文字が有効文字長を超えている
               //err = ERR_LONG;
               c_newLine();
               //error();
               continue;  
            }
            // 行バッファに格納し、改行する
            strcpy(lineBuffer, textline);
            tlimR((char*)lineBuffer); //文末の余分空白文字の削除        
         } else {
            // 入力なし
            continue;
        }
        break;
      }
    return textline;
}

uint8_t prevPressKey = 0;   // 直前入力キーの値(INKEY()、[ESC]中断キー競合防止用)

char host_getKey() {
  int16_t rc = 0;
  
  if (prevPressKey) {
    // 一時バッファに入力済キーがあればそれを使う
    rc = prevPressKey;
    prevPressKey = 0;
  } else if (c_kbhit( )) {
    // キー入力
    rc = c_getch();
  }
  return rc;  
}

// 中断キー入力チェック
bool host_ESCPressed() {
    uint8_t c = c_kbhit();
    if (c) {
        if (c == PS2_CTRL_C || c==PS2_ESC ) { // 読み込んでもし[ESC],［CTRL_C］キーだったら
            prevPressKey = 0;
            return true;
        } else {
            prevPressKey = c;        
        }
     }
     return false;
}

// 空き領域の表示
void host_outputFreeMem(unsigned int val) {
  host_newLine(CDEV_SCREEN);
  host_outputInt(val,CDEV_SCREEN);
  host_outputChar(' ',CDEV_SCREEN);
  host_outputProgMemString(bytesFreeStr,CDEV_SCREEN);      
}

void host_saveProgram(bool autoexec,int16_t flleNo) {

  // プログラムsysPROGENDの保存
  mem[MEMORY_SIZE-2] = sysPROGEND & 0xFF;
  mem[MEMORY_SIZE-1] = (sysPROGEND >> 8) & 0xFF;
  
  FlashMan.saveProgram(flleNo, mem);  
}

void host_loadProgram(int16_t flleNo) {
  FlashMan.loadProgram(flleNo, mem);

  // プログラムsysPROGENDのセット
  sysPROGEND = mem[MEMORY_SIZE-2] | (mem[MEMORY_SIZE-1] << 8);

}
