#include "Table.h"

#include <cstdint>

int main() {
  table::Curve<std::uint8_t, 2u, std::int16_t> curve;
  const std::int16_t axis[] = {0, 10};
  const std::uint8_t cells[] = {10u, 20u};
  return curve.init(axis, cells) == table::Status::Ok ? 0 : 1;
}
