#include "basic.h"
#include "host.h"

// BASIC
unsigned char mem[MEMORY_SIZE];
#define TOKEN_BUF_SIZE    64
unsigned char tokenBuf[TOKEN_BUF_SIZE];

const char welcomeStr[] PROGMEM = "Arduino BASIC";
char autorun = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(100);
    
    reset();
    host_init(0);
    host_cls();
    
    // 起動メッセージ出力
    host_outputProgMemString(welcomeStr,CDEV_SCREEN);

    // show memory size （メモリサイズ出力）
    host_outputFreeMem(sysVARSTART - sysPROGEND);
    host_newLine(CDEV_SCREEN);
    host_outputString((char *)errorTable[ERROR_NONE],CDEV_SCREEN);
    host_newLine(CDEV_SCREEN);    
}

void loop() {
    int ret = ERROR_NONE;

    if (!autorun) {
        // 自動実行でない場合の処理
        // get a line from the user
        char *input = host_readLine();

        // special editor commands
        //（特別な編集コマンドの入力チェック）
        if (input[0] == '?' && input[1] == 0) {
            // 空きメモリ容量出力
            host_outputFreeMem(sysVARSTART - sysPROGEND-2);
            host_newLine(CDEV_SCREEN);
               return;
        }
        
        // otherwise tokenize
        // （ライン入力をトークンへ変換し、トークンバッファに格納）
        ret = tokenize((unsigned char*)input, tokenBuf, TOKEN_BUF_SIZE);

    } else {
/*
        // 自動実行の場合の処理、トークンバッファに"RUN"をセット
        host_loadProgram();
        tokenBuf[0] = TOKEN_RUN; 
        tokenBuf[1] = 0;
        autorun = 0;
*/
    }
  
    // execute the token buffer
    // (トークンバッファのトークン実行）
    if (ret == ERROR_NONE) {
        //Serial.println("step1");
        host_newLine(CDEV_SCREEN);
        ret = processInput(tokenBuf);  // インタープリタの処理
    }
  
    if (ret != ERROR_NONE) {
        // 実行エラー発生
        host_newLine(CDEV_SCREEN);
        if (lineNumber !=0) {
            // 行番号付きの場合、行番号表示
            host_outputInt(lineNumber,CDEV_SCREEN);
            host_outputChar('-',CDEV_SCREEN);
        }
        
        // エラーメッセージ出力
        host_outputString((char *)errorTable[ret],CDEV_SCREEN);
        host_newLine(CDEV_SCREEN);     
    } else {
       if (isExecute() && tokenBuf[0]) {
          host_outputProgMemString(errorTable[ERROR_NONE],CDEV_SCREEN);
          host_newLine(CDEV_SCREEN);
       }
    }
}
