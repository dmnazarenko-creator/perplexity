// ProjLib.h

#ifndef ProjLib_H
#define ProjLib_H

#ifdef __cplusplus
extern "C" { // Эта конструкция необходима для совместимости с кодом на C++.
#endif

#include <stdint.h> // for uint32_t

/* Example
  {
    int32_t a = 0xFFFFFF00; char String[32 + 1]; // 32 + завершающий символ строки (\0)
    while(1){
      ProjLib_ValueToStingBin(a, 12, String); Serial.print("0b"); Serial.print(String); Serial.print(" - ");
      ProjLib_ValueToStingHex(a,  3, String); Serial.print("0x"); Serial.println(String); a++; delay(20);
      }
  }
*/
void ProjLib_ValueToStingBin(uint32_t value, uint8_t Bits, char *buffer);
void ProjLib_ValueToStingHex(uint32_t value, uint8_t Char, char *buffer);
void ProjLib_BufBitRotR(uint8_t *Buf, int16_t BufSize);
void ProjLib_BufBitRotL(uint8_t *Buf, int16_t BufSize);

#ifdef __cplusplus
}
#endif

#endif /* ProjLib_H */

