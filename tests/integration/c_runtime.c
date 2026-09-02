#include "table/table.h"

#include <stdint.h>

int main(void) {
  const int32_t axis[] = {0, 100};
  const uint16_t cells[] = {100u, 200u};
  table_view_t curve = {0};
  float output = 0.0F;
  if (table_curve_view_init(&curve, axis, 2u, TABLE_SCALAR_I32, cells, TABLE_SCALAR_U16) != TABLE_OK)
    return 1;
  if (table_curve_lookup(&curve, 50.0F, &output) != TABLE_OK)
    return 2;
  return output == 150.0F ? 0 : 3;
}
