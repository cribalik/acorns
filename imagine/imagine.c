#include "imagine.h"
#include <stdio.h>

struct Img_Color imagine_color_create(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	struct Img_Color c;
	c.r = r; c.g = g; c.b = b; c.a = a;
	return c;
}

/**
 * Alpha blend one color over another (non-premultiplied alpha)
 *
 * Example
 *
 *  struct Img_Color
 *    background = {0, 113, 70, 160},
 *    foreground = {0, 255, 255, 200};
 *
 *  struct Img_Color blended = imagine_color_alpha_blend(backgrund, foreground);
 *
 */
struct Img_Color imagine_color_alpha_blend(struct Img_Color back, struct Img_Color front) {
	#define IMAGINE_EPSILON 0.00001

	struct Img_Color result;
	double src_a, dst_a, out_a;

	dst_a = (double) back.a / 255.0;
	src_a = (double) front.a / 255.0;
	out_a = src_a + dst_a*(1-src_a);
	if (src_a > IMAGINE_EPSILON) {
		result.a = out_a * 255.0;
		result.r = (back.r*dst_a*(1-src_a) + front.r*src_a) / out_a;
		result.g = (back.g*dst_a*(1-src_a) + front.g*src_a) / out_a;
		result.b = (back.b*dst_a*(1-src_a) + front.b*src_a) / out_a;
	}
	else {
		result = back;
	}
	return result;

	#undef IMAGINE_EPSILON
}

/**
 * Parameters
 *
 *    rgb -> [r, g, b, x, x, r, g, b, x, x, r, ...]
 *           |--- stride ---|--- stride ---|---...
 *
 * Returns
 *    0 -> OK
 *    1 -> ERROR
 *
 * Example
 *
 *  char bmp[4*3] = {255, 123, 123, x, 255, 123, 123, x, 255, 123, 123, x, 255, 123, 123, x};
 *  int e = imagine_bmp_save("output.bmp", bmp, 2, 2, 4);
 *  if (e) {puts("Failed to save bmp file"); return 1;}
 */
int imagine_bmp_save(const char *filename, const char* rgb, int width, int height, int stride) {
	int i,j,ipos,err;
	int bytesPerLine;
	FILE *file;

	/* BMP header */
	short bfType;
	unsigned int bfSize;
	short bfReserved1;
	short bfReserved2;
	unsigned int bfOffBits;
	unsigned int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
	/* End BMP header */

	file = fopen(filename, "wb");
	if (!file) return 1;

	bytesPerLine = (3 * (width + 1) / 4) * 4;
	bfType = ('M' << 8) | 'B';
	bfOffBits = 54;
	bfSize = bfOffBits + bytesPerLine * height;
	bfReserved1 = 0;
	bfReserved2 = 0;
	biSize = 40;
	biWidth = width;
	biHeight = height;
	biPlanes = 1;
	biBitCount = 24;
	biCompression = 0;
	biSizeImage = bytesPerLine * height;
	biXPelsPerMeter = 0;
	biYPelsPerMeter = 0;
	biClrUsed = 0;       
	biClrImportant = 0; 

	fwrite(&bfType, 2, 1, file);
	fwrite(&bfSize, 4, 1, file);
	fwrite(&bfReserved1, 2, 1, file);
	fwrite(&bfReserved2, 2, 1, file);
	fwrite(&bfOffBits, 4, 1, file);
	fwrite(&biSize, 4, 1, file);
	fwrite(&biWidth, 4, 1, file);
	fwrite(&biHeight, 4, 1, file);
	fwrite(&biPlanes, 2, 1, file);
	fwrite(&biBitCount, 2, 1, file);
	fwrite(&biCompression, 4, 1, file);
	fwrite(&biSizeImage, 4, 1, file);
	fwrite(&biXPelsPerMeter, 4, 1, file);
	fwrite(&biYPelsPerMeter, 4, 1, file);
	fwrite(&biClrUsed, 4, 1, file);
	fwrite(&biClrImportant, 4, 1, file);

	for (i = height - 1; i >= 0; i--) {
		ipos = i * width * stride;
		for (j = 0; j < width; j++) {
			fwrite(rgb+ipos, 3, 1, file);
			ipos += stride;
		}
	}

	fclose(file);

	return 0;
}
