#include <Table.h>

#include <stdint.h>

table::Map<float, 3, 2, uint16_t, uint8_t> fuelMap;

void setup() {
  Serial.begin(115200);

  const uint16_t rpm[] = {1000, 2000, 3000};
  const uint8_t loadPercent[] = {20, 100};
  const float fuel[2][3] = {
      {2.0F, 2.5F, 3.0F},
      {4.0F, 5.0F, 6.0F},
  };
  if (fuelMap.init(rpm, loadPercent, fuel) != table::Status::Ok) {
    Serial.println("Map validation failed");
    return;
  }

  float result = 0.0F;
  if (fuelMap.lookup(2500.0F, 60.0F, result) == table::Status::Ok) {
    Serial.print("Fuel value: ");
    Serial.println(result);
  }
}

void loop() {}
