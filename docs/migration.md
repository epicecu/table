# Migrating from 0.3

Version 1.0 is an intentional source and storage break. The `Table.h` entry
point remains, but the legacy global `Table` class does not. Choose
`table::Curve` for one input or `table::Map` for two inputs.

- Replace `initialise()` and repeated setters with one validated `init()` call.
- Replace `getValue()` with `lookup()` and handle its explicit `Status`.
- Expect finite out-of-range input to clamp instead of returning `-1`.
- Store map cells in row-major `[y][x]` order.
- Replace `setValueByIndex()` with checked `setCell()`.
- Do not persist object memory or use the removed `saveData()`/`loadData()` API.
- Use the Protobuf v1 snapshot contract for portable Programmor interchange.

The legacy cache was removed. Lookup is const, re-entrant, and uses binary axis
searches. Existing firmware should migrate deliberately rather than attempting
to reinterpret old object or EEPROM bytes.
