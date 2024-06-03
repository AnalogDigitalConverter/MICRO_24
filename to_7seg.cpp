#include "to_7seg.h"

// converts to seven segment (active high, GFEDCBA)
// input codes from 0 to 15 give simbols:
//  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, -, r, n, E, F}
//  other inputs give a blank display
int8_t to_7seg (uint8_t code) {
  // complete the code for this function  ++++++++++++++++++++++++++++++
  static int8_t const sseg[] = {
    0x3f, 0x30, 0x5b, 0x4f,
    0x66, 0x6d, 0x7d, 0x07,
    0x7f, 0x6f, 0x77, 0x40,
    0x50, 0x54, 0x79, 0x71
  };

  return (code > 0x0f) ? 0 : sseg[code];
  //  ------------------------------------------------------------------
}
