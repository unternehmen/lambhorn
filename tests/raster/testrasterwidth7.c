#include "raster.h"

int main(void) {
        if (get_raster_width_in_bytes(7) == 1) {
                return 0;
        } else {
                return 1;
        }
}