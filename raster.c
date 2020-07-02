#include <string.h>

/* Get the width in bytes of a raster that has a number of columns. */
int get_raster_width_in_bytes(int columns) {
        if (columns == 0) {
                return 0;
        } else {
                return ((columns - 1) / 8) + 1;
        }
}

/* Get the index of the raster chunk containing a given coordinate. */
int get_raster_chunk_index(int cols,
                           int rows,
                           int x,
                           int y) {
        int raster_width = get_raster_width_in_bytes(cols);

        return (rows * raster_width) - (raster_width * (y + 1)) + (x / 8);
}

/* Convert a 1-bit image into a packed 1-bit raster. */
void convert_image_to_raster(int width,
                             int height,
                             int pitch,
                             unsigned char *pixels,
                             unsigned char *raster) {
        int raster_width;
        int raster_size;
        int i;

        raster_width = get_raster_width_in_bytes(width);
        raster_size  = sizeof(*raster) * raster_width * height;

        memset(raster, 0, raster_size);

        for (i = 0; i < width; i++) {
                int j;

                for (j = 0; j < height; j++) {
                        if (pixels[j * pitch + i] == 0) {
                                raster[
                                    get_raster_chunk_index(
                                        width,
                                        height,
                                        i, j)]
                                    |= 1 << (7 - (i % 8));
                        }
                }
        }
}
