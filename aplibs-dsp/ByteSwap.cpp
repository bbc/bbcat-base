
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ByteSwap.h"

#ifndef MACHINE_IS_BIG_ENDIAN
// endianness cannot be determined at compile time, determine at run-time
bool __MachineIsBigEndian()
{
	static const uint16_t val = 1;
	static const uint8_t *ptr = (const uint8_t *)&val;
	return (ptr[0] == 0);
}
const bool MACHINE_IS_BIG_ENDIAN = __MachineIsBigEndian();
#endif

void ByteSwap(void *data, uint8_t itemsize, uint_t nitems, uint8_t type)
{
	if ((itemsize > 1) &&
		((type == SWAP_ALWAYS) ||
		 ((type == SWAP_FOR_LE) &&  MACHINE_IS_BIG_ENDIAN) ||
		 ((type == SWAP_FOR_BE) && !MACHINE_IS_BIG_ENDIAN))) {
		uint8_t *p = (uint8_t *)data;
		uint8_t nb = itemsize >> 1;
		uint8_t i, j;

		do {
			for (i = 0, j = itemsize - 1; i < nb; i++, j--) {
				uint8_t t = p[i];
				p[i] = p[j];
				p[j] = t;
			}
			p += itemsize;
			nitems--;
		}
		while (nitems);
	}
}

void ByteSwap(uint32_t *val, uint_t nitems, uint8_t type)
{
	return ByteSwap(val, sizeof(*val), nitems, type);
}

void ByteSwap(uint32_t& val, uint8_t type)
{
	return ByteSwap(&val, sizeof(val), 1, type);
}

void ByteSwap(int32_t *val, uint_t nitems, uint8_t type)
{
	return ByteSwap(val, sizeof(*val), nitems, type);
}

void ByteSwap(int32_t& val, uint8_t type)
{
	return ByteSwap(&val, sizeof(val), 1, type);
}

void ByteSwap(uint16_t *val, uint_t nitems, uint8_t type)
{
	return ByteSwap(val, sizeof(*val), nitems, type);
}

void ByteSwap(uint16_t& val, uint8_t type)
{
	return ByteSwap(&val, sizeof(val), 1, type);
}

void ByteSwap(int16_t *val, uint_t nitems, uint8_t type)
{
	return ByteSwap(val, sizeof(*val), nitems, type);
}

void ByteSwap(int16_t& val, uint8_t type)
{
	return ByteSwap(&val, sizeof(val), 1, type);
}
