// 74HC595 + SPI // https://forum.arduino.cc/t/74hc595-with-other-ics-sharing-spi-lines/974129
// 74HC595 + SPI // https://github.com/GyverLibs/GyverHC595
// ASCII U:0x55=0b01010101, https://ru.wikipedia.org/wiki/ASCII
// ASCII 3:0x33=0b00110011, ???:0xAA

// \r\n - ОС Windows
// \n   - Unix/Linux и macOS (современные версии)
// \r   - Классические Mac OS (старые версии)
// \r 13 - Carriage Return (CR), "возврат каретки". Его числовой код в ASCII — 13.
// \n 10  - Line Feed (LF),  "перевод строки". Его числовой код в ASCII — 10.

const char String_Ok[]   = "\r\nOk\r\n";
const char String_Er[]   = "\r\nEr\r\n";
const char String_Ver[]  = "\r\nVer.2025-11-16\r\n";
const char String_Help[] = "\r\nCommands:\r\n"
"h (H)               help         Respoce : \\r\\n help content \\r\\n\r\n"
"v (V)               version      Respoce : \\r\\n Ver.20xx-xx-xx \\r\\n\r\n"
"t01 .. t99 / T01 .. T99 test T/B Respoce : test data\r\n"
"s1 .. s3  (S1 .. S3) Sound       Respoce : Er/Ok\r\n"
"l0 .. l3  (L0 .. L3) LEDs        Respoce : Er/Ok\r\n";

#define  SPI_Mode    'D' // 'S'-Single / 'D'-Double
#define  OutDataMode 'B' // 'B'-Bin / 'T'-Text
#define  DebugMode   (7) // (0)-Rev1 / (7)-Rev2
#define  Tone_Enable 'E' // 'E'-Enable / 'D'-Disable

#ifndef  Tone_Enable
 #error 'Tone_Enable' is not define
#elif   (Tone_Enable == 'D')
 // NONE
#elif   (Tone_Enable == 'E')
 #include "Tone.h" // Подключаем библиотеку Tone
#else // Tone_Enable
 #error Unsupported Tone_Enable value
#endif // Tone_Enable

#include <SPI.h>
#include "ProjLib.h"

// Настройки UART
  #define UART_BAUD_RATE 2000000 // (Max)   2 МBOD
//#define UART_BAUD_RATE  500000
//#define UART_BAUD_RATE  115200 // (Min) 115 kBOD

// Настройки SPI
  #define SPI_CLK_SPEED  8000000 // (Max)   8 МГц
//#define SPI_CLK_SPEED   125000 // (Min) 125 kГц
// 11 MOSI - Arduino to device
// 12 MISO - device to Arduino
// 13 SCK  - Clock

// Настройки GPIO // 2..10

#ifndef  DebugMode
#error  'DebugMode' is not define
#elif   (DebugMode == 1)
/* test only */ #define Pin_W_CLK   13 //  test only
/* test only */ //#define Pin_W_MISO  12 //  test only
/* test only */ #define Pin_W_MOSI  11 //  test only
#elif  ((DebugMode == 2) || (DebugMode == 3) || (DebugMode == 4) || (DebugMode == 5) || (DebugMode == 6) || (DebugMode == 7) || (DebugMode == 0))
 // NONE
#else // DebugMode
 #error Unsupported DebugMode value
#endif // DebugMode

#define Pin_W_OE   10 //  4
#define Pin_W_CLR   9 //  6
#define Pin_W_Load  8 //  8
#define Pin_R_Load  7 // 18
#define Pin_R_nCE   6 // 20
#define Pin_LEDR    5
#define Pin_LEDG    4

#ifndef  Tone_Enable
 #error 'Tone_Enable' is not define
#elif   (Tone_Enable == 'D')
 // NONE
#elif   (Tone_Enable == 'E')
 #define Pin_Sound   3
#else // Tone_Enable
 #error Unsupported Tone_Enable value
#endif // Tone_Enable


#define Code_All_Off      '0'
#define Code_LedR_On      '1'
#define Code_LedR_Off     '2'
#define Code_LedR_Flash   '3'
#define Code_LedG_On      '4'
#define Code_LedG_Off     '5'
#define Code_LedG_Flash   '6'

#ifndef  Tone_Enable
 #error 'Tone_Enable' is not define
#elif   (Tone_Enable == 'D')
 // NONE
