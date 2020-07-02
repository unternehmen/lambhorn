#include "raster.h"

int main(void) {
        if (get_raster_width_in_bytes(1) == 1) {
                return 0;
        } else {
                return 1;
        }
}
