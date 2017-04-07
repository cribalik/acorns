#ifndef IMAGINE_H
#define IMAGINE_H

/* Img_Color */
struct Img_Color {
	unsigned char r,g,b,a;
};
struct Img_Color imagine_color_create(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
struct Img_Color imagine_color_alpha_blend(struct Img_Color back, struct Img_Color front);

/* BMP */

int imagine_bmp_save(const char *filename, const char* bitmap, int width, int height, int stride);

#endif /* IMAGINE_H */
