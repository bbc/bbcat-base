#ifndef __BYTE_SWAP__
#define __BYTE_SWAP__

#include "misc.h"

BBC_AUDIOTOOLBOX_START

enum {
  SWAP_NEVER = 0,
  SWAP_ALWAYS,
  SWAP_FOR_LE,
  SWAP_FOR_BE,
};

extern void ByteSwap(void     *data, uint8_t itemsize, uint_t nitems = 1, uint8_t type = SWAP_ALWAYS);
extern void ByteSwap(uint32_t *val, uint_t nitems = 1, uint8_t type = SWAP_ALWAYS);
extern void ByteSwap(int32_t  *val, uint_t nitems = 1, uint8_t type = SWAP_ALWAYS);
extern void ByteSwap(uint16_t *val, uint_t nitems = 1, uint8_t type = SWAP_ALWAYS);
extern void ByteSwap(int16_t  *val, uint_t nitems = 1, uint8_t type = SWAP_ALWAYS);

extern void ByteSwap(uint32_t& val, uint8_t type = SWAP_ALWAYS);
extern void ByteSwap(int32_t&  val, uint8_t type = SWAP_ALWAYS);
extern void ByteSwap(uint16_t& val, uint8_t type = SWAP_ALWAYS);
extern void ByteSwap(int16_t&  val, uint8_t type = SWAP_ALWAYS);

#define BYTESWAP_ARRAY(x) ByteSwap(x, sizeof((x)[0]), sizeof(x) / sizeof((x)[0]))
#define BYTESWAP_VAR(x) ByteSwap(&x, sizeof(x), 1)

BBC_AUDIOTOOLBOX_END

#endif
