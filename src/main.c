#include <stdint.h>
#define STBI_ONLY_JPEG
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#define _SQUARE(uchar) ((uint64_t)(uchar) * (uint64_t)(uchar))
#define SQUARE(r, g, b, a) (_SQUARE(r) + _SQUARE(g) + _SQUARE(b) + _SQUARE(a))

typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
} __attribute__((aligned(32))) pixel_t;

static inline uint64_t get_variance(uint32_t *data, unsigned char *mask,
                                    size_t size) {
  uint64_t count = 0;
  uint64_t sum_r = 0;
  uint64_t sum_g = 0;
  uint64_t sum_b = 0;
  uint64_t sum_a = 0;
  uint64_t sum_square = 0;
  pixel_t pixel = {0};

  for (size_t i = 0; i < size; ++i) {
    if (mask[i] == 0) {
      pixel = *((pixel_t *)&data[i]);
      sum_r += pixel.r;
      sum_g += pixel.g;
      sum_b += pixel.b;
      sum_a += pixel.a;
      sum_square += SQUARE(pixel.r, pixel.g, pixel.b, pixel.a);
      count++;
    }
  }

  return sum_square / count -
         SQUARE(sum_r, sum_g, sum_b, sum_a) / (count * count);

  return 0;
}

// http://fredericgoset.ovh/mathematiques/courbes/en/filled_circle.html
static inline unsigned char *create_circle_mask(int diameter) {
  unsigned char *grid = calloc(diameter * diameter, sizeof(*grid));
  int radius = diameter / 2;
  int x = 0;
  int y = radius;
  int m = 5 - 4 * radius;

  while (x <= y) {
    for (int xx = radius - y; xx <= radius + y; xx++) {
      if (radius - x >= 0 && radius - x < diameter && xx >= 0 && xx < diameter)
        grid[(radius - x) * diameter + xx] = 1;
      if (radius + x >= 0 && radius + x < diameter && xx >= 0 && xx < diameter)
        grid[(radius + x) * diameter + xx] = 1;
    }

    for (int xx = radius - x; xx <= radius + x; xx++) {
      if (radius - y >= 0 && radius - y < diameter && xx >= 0 && xx < diameter)
        grid[(radius - y) * diameter + xx] = 1;
      if (radius + y >= 0 && radius + y < diameter && xx >= 0 && xx < diameter)
        grid[(radius + y) * diameter + xx] = 1;
    }

    if (m > 0) {
      y--;
      m -= 8 * y;
    }

    x++;
    m += 8 * x + 4;
  }

  return grid;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    // usage: ./sorcel INFILE OUTFILE
    return 1;
  }

  char *path = argv[1];
  int width;
  int height;
  int channels;

  uint32_t *data = (uint32_t *)stbi_load(path, &width, &height, &channels, 4);
  if (width != height) {
    // image not square
    return 1;
  }

  if (channels != 3) {
    // image has transparency
    return 1;
  }

  unsigned char *mask = create_circle_mask(width);
  uint64_t variance = get_variance(data, mask, width * height);
  if (variance < 700) {
    // apply mask
    for (int i = 0; i < width * height; ++i) {
      if (mask[i] == 0) {
        data[i] &= 0x00ffffff; // 0xaabbggrr
      }
    }
  }

  stbi_write_png(argv[2], width, height, 4, data, width * 4);
  stbi_image_free(data);
  free(mask);
  return 0;
}
