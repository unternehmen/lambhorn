#include "raster.h"

int main(void) {
        if (get_raster_width_in_bytes(0) == 0) {
                return 0;
        } else {
                return 1;
        }
}
