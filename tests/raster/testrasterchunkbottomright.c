#include "raster.h"

int main(void) {
        if (get_raster_chunk_index(16, 16, 15, 15) == 1) {
                return 0;
        } else {
                return 1;
        }
}
