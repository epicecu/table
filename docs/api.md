# API design

## Storage and lifetime

The C API binds borrowed arrays. Immutable views accept constant storage,
including flash-resident calibration data. Mutable bindings require writable
arrays that outlive the binding. The C++ facade owns all arrays at compile-time
fixed dimensions.

The library never allocates or frees application memory. Copying or moving C++
owners is disabled because native descriptors retain pointers to their arrays.

The safe Rust `Curve<X, C>` and `Map<X, Y, C>` owners store dynamic arrays in
boxed slices. Moving the Rust owner therefore does not move the referenced
elements. Cloning reconstructs the native descriptor against newly allocated
storage rather than copying its pointers. The safe API never exposes those
descriptors, and requires exclusive access for mutation.

## Validation and arithmetic

Axes must contain at least one finite point and must be strictly increasing
after conversion to float32. Float cells must be finite. Maps store cells in
row-major `cells[y * x_count + x]` order.

Lookup uses float32 linear or bilinear interpolation. Each coordinate clamps to
the nearest edge. NaN and infinite inputs are rejected. A one-point axis has a
zero interpolation fraction and therefore behaves as a constant.

Individual axis mutation must preserve ordering. Whole-table replacement first
validates a matching candidate and then copies it, so validation errors cannot
partially update the active table. Applications must serialise mutation against
lookup.

## Protobuf snapshots

`table.v1` is the stable interchange contract. Scalar type metadata is
carried in the payload and dimensions derive from axis lengths. Nanopb callbacks
stream values into staging capacity selected by the application.

Nanopb options are code-generation controls, not application metadata.
Programmor configuration owns names, units, ranges, precision, and access rules.

With the Cargo `prost` feature, schema-generated messages are available under
`table::proto::v1`. Converting a snapshot into a typed Rust owner verifies its
declared scalar types, selected value families, integer ranges, dimensions,
finite values, and native Table invariants before returning an active value.
An application schema that imports `table/v1/table.proto` can make Prost reuse
these types with
`Config::extern_path(".table.v1", "::table::proto::v1")`.
