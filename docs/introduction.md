# Introduction

Table provides one-dimensional curves and two-dimensional maps for ECU
calibration, sensor conversion, control maps, and other embedded lookup tasks.
The portable implementation is C11, the fixed-storage facade is C++11, and the
safe Rust crate owns dynamically sized storage over the same C implementation.

## Behaviour

- Axes contain at least one finite, strictly increasing point.
- Integer and float32 axes and cells can be mixed independently.
- Lookup uses float32 linear or bilinear interpolation.
- Finite coordinates outside an axis clamp to its nearest edge.
- One-point axes behave as constants.
- Maps store cells in row-major `[y][x]` order.
- Failed validation or replacement does not partially modify active data.

## Storage models

The C API borrows arrays owned by the caller. Immutable views can refer to
flash-resident calibration data, while mutable bindings require writable arrays
that remain alive for the binding's lifetime.

The C++ facade owns fixed-size arrays selected through template parameters. The
safe Rust crate stores axes and cells in boxed slices so the referenced elements
remain stable when an owner moves.

Continue with [installation](./installation), try the [examples](./examples),
or read the [API design contract](./api).
