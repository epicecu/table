# Changelog

## Unreleased

## 1.0.0 - 2026-09-04

- Replaced the legacy C++-only `Table` class with a portable C11 core.
- Added fixed-storage C++11 `Curve` and `Map` facades.
- Added validated typed storage, checked mutation, edge clamping, and binary search.
- Added Arduino, PlatformIO, CMake package, and Taskfile workflows.
- Added reusable Protobuf v1 messages and an optional Nanopb staging adapter.
- Added safe owning Rust bindings and optional Prost snapshot conversions over
  the existing C implementation.
- Added a dedicated blocking MISRA C:2012 analysis task, workflow, and status
  badge for the core C implementation.
- Added a VitePress documentation site with generated native C, C++, and Rust
  API reference pages, version navigation, and GitHub Pages deployment.
- Standardised the public APIs on `Table.h`, `table::`, `table_*`, and
  `table.v1` naming.
- Adopted MIT licensing and Corelib-aligned quality gates.

## 0.3.0

- Final release of the legacy LGPL C++ implementation.