#elif   (Tone_Enable == 'E')
 #define Code_SoundOff     'A'
 #define Code_SoundOK      '7'
 #define Code_SoundErr     '8'
 #define Code_SoundNone    '9'

 // Ноты четвёртой октавы (C4-G4)
 #define NOTE_C4 262
 #define NOTE_CS4 277
 #define NOTE_D4 294
 #define NOTE_DS4 311
 #define NOTE_E4 330
 #define NOTE_F4 349
 #define NOTE_FS4 370
 #define NOTE_G4 392
 #define NOTE_GS4 415
 #define NOTE_A4 440
 #define NOTE_AS4 466
 #define NOTE_B4 494

 //const int notes_OK[]   = { 8, 200, 100, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4 }; // Частоты нот (до, ре, ми, фа, соль)
 const int notes_OK[]   = { 7, 150, 150, NOTE_G4, NOTE_B4, NOTE_G4, NOTE_D5 };
 //const int notes_Err[]  = { 8, 200, 100, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4, NOTE_C4 }; // Частоты нот (до, ре, ми, фа, соль)
 const int notes_Err[]  = { 6, 150, 150, NOTE_B3, NOTE_A3, NOTE_FS3 }; // B3 → A3 → F#3 (низкий регистр, короткие импульсы).
 //const int notes_None[] = { 8, 200, 100, NOTE_C4, NOTE_E4, NOTE_G4, NOTE_E4, NOTE_C4 }; // Частоты нот (до, ре, ми, фа, соль)
 const int notes_None[] = { 6, 100, 100, NOTE_C5, NOTE_FS5, NOTE_GS5 }; // Частоты нот (до, ре, ми, фа, соль)
 Tone myTone;
 void playMelody(const int notes[]) {
  for(int i = 3; i < notes[0]; i++) {
    myTone.play(notes[i], notes[1]); // Воспроизводим каждую ноту длительностью notes[1]
    delay(notes[2]);                 // Короткая пауза перед следующей нотой
  }
  myTone.stop();                     // Останавливаем звук
 }
#else // Tone_Enable
 #error Unsupported Tone_Enable value
#endif // Tone_Enable

//#define Lib_Pin_W_OE()   { digitalWrite(Pin_W_OE,   LOW); /*delayMicroseconds(1);*/ digitalWrite(Pin_W_OE,   HIGH); } // ---\___/--- 4 uS
#define Lib_Pin_W_CLR()  { digitalWrite(Pin_W_CLR,  LOW); /*delayMicroseconds(1);*/ digitalWrite(Pin_W_CLR,  HIGH); } // ---\___/--- 4 uS
#define Lib_Pin_W_Load() { digitalWrite(Pin_W_Load, LOW); /*delayMicroseconds(1);*/ digitalWrite(Pin_W_Load, HIGH); } // ---\___/--- 4 uS
#define Lib_Pin_R_Load() { digitalWrite(Pin_R_Load, LOW); /*delayMicroseconds(1);*/ digitalWrite(Pin_R_Load, HIGH); } // ---\___/--- 4 uS
//#define Lib_Pin_R_nCE()  { digitalWrite(Pin_R_nCE,  LOW); /*delayMicroseconds(1);*/ digitalWrite(Pin_R_nCE,  HIGH); } // ---\___/--- 4 uS
#define Lib_LEDR_On()    { digitalWrite(Pin_LEDR,     LOW); }
#define Lib_LEDR_Off()   { digitalWrite(Pin_LEDR,    HIGH); }
#define Lib_LEDG_On()    { digitalWrite(Pin_LEDG,     LOW); }
#define Lib_LEDG_Off()   { digitalWrite(Pin_LEDG,    HIGH); }

#ifndef  Tone_Enable
 #error 'Tone_Enable' is not define
#elif   (Tone_Enable == 'D')
 #define Lib_Sound_OK()   // NONE
 #define Lib_Sound_Err()  // NONE
 #define Lib_Sound_None() // NONE
 #define Lib_Sound_Off()  // NONE
#elif   (Tone_Enable == 'E')
 #define Lib_Sound_OK()   { playMelody(notes_OK);            }
 #define Lib_Sound_Err()  { playMelody(notes_Err);           }
 #define Lib_Sound_None() { playMelody(notes_None);          }
 #define Lib_Sound_Off()  { myTone.stop();                   }
