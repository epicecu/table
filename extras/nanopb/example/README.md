# Programmor share example

Import the reusable schema into the application share definition:

```proto
import "table/v1/table.proto";

message CalibrationShare {
  table.v1.Map ignition_map = 1;
}
```

Generate both the application schema and `table.proto` with Nanopb 0.4.9.1.
Before encoding the containing share, bind its generated `ignition_map` field
with `table_nanopb_map_encode_init()`.

For an update, bind the generated field to separate caller-owned staging arrays
with `table_nanopb_map_decode_init()`, decode the complete share, call
`table_nanopb_map_decode_finish()`, and commit the returned view using
`table_replace()`. Never decode directly into the active calibration.

Nanopb `.options` files control generated C storage and callbacks. Labels,
units, precision, limits, and editor behaviour belong in Programmor device
configuration rather than the wire payload or Nanopb options.
