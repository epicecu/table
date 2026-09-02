<div align="center">

<img src="support/epicecu-tables-logo.png" alt="Table" width="400" />

### Heap-free lookup curves and maps for embedded C and C++

</div>

[![CI](https://github.com/epicecu/table/actions/workflows/ci.yml/badge.svg)](https://github.com/epicecu/table/actions/workflows/ci.yml)
[![MISRA C Analysis](https://github.com/epicecu/table/actions/workflows/misra.yml/badge.svg)](https://github.com/epicecu/table/actions/workflows/misra.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Table provides validated linear and bilinear interpolation for ECU
calibration, sensor conversion, control maps, and other embedded lookup tasks.
The portable core is C11, the fixed-storage facade is C++11, and neither path
allocates heap memory.

## Highlights

- Caller-owned C storage and compile-time-sized C++ storage.
- Independent fixed-width integer or float32 axes and cells.
- Strictly increasing axis validation and checked mutation.
- Deterministic edge clamping and float32 interpolation.
- Immutable views for flash-resident calibration data.
- Arduino and PlatformIO compatibility plus installable CMake packages.
- Optional reusable Protobuf messages and Nanopb callbacks for Programmor shares.
- Safe owning Rust bindings over the same C implementation.

## C++ curve

```cpp
#include <Table.h>

#include <cstdint>

table::Curve<std::uint8_t, 4, std::int16_t> throttle;

const std::int16_t millivolts[] = {500, 1500, 2500, 4500};
const std::uint8_t percent[] = {0, 25, 55, 100};

if (throttle.init(millivolts, percent) == table::Status::Ok) {
  float result = 0.0F;
  throttle.lookup(2000.0F, result);
}
```

## C map

```c
#include <Table.h>

#include <stdint.h>

const uint16_t rpm[] = {1000u, 2000u, 3000u};
const uint8_t load[] = {20u, 100u};
const float cells[] = {
    2.0F, 2.5F, 3.0F,
    4.0F, 5.0F, 6.0F,
};

table_view_t map = {0};
float result = 0.0F;

if (table_map_view_init(&map, rpm, 3u, TABLE_SCALAR_U16,
                        load, 2u, TABLE_SCALAR_U8,
                        cells, TABLE_SCALAR_F32) == TABLE_OK) {
  table_map_lookup(&map, 2500.0F, 60.0F, &result);
}
```

Map cells use row-major `[y][x]` order. A curve requires at least one X point;
a map requires at least one point on each axis. One-point axes behave as
constants. Finite inputs below or above an axis are clamped to its nearest edge.

Supported storage types are signed and unsigned 8-, 16-, and 32-bit integers
and float32. Lookup converts points to float32. Axis values must remain distinct
after that conversion, which prevents a zero interpolation interval when large
32-bit integers are too closely spaced.

## Arduino and PlatformIO

Add the repository or a release archive to an Arduino project manually, or use
the repository as a PlatformIO library dependency, then include `<Table.h>`.
The library contains no Arduino API dependency and also works in
framework-neutral CMake projects. Examples are available under
`examples/Curve` and `examples/Map`.

The repository does not use PlatformIO for its own development workflow. Its
standard Arduino package metadata allows both build systems to consume it
normally. The generic `Table` distribution name is not submitted to Arduino
Library Manager.

## Rust

The Cargo workspace provides a safe `table` crate backed by the existing C11
implementation. It owns dynamically sized axis and cell storage while
preserving the native validation, lookup, mutation, and replacement behaviour:

```rust
use table::Map;

let mut fuel = Map::new(
    vec![1_000_u16, 2_000, 3_000],
    vec![20_u8, 100],
    vec![2.0_f32, 2.5, 3.0, 4.0, 5.0, 6.0],
)
.unwrap();

assert_eq!(fuel.lookup(2_500.0, 60.0).unwrap(), 4.125);
fuel.set_cell(1, 1, 5.5).unwrap();
```

Enable the optional `prost` feature for generated `table.v1` messages and
validated snapshot conversions. Git consumers can select the safe package and
pin an exact repository revision:

```toml
[dependencies]
table = { git = "https://github.com/epicecu/table.git", rev = "<commit>", features = ["prost"] }
```

The raw `table-sys` package is intended for language-integration work; ordinary
applications should use `table`.

## Programmor Protobuf shares

The optional schema defines reusable `table.v1.Curve` and
`table.v1.Map` messages:

```proto
import "table/v1/table.proto";

message CalibrationShare {
  table.v1.Map ignition_map = 1;
}
```

The Nanopb adapter encodes from immutable table views and decodes into separate
caller-owned staging arrays. The application validates the complete snapshot
before replacing an active calibration, so an invalid or truncated update does
not partially modify live data.

Nanopb `.options` control generated embedded storage and callbacks only.
Programmor device configuration owns labels, units, precision, display limits,
access rules, and table-editor behaviour.

The base Arduino and CMake library does not depend on Nanopb. Enable
`TABLE_BUILD_NANOPB` only when a compatible Nanopb 0.4.9.1 target is
available. See `extras/nanopb/example` for the integration sequence.

## Development

The Taskfile is the supported development entry point:

```sh
task quick                 # Build and run native and Rust tests.
task check                 # Strict GCC and Clang builds.
task quality:format-check  # Verify Corelib-aligned source style.
task quality:misra         # Run core MISRA C:2012 analysis.
task quality:coverage      # Enforce 85% owned-source line coverage.
task all                   # Run the complete release gate.
```

`task rust:all` runs formatting, generated-schema, Clippy, documentation, and
test checks specifically for the Rust binding.

`task quality:misra` is a blocking, open-source Cppcheck regression gate over
the core C implementation. It provides partial MISRA C:2012 automated analysis,
not a formal compliance claim; see [the scope and accepted findings](docs/misra.md).

Build output and bootstrapped tools remain below `build/`. The optional Nanopb
runtime and test-only GoogleTest dependency are fetched only for development.

## Integration constraints

- Lookups through an immutable view are re-entrant and do not mutate state.
- Mutations and whole-table replacement must not race with lookup.
- Table descriptors and native arrays are not persistent or wire formats.
- Protobuf v1 snapshots are the stable portable interchange contract.
- A failed API call leaves lookup output or active table data unchanged unless
  documented otherwise.

See [API design](docs/api.md), [source style](docs/style.md), and
[migration guidance](docs/migration.md) for the complete contracts.

## Licence

Table 1.0.0 and later are available under the [MIT Licence](LICENSE).
Earlier published releases retain the licence under which they were released.