#else // Tone_Enable
 #error Unsupported Tone_Enable value
#endif // Tone_Enable

void PortControl(uint8_t Data) {
    switch(Data) {
        case Code_All_Off:    Lib_LEDG_Off(); Lib_LEDR_Off(); Lib_Sound_Off(); break;
        case Code_LedR_On:    Lib_LEDR_On();    break;
        case Code_LedR_Off:   Lib_LEDR_Off();   break;
        case Code_LedR_Flash: Lib_LEDR_On();    break;
        case Code_LedG_On:    Lib_LEDG_On();    break;
        case Code_LedG_Off:   Lib_LEDG_Off();   break;
        case Code_LedG_Flash: Lib_LEDG_On();    break;

#ifndef  Tone_Enable
 #error 'Tone_Enable' is not define
#elif   (Tone_Enable == 'D')
 // NONE
#elif   (Tone_Enable == 'E')
        case Code_SoundOK:    Lib_Sound_OK();   break;
        case Code_SoundErr:   Lib_Sound_Err();  break;
        case Code_SoundNone:  Lib_Sound_None(); break;
        case Code_SoundOff:   Lib_Sound_Off();  break;
#else // Tone_Enable
 #error Unsupported Tone_Enable value
#endif // Tone_Enable

        default:              Lib_LEDG_Off(); Lib_LEDR_Off(); Lib_Sound_Off(); break;
    }
}

// Буфер данных
#define BUFFER_SIZE (8) // 2 boards 32 bit, 2 * 32 bit = 2 * 4 * 8 bit
uint8_t BufUART_Rx[BUFFER_SIZE];
uint8_t BufUART_Tx[BUFFER_SIZE];
int BufPointer;

//void SPI_DataExchange(void) { for(int i = 0; i < BUFFER_SIZE; i++) { BufUART_Tx[                    i] = SPI.transfer(BufUART_Rx[i]); } Lib_Pin_W_Load(); }
  void SPI_DataExchange(void) { for(int i = 0; i < BUFFER_SIZE; i++) { BufUART_Tx[(BUFFER_SIZE - 1) - i] = SPI.transfer(BufUART_Rx[i]); } Lib_Pin_W_Load(); }

void setup() {
  // Настройка GPIO
#ifndef  DebugMode
#error  'DebugMode' is not define
#elif   (DebugMode == 1)
/* test only */  pinMode(Pin_W_CLK,   OUTPUT); digitalWrite(Pin_W_CLK,     LOW); //  4 // 74HC595 OE:LOW, Z:HIGH (-Z--\_OE_/--Z-)
/* test only */  //pinMode(Pin_W_MISO,  OUTPUT); digitalWrite(Pin_W_MISO,     LOW); //  4 // 74HC595 OE:LOW, Z:HIGH (-Z--\_OE_/--Z-)
/* test only */  pinMode(Pin_W_MOSI,  OUTPUT); digitalWrite(Pin_W_MOSI,     LOW); //  4 // 74HC595 OE:LOW, Z:HIGH (-Z--\_OE_/--Z-)
#elif  ((DebugMode == 2) || (DebugMode == 3) || (DebugMode == 4) || (DebugMode == 5) || (DebugMode == 6) || (DebugMode == 7) || (DebugMode == 0))
 // NONE
#else // DebugMode
 #error Unsupported DebugMode value
#endif // DebugMode

  pinMode(Pin_W_OE,    OUTPUT); digitalWrite(Pin_W_OE,     LOW); //  4 // 74HC595 OE:LOW, Z:HIGH (-Z--\_OE_/--Z-)
  pinMode(Pin_W_CLR,   OUTPUT); digitalWrite(Pin_W_CLR,   HIGH); //  6 // 74HC595 Reset:LOW ---\__Reset__/---
  pinMode(Pin_W_Load,  OUTPUT); digitalWrite(Pin_W_Load,  HIGH); //  8 // 74HC595 LATCH ___/--- Latch    (---\__/---)
  pinMode(Pin_R_Load,  OUTPUT); digitalWrite(Pin_R_Load,  HIGH); // 18 // 74HC165 ---\__/--- параллельная загрузка данных в регистр перед чтением.
  pinMode(Pin_R_nCE,   OUTPUT); digitalWrite(Pin_R_nCE,    LOW); // 20 // 74HC165 не астивен ------\____CLK ENABLE__/---не астивен
  pinMode(Pin_LEDR,    OUTPUT); digitalWrite(Pin_LEDR,    HIGH);
  pinMode(Pin_LEDG,    OUTPUT); digitalWrite(Pin_LEDG,    HIGH);

#ifndef  Tone_Enable
 #error 'Tone_Enable' is not define
#elif   (Tone_Enable == 'D')
 // NONE
#elif   (Tone_Enable == 'E')
  pinMode(Pin_Sound,   OUTPUT); // Настраиваем пин вывода сигнала на динамику
  myTone.begin(Pin_Sound);      // Инициализируем модуль Tone с выбранным пином
                             // digitalWrite(Pin_Sound,   HIGH);
#else // Tone_Enable
 #error Unsupported Tone_Enable value
#endif // Tone_Enable

#ifndef  DebugMode
 #error 'DebugMode' is not define
#elif   (DebugMode == 1)
 // NONE
#elif  ((DebugMode == 2) || (DebugMode == 3) || (DebugMode == 4) || (DebugMode == 5) || (DebugMode == 6) || (DebugMode == 7) || (DebugMode == 0))
  // Настройка SPI
  SPI.begin();
  SPI.beginTransaction(SPISettings(SPI_CLK_SPEED, MSBFIRST, SPI_MODE0));
#else // DebugMode
 #error Unsupported DebugMode value
#endif // DebugMode

  // Настройка UART
  Serial.begin(UART_BAUD_RATE);
  Serial.println();               Serial.println("Program running");
  Serial.print("DebugMode   : "); Serial.println(DebugMode);
  Serial.print("OutDataMode : "); Serial.println(OutDataMode);
  Serial.print("SPI_Mode    : "); Serial.println(SPI_Mode);
  Serial.print("Tone_Enable : "); Serial.println(Tone_Enable);

  Lib_Pin_W_CLR();
}

