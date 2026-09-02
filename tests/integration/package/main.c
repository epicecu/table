#include "Table.h"

#include <stdint.h>

int main(void) {
  const int16_t axis[] = {0, 10};
  const uint8_t cells[] = {10u, 20u};
  table_view_t view = {0};
  return table_curve_view_init(&view, axis, 2u, TABLE_SCALAR_I16, cells, TABLE_SCALAR_U8) == TABLE_OK ? 0 : 1;
}
