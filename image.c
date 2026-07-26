#include <assert.h>
#include <stdint.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

uint32_t pixels[] = {
	0xFF000000,
	0xFF0000FF,
	0xFF0000FF,
	0xFF000000,
};

int main()
{
	int ok = stbi_write_png("image.png", 2, 2, 4, pixels, 2 * sizeof(uint32_t));
	assert(ok);

	return 0;
}