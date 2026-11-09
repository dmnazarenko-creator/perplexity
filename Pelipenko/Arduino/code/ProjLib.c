// ProjLib.c

#include "ProjLib.h"

void ProjLib_ValueToStingBin(uint32_t value, uint8_t Bits, char *buffer) {
  for (int i = (Bits - 1); i >= 0; i--) { buffer[Bits - 1 - i] = ((value >> i) & 1) ? '1' : '0'; }
//for (int i = (Bits - 1); i >= 0; i--) { buffer[Bits - 1 - i] = ((value >> i) & 1) ? '*' : '_'; }
  buffer[Bits] = '\0'; // Добавляем завершающий символ строки (\0)
}

const char HEX_DIGITS[] = "0123456789ABCDEF";
void ProjLib_ValueToStingHex(uint32_t value, uint8_t Char, char *buffer) {
  for(int i = 0; i < Char; i++) { buffer[i] = HEX_DIGITS[(value >> (((Char << 2) - 4) - (i << 2))) & 0xF]; }
  buffer[Char] = '\0'; // Добавляем завершающий символ строки (\0)
}

void ProjLib_BufBitRotR(uint8_t *Buf, int16_t BufSize) {
    uint8_t carry = (Buf[BufSize - 1] & 0x01) ? 0x80 : 0x00;
    for (int16_t i = BufSize - 1; i > 0; i--) {
      Buf[i]  = (Buf[i] >> 1);
      Buf[i] |= (Buf[i-1] & 0x01) ? 0x80 : 0x00;
    }
    Buf[0]  = (Buf[0] >> 1);
    Buf[0] |= carry;
}

void ProjLib_BufBitRotL(uint8_t *Buf, int16_t BufSize) {
  uint8_t carry = (Buf[0] & 0x80) ? 0x01 : 0x00;
  for (int16_t i = 0; i < BufSize-1; i++) {
    Buf[i]  = (Buf[i] << 1);
    Buf[i] |= (Buf[i+1] & 0x80) ? 0x01 : 0x00;
  }
  Buf[BufSize-1]  = (Buf[BufSize-1] << 1);
  Buf[BufSize-1] |= carry;
}

