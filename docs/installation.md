# Installation

## Arduino and PlatformIO

Use the repository URL or a release archive as a library dependency, then
include the framework-neutral umbrella header:

```cpp
#include <Table.h>
```

For PlatformIO, pin a release or commit in `platformio.ini`:

```ini
lib_deps =
  https://github.com/epicecu/table.git#<tag-or-commit>
```

The generic `Table` distribution name is not submitted to Arduino Library
Manager, so repository or archive installation is intentional.

## CMake

Add the repository directly:

```cmake
add_subdirectory(path/to/table)
target_link_libraries(application PRIVATE Table::Table)
```

An installed package can instead be consumed with:

```cmake
find_package(Table 1 CONFIG REQUIRED)
target_link_libraries(application PRIVATE Table::Table)
```

Enable `TABLE_BUILD_NANOPB` only when providing a compatible Nanopb target or
`TABLE_NANOPB_ROOT`.

## Rust

Git consumers should select the safe package and pin an exact revision:

```toml
[dependencies]
table = { git = "https://github.com/epicecu/table.git", rev = "<commit>" }
```

Add `features = ["prost"]` when consuming the stable `table.v1` snapshot
messages. The raw `table-sys` package is intended for language integration.
