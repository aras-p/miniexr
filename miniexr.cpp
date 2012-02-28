#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])


unsigned char* miniexr_write (unsigned width, unsigned height, const void* rgba16f, size_t* outSize)
{
	const unsigned ww = width-1;
	const unsigned hh = height-1;
	const uint8_t kHeader[] = {
		0x76, 0x2f, 0x31, 0x01, // magic
		2, 0, 0, 0, // version, scanline
		// channels
		'c','h','a','n','n','e','l','s',0,
		'c','h','l','i','s','t',0,
		55,0,0,0,
		'B',0, 1,0,0,0, 0, 0,0,0,1,0,0,0,1,0,0,0, // R, half
		'G',0, 1,0,0,0, 0, 0,0,0,1,0,0,0,1,0,0,0, // G, half
		'R',0, 1,0,0,0, 0, 0,0,0,1,0,0,0,1,0,0,0, // B, half
		0,
		// compression
		'c','o','m','p','r','e','s','s','i','o','n',0,
		'c','o','m','p','r','e','s','s','i','o','n',0,
		1,0,0,0,
		0, // no compression
		// dataWindow
		'd','a','t','a','W','i','n','d','o','w',0,
		'b','o','x','2','i',0,
		16,0,0,0,
		0,0,0,0,0,0,0,0,
		ww&0xFF, (ww>>8)&0xFF, (ww>>16)&0xFF, (ww>>24)&0xFF,
		hh&0xFF, (hh>>8)&0xFF, (hh>>16)&0xFF, (hh>>24)&0xFF,
		// displayWindow
		'd','i','s','p','l','a','y','W','i','n','d','o','w',0,
		'b','o','x','2','i',0,
		16,0,0,0,
		0,0,0,0,0,0,0,0,
		ww&0xFF, (ww>>8)&0xFF, (ww>>16)&0xFF, (ww>>24)&0xFF,
		hh&0xFF, (hh>>8)&0xFF, (hh>>16)&0xFF, (hh>>24)&0xFF,
		// lineOrder
		'l','i','n','e','O','r','d','e','r',0,
		'l','i','n','e','O','r','d','e','r',0,
		1,0,0,0,
		0, // increasing Y
		// pixelAspectRatio
		'p','i','x','e','l','A','s','p','e','c','t','R','a','t','i','o',0,
		'f','l','o','a','t',0,
		4,0,0,0,
		0,0,0x80,0x3f, // 1.0f
		// screenWindowCenter
		's','c','r','e','e','n','W','i','n','d','o','w','C','e','n','t','e','r',0,
		'v','2','f',0,
		8,0,0,0,
		0,0,0,0, 0,0,0,0,
		// screenWindowWidth
		's','c','r','e','e','n','W','i','n','d','o','w','W','i','d','t','h',0,
		'f','l','o','a','t',0,
		4,0,0,0,
		0,0,0x80,0x3f, // 1.0f
		// end of header
		0,
	};
	const int kHeaderSize = ARRAY_SIZE(kHeader);

	const int kScanlineTableSize = 8 * height;
	const uint32_t pixelRowSize = width * 3 * 2;
	const uint32_t fullRowSize = pixelRowSize + 8;

	uint32_t bufSize = kHeaderSize + kScanlineTableSize + height * fullRowSize;
	uint8_t* buf = (uint8_t*)malloc (bufSize);

	// copy in header
	memcpy (buf, kHeader, kHeaderSize);

	// line offset table
	uint32_t ofs = kHeaderSize + kScanlineTableSize;
	uint8_t* ptr = buf + kHeaderSize;
	for (int y = 0; y < height; ++y)
	{
		*ptr++ = ofs & 0xFF;
		*ptr++ = (ofs >> 8) & 0xFF;
		*ptr++ = (ofs >> 16) & 0xFF;
		*ptr++ = (ofs >> 24) & 0xFF;
		*ptr++ = 0;
		*ptr++ = 0;
		*ptr++ = 0;
		*ptr++ = 0;
		ofs += fullRowSize;
	}

	// scanline data
	const uint8_t* src = (const uint8_t*)rgba16f;
	for (int y = 0; y < height; ++y)
	{
		// coordinate
		*ptr++ = y & 0xFF;
		*ptr++ = (y >> 8) & 0xFF;
		*ptr++ = (y >> 16) & 0xFF;
		*ptr++ = (y >> 24) & 0xFF;
		// data size
		*ptr++ = pixelRowSize & 0xFF;
		*ptr++ = (pixelRowSize >> 8) & 0xFF;
		*ptr++ = (pixelRowSize >> 16) & 0xFF;
		*ptr++ = (pixelRowSize >> 24) & 0xFF;
		// B, G, R
		memcpy (ptr, src, width*6);
		const uint8_t* chsrc;
		chsrc = src + 4;
		for (int x = 0; x < width; ++x)
		{
			*ptr++ = chsrc[0];
			*ptr++ = chsrc[1];
			chsrc += 8;
		}
		chsrc = src + 2;
		for (int x = 0; x < width; ++x)
		{
			*ptr++ = chsrc[0];
			*ptr++ = chsrc[1];
			chsrc += 8;
		}
		chsrc = src + 0;
		for (int x = 0; x < width; ++x)
		{
			*ptr++ = chsrc[0];
			*ptr++ = chsrc[1];
			chsrc += 8;
		}

		src += width*8;
	}

	assert (ptr - buf == bufSize);

	*outSize = bufSize;
	return buf;
}
