# Protobuf and Nanopb

The optional schema defines stable `table.v1.Curve` and `table.v1.Map`
snapshots. Applications can import those messages into a larger protocol
without treating native descriptors or arrays as wire formats.

```proto
import "table/v1/table.proto";

message CalibrationShare {
  table.v1.Map ignition_map = 1;
}
```

## Embedded Nanopb

The Nanopb adapter encodes immutable views and decodes into separate
caller-owned staging arrays. Validate a complete staged snapshot before
replacing the active calibration so malformed or truncated data cannot produce
a partial update.

Nanopb `.options` control generated storage and callbacks only. Application
configuration owns labels, units, precision, display limits, and access rules.

## Rust Prost support

Enable the `prost` feature to expose generated messages under
`table::proto::v1`. Converting a message into a typed owner verifies scalar
metadata, value families, ranges, dimensions, finite values, and native Table
invariants before returning the active value.

See the [Rust Protobuf API](./reference/rust/protobuf) and the
[embedded integration sequence](https://github.com/epicecu/table/tree/main/extras/nanopb/example).
