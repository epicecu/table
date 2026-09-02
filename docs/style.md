# Table source style

The repository follows the same compact embedded C and C++ style as Corelib.
C headers use `.h`, C++ headers use `.hpp`, C implementations use `.c`, and C++
implementations use `.cpp`. The core targets C11 and the facade targets C++11.

Complete function declarations and definitions remain on one physical line.
Opening braces attach to declarations. Use two spaces, no tabs, right-bound
pointer and reference symbols, and deterministic include ordering.

Every owned symbol has concise Doxygen documentation. Document parameters,
outputs, return behaviour, borrowed lifetimes, capacity, mutation, concurrency,
and serialisation constraints where relevant. Comments explain intent and
invariants rather than restating code.

Use UK/Australian English in prose, comments, documentation, and diagnostics.
Use conventional US English for source identifiers. Do not rename generated or
protocol-defined identifiers to enforce this convention.

Generated Protobuf/Nanopb files retain generator formatting and must not be
edited manually. Generated API pages below `docs/reference` are derived from
public source comments with `task docs:api` and must not be edited manually.
Apply formatting only to owned source using:

```sh
task quality:format
task quality:format-check
```