void loop() {
#ifndef  DebugMode
#error  'DebugMode' is not define
#elif   (DebugMode == 1)
 #define delay_uS    10
  while(1){
/* test only */  digitalWrite(Pin_W_CLK,   HIGH); delayMicroseconds(delay_uS); // delay(delay_mS);
/* test only */  digitalWrite(Pin_W_MOSI,  HIGH); delayMicroseconds(delay_uS); // delay(delay_mS);
   digitalWrite(Pin_W_OE,    HIGH); delayMicroseconds(delay_uS); // delay(delay_mS);
   digitalWrite(Pin_W_CLR,   HIGH); delayMicroseconds(delay_uS); // delay(delay_mS);
   digitalWrite(Pin_W_Load,  HIGH); delayMicroseconds(delay_uS); // delay(delay_mS);
   digitalWrite(Pin_R_Load,  HIGH); delayMicroseconds(delay_uS); // delay(delay_mS);
   digitalWrite(Pin_R_nCE,   HIGH); delayMicroseconds(delay_uS); // delay(delay_mS);
/* test only */  digitalWrite(Pin_W_CLK,   LOW); delayMicroseconds(delay_uS); // delay(delay_mS);
/* test only */  digitalWrite(Pin_W_MOSI,  LOW); delayMicroseconds(delay_uS); // delay(delay_mS);
   digitalWrite(Pin_W_OE,     LOW); delayMicroseconds(delay_uS); // delay(delay_mS);
   digitalWrite(Pin_W_CLR,    LOW); delayMicroseconds(delay_uS); // delay(delay_mS);
   digitalWrite(Pin_W_Load,   LOW); delayMicroseconds(delay_uS); // delay(delay_mS);
   digitalWrite(Pin_R_Load,   LOW); delayMicroseconds(delay_uS); // delay(delay_mS);
   digitalWrite(Pin_R_nCE,    LOW); delayMicroseconds(delay_uS); // delay(delay_mS);
  }
#elif   (DebugMode == 2)
  for(int i = 0; i < 256; i++) {
     //Lib_Pin_W_CLR();
     SPI.transfer(i);
     Lib_Pin_W_Load();
     Lib_Pin_R_Load();
     //Lib_Pin_W_OE();
     //Lib_Pin_R_nCE();
  }
#elif   (DebugMode == 3)
  if(Serial.available() > 0) {   // Проверяем наличие символов в буфере последовательного порта
    char receivedChar = Serial.read(); // Читаем один символ
    Serial.write(receivedChar); // Отправляем этот же символ обратно в порт
  }
#elif   (DebugMode == 4)
  if(Serial.available() > 0) {   // Проверяем наличие символов в буфере последовательного порта
    char receivedChar = Serial.read(); // Читаем один символ

    Lib_Pin_W_CLR();
    Lib_Pin_R_Load();

    // Обмен данными через SPI
    // digitalWrite(Pin_W_Load, LOW);
    receivedChar = SPI.transfer(receivedChar);
    // digitalWrite(Pin_W_Load, HIGH); // LATCH Data 74HC595
    Lib_Pin_W_Load();

    Serial.write(receivedChar); // Отправляем этот же символ обратно в порт
  }
#elif   (DebugMode == 5)
  if(Serial.available() >= BUFFER_SIZE) { // Проверяем наличие достаточного количества символов в буфере
    for(BufPointer = 0; BufPointer < BUFFER_SIZE; BufPointer++) { BufUART_Rx[BufPointer] = Serial.read(); } // Читаем байты один за другим

    Lib_Pin_W_CLR();
    Lib_Pin_R_Load();

    // Обмен данными через SPI
    // digitalWrite(Pin_W_Load, LOW);
    for(int i = 0; i < BufPointer; i++) { BufUART_Tx[i] = SPI.transfer(BufUART_Rx[i]); }
    // digitalWrite(Pin_W_Load, HIGH); // LATCH Data 74HC595
    Lib_Pin_W_Load();

    // Передаем данные BufUART_Tx через UART
    //Serial.write(BufUART_Tx, BUFFER_SIZE);

        Serial.print("Rx: ");
        for(BufPointer = 0; BufPointer < BUFFER_SIZE; BufPointer++) {
          Serial.print("0x");
          if(BufUART_Rx[BufPointer] < 16) { Serial.print("0"); }
          Serial.print(BufUART_Rx[BufPointer], HEX);
          Serial.print(' ');
        }
        Serial.println();

        Serial.print("Tx: ");
        for(BufPointer = 0; BufPointer < BUFFER_SIZE; BufPointer++) {
          Serial.print("0x");
          if(BufUART_Tx[BufPointer] < 16) { Serial.print("0"); }
          Serial.print(BufUART_Tx[BufPointer], HEX);
          Serial.print(' ');
        }
        Serial.println();

        Serial.println();
  }
#elif   (DebugMode == 6)
  PortControl(Code_All_Off);   delay(2000);
  PortControl(Code_LedR_On);   delay(2000);
  PortControl(Code_LedR_Off);  delay(2000);
  PortControl(Code_SoundOK);   delay(2000);
  PortControl(Code_LedG_On);   delay(2000);
  PortControl(Code_LedG_Off);  delay(2000);
  PortControl(Code_SoundErr);  delay(2000);
  PortControl(Code_LedR_On);   delay(2000);
  PortControl(Code_LedG_On);   delay(2000);
  PortControl(Code_LedR_Off);  delay(2000);
  PortControl(Code_LedG_Off);  delay(2000);
  PortControl(Code_SoundNone); delay(2000);
#elif   (DebugMode == 7)
  if(Serial.available() > 0) {   // Проверяем наличие символов в буфере последовательного порта
    char receivedChar = Serial.read();
    static int ComValue = 0, State = 0;
    switch(State){
    case 0:
      if((receivedChar == 'h') || (receivedChar == 'H')) { Serial.print(String_Help); }
      if((receivedChar == 'v') || (receivedChar == 'V')) { Serial.print(String_Ver);  }
      if( receivedChar == 't'                          ) { State = 1; } // 1, 2
      if( receivedChar == 'T'                          ) { State = 3; } // 3, 4
      if((receivedChar == 's') || (receivedChar == 'S')) { State = 5; } // 5
      if((receivedChar == 'l') || (receivedChar == 'L')) { State = 6; } // 6
      break;

    case 1:
    case 3:
      if((receivedChar <  '0') || (receivedChar >  '9')) { receivedChar = '0'; }
      ComValue  = (receivedChar - '0') * 10;
      State++;
      break;

    case 2:
    case 4:
      if((receivedChar <  '0') || (receivedChar >  '9')) { receivedChar = '0'; }
      ComValue += (receivedChar - '0');
      if(State == 2) { //Serial.print("\r\nComm t, ComValue = "); Serial.println(ComValue);
// Start Test with HEX output
      } else
      if(State == 4) { //Serial.print("\r\nComm T, ComValue = "); Serial.println(ComValue);
// Start Test with BIN output
      }
      State = 0;
      break;

    case 5:
      if((receivedChar <  '1') || (receivedChar >  '3')) { Serial.print(String_Er); } else {
        ComValue  = (receivedChar - '0');
        if(ComValue == 1) { Lib_Sound_OK();   }
        if(ComValue == 2) { Lib_Sound_Err();  }
        if(ComValue == 3) { Lib_Sound_None(); }
        Serial.print(String_Ok);
      }
      State = 0;
      break;

    case 6:
      if((receivedChar <  '0') || (receivedChar >  '3')) { Serial.print(String_Er); } else {
        ComValue  = (receivedChar - '0');
        if((ComValue & 1) > 0) { Lib_LEDR_On(); } else { Lib_LEDR_Off(); }
        if((ComValue & 2) > 0) { Lib_LEDG_On(); } else { Lib_LEDG_Off(); }
        Serial.print(String_Ok);
      }
      State = 0;
      break;

    default:
      break;
    }
  }
#elif   (DebugMode == 0)
  if(Serial.available() >= BUFFER_SIZE) { // Проверяем наличие достаточного количества символов в буфере
    for(BufPointer = 0; BufPointer < BUFFER_SIZE; BufPointer++) { BufUART_Rx[BufPointer] = Serial.read(); } // Читаем байты один за другим

    Lib_Pin_W_CLR();
    Lib_Pin_R_Load();
    SPI_DataExchange(); // Обмен данными через SPI
#ifndef  SPI_Mode
 #error 'SPI_Mode' is not define
#elif   (SPI_Mode == 'S')
 // NONE
#elif   (SPI_Mode == 'D')
    delayMicroseconds(100); // delay(delay_mS);
    Lib_Pin_W_CLR();
    Lib_Pin_R_Load();
    SPI_DataExchange(); // Обмен данными через SPI
#else // SPI_Mode
 #error Unsupported SPI_Mode value
#endif // SPI_Mode

    // Передаем данные BufUART_Tx через UART
#ifndef  OutDataMode
 #error 'OutDataMode' is not define
#elif   (OutDataMode == 'B')
    Serial.write(BufUART_Tx, BUFFER_SIZE);
#elif   (OutDataMode == 'T')
    {
    char String[8 + 1]; // 8 + завершающий символ строки (\0)
    Serial.print("Rx: "); for(BufPointer = 0; BufPointer < BUFFER_SIZE; BufPointer++) { ProjLib_ValueToStingBin(BufUART_Rx[BufPointer],                          8, String); Serial.print(String); Serial.print('.'); }
                          for(BufPointer = 0; BufPointer < BUFFER_SIZE; BufPointer++) { ProjLib_ValueToStingHex(BufUART_Rx[BufPointer],                          2, String); Serial.print(String); Serial.print('.'); } Serial.println();
    Serial.print("Tx: "); for(BufPointer = 0; BufPointer < BUFFER_SIZE; BufPointer++) { ProjLib_ValueToStingBin(BufUART_Tx[BufPointer],                          8, String); Serial.print(String); Serial.print('.'); }
                          for(BufPointer = 0; BufPointer < BUFFER_SIZE; BufPointer++) { ProjLib_ValueToStingHex(BufUART_Tx[BufPointer],                          2, String); Serial.print(String); Serial.print('.'); } Serial.println();
    Serial.print("XOR "); for(BufPointer = 0; BufPointer < BUFFER_SIZE; BufPointer++) { ProjLib_ValueToStingBin(BufUART_Rx[BufPointer] ^ BufUART_Tx[BufPointer], 8, String); Serial.print(String); Serial.print('.'); }
                          for(BufPointer = 0; BufPointer < BUFFER_SIZE; BufPointer++) { ProjLib_ValueToStingHex(BufUART_Rx[BufPointer] ^ BufUART_Tx[BufPointer], 2, String); Serial.print(String); Serial.print('.'); } Serial.println(); Serial.println();
    }
#else // OutDataMode
 #error Unsupported OutDataMode value
#endif // OutDataMode
  }
#else // DebugMode
 #error Unsupported DebugMode value
#endif // DebugMode
}

