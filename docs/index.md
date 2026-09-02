---
layout: home

hero:
  name: Table.h
  text: Embedded lookup curves and maps
  tagline: Validated linear and bilinear interpolation for C, C++, and Rust without heap allocation in the native core.
  image:
    src: /epicecu-tables-logo.png
    alt: EpicECU Table.h
  actions:
    - theme: brand
      text: Get started
      link: /introduction
    - theme: alt
      text: Browse the API
      link: /reference/c/

features:
  - title: Embedded first
    details: Caller-owned C storage and compile-time-sized C++ storage with no dependency on an operating system or Arduino APIs.
  - title: Validated behaviour
    details: Strict axis validation, finite-value checks, deterministic clamping, and atomic whole-table replacement.
  - title: One implementation
    details: C++, Rust, and optional Protobuf integrations reuse the portable C11 interpolation and validation core.
---
