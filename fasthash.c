/* The MIT License

   Copyright (C) 2012 Zilong Tan (eric.zltan@gmail.com)

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include "fasthash.h"

static inline uint64_t little_endian64(uint64_t x)
{
	static const union {
		uint16_t x;
		uint8_t b[2];
	} _endian = { 1 };
	if (_endian.b[0])
		return x;
	return (x >> 56) | ((x >> 40) & 0xff00) | ((x >> 24) & 0xff0000) |
		   ((x >> 8) & 0xff000000) | ((x << 8) & 0xff00000000) |
		   ((x << 24) & 0xff0000000000) | ((x << 40) & 0xff000000000000) |
		   (x << 56);
}

static inline uint64_t mix(uint64_t x)
{
	x ^= x >> 23;
	x *= 0x2127599bf4325c37ULL;
	return x ^= x >> 47;
}

uint64_t fasthash64(const void *buf, size_t len, uint64_t seed)
{
	const uint64_t m = 0x880355f21e6d1965ULL;
	const uint64_t *pos = (const uint64_t *)buf;
	const uint64_t *end = pos + (len / 8);
	const uint8_t *pos2;
	uint64_t h = seed ^ (len * m);
	uint64_t v;

	while (pos != end)
	{
		v = little_endian64(*pos++);
		h ^= mix(v);
		h *= m;
	}

	pos2 = (const uint8_t *)pos;
	v = 0;

	switch (len & 7)
	{
	case 7:
		v ^= (uint64_t)pos2[6] << 48;
	case 6:
		v ^= (uint64_t)pos2[5] << 40;
	case 5:
		v ^= (uint64_t)pos2[4] << 32;
	case 4:
		v ^= (uint64_t)pos2[3] << 24;
	case 3:
		v ^= (uint64_t)pos2[2] << 16;
	case 2:
		v ^= (uint64_t)pos2[1] << 8;
	case 1:
		v ^= (uint64_t)pos2[0];
		h ^= mix(v);
		h *= m;
	}

	return mix(h);
}

uint32_t fasthash32(const void *buf, size_t len, uint32_t seed)
{
	// the following trick converts the 64-bit hashcode to Fermat
	// residue, which shall retain information from both the higher
	// and lower parts of hashcode.
	const uint64_t h = fasthash64(buf, len, seed);
	return h - (h >> 32);
}
