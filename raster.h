#ifndef RASTER_H
#define RASTER_H

extern void convert_image_to_raster(int width,
                                    int height,
                                    int pitch,
                                    unsigned char *pixels,
                                    unsigned char *raster);
extern int get_raster_chunk_index(int raster_width,
                                  int raster_height,
                                  int x,
                                  int y);
extern int get_raster_width_in_bytes(int columns);


#endif /* RASTER_H */
