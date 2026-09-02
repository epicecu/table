#include "Table.h"

#include <cstdint>
#include <iostream>

int main() {
  table::Map<float, 3u, 2u, std::uint16_t, std::uint16_t> fuelMap;
  const std::uint16_t rpm[] = {1000u, 2000u, 3000u};
  const std::uint16_t load[] = {20u, 100u};
  const float fuel[2][3] = {{2.0F, 2.5F, 3.0F}, {4.0F, 5.0F, 6.0F}};
  if (fuelMap.init(rpm, load, fuel) != table::Status::Ok)
    return 1;
  float result = 0.0F;
  if (fuelMap.lookup(2500.0F, 60.0F, result) != table::Status::Ok)
    return 2;
  std::cout << "Fuel value: " << result << '\n';
  return 0;
}
