#include "table/table.hpp"

#include <cstdint>

int main() {
  table::Curve<std::uint16_t, 2u> curve;
  const std::int32_t axis[] = {0, 100};
  const std::uint16_t cells[] = {100u, 200u};
  if (curve.init(axis, cells) != table::Status::Ok)
    return 1;
  float output = 0.0F;
  if (curve.lookup(50.0F, output) != table::Status::Ok)
    return 2;
  return output == 150.0F ? 0 : 3;
}
