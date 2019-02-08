#include <Arduino.h>
#include <stdint.h>

// ** (1)起動時コンソール画面指定 0:シリアルターミナル
#define USE_SCREEN_MODE 0  // 現バージョンでは必ず0を指定

// ** ターミナルモード時のデフォルト スクリーンサイズ  *************************
// ※ 可動中では、WIDTHコマンドで変更可能  (デフォルト:80x24)
#define TERM_W       80
#define TERM_H       24

// ** Serial1のデフォルト通信速度 *********************************************
#define GPIO_S1_BAUD    115200 // // (デフォルト:115200)

// ** デフォルトのターミナル用シリアルポートの指定 0:USBシリアル 1:GPIO UART1
// ※ 可動中では、SMODEコマンドで変更可能
#define DEF_SMODE     0 // (デフォルト:0) 現バージョンでは必ず0を指定

#define MAGIC_AUTORUN_NUMBER    0xFC

#define MAXTEXTLEN          128  // 1行の最大文字数
#if !defined (__STM32F1__)
#define WiringPinMode  int
#endif

#define FLASH_PAGE_NUM         128     // 全ページ数
#define FLASH_PAGE_SIZE        1024    // ページ内バイト数
#define FLASH_PAGE_PAR_PRG     4       // 1プログラム当たりの利用ページ数
#define FLASH_SAVE_NUM         6       // プログラム保存可能数

// 入出力キャラクターデバイス
#define CDEV_SCREEN   0  // メインスクリーン
#define CDEV_SERIAL   1  // シリアル
#define CDEV_GSCREEN  2  // グラフィック
#define CDEV_MEMORY   3  // メモリー
#define CDEV_SDFILES  4  // ファイル

char * gettbuf();
void cleartbuf();
void c_putch(uint8_t c, uint8_t devno);
void host_init(int buzzerPin);
void host_sleep(long ms);
void host_digitalWrite(int pin,int state);
int host_digitalRead(int pin);
int host_analogRead(int pin);
void host_pinMode(int pin, WiringPinMode mode);

void host_cls();
void host_showBuffer();
void host_moveCursor(int x, int y);
void host_outputString(char *str, uint8_t devno);
void host_outputProgMemString(const char *str, uint8_t devno);
void host_outputChar(char c, uint8_t devno);
void host_outputFloat(float f,uint8_t  devno);
char *host_floatToStr(float f, char *buf);
int host_outputInt(long val, uint8_t devno);
void host_newLine(uint8_t devno);
char *host_readLine();
uint16_t host_input();
char *host_getInputText();
char host_getKey();
bool host_ESCPressed();
void host_outputFreeMem(unsigned int val);

void host_saveProgram(bool autoexec, int16_t flleNo);
void host_loadProgram(int16_t flleNo);
