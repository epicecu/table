#include <Table.h>

#include <stdint.h>

table::Curve<uint8_t, 4, int16_t> throttleCurve;

void setup() {
  Serial.begin(115200);

  const int16_t sensorMillivolts[] = {500, 1500, 2500, 4500};
  const uint8_t throttlePercent[] = {0, 25, 55, 100};
  if (throttleCurve.init(sensorMillivolts, throttlePercent) != table::Status::Ok) {
    Serial.println("Curve validation failed");
    return;
  }

  float result = 0.0F;
  if (throttleCurve.lookup(2000.0F, result) == table::Status::Ok) {
    Serial.print("Throttle: ");
    Serial.println(result);
  }
}

void loop() {}
