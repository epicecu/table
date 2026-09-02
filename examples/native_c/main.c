#include "Table.h"

#include <stdint.h>
#include <stdio.h>

int main(void) {
  const int32_t rpm[] = {0, 1000, 2000, 3000};
  const float advance[] = {5.0F, 10.0F, 18.0F, 24.0F};
  table_view_t curve = {0};
  float result = 0.0F;
  if (table_curve_view_init(&curve, rpm, 4u, TABLE_SCALAR_I32, advance, TABLE_SCALAR_F32) != TABLE_OK)
    return 1;
  if (table_curve_lookup(&curve, 1500.0F, &result) != TABLE_OK)
    return 2;
  printf("Advance at 1500 rpm: %.1f\n", (double)result);
  return 0;
}
