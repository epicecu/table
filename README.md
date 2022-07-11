<div align="center">

<img src="support/epicecu-tables-logo.png" alt="EpicECU Tables" width="400" />

##### An open source Table library supporting 2d and 3d modes. This is useful for lookups e.g. fuel, timing maps.

</div>

[![Tests](https://github.com/epicecu/table/actions/workflows/unit_tests.yml/badge.svg?branch=main)](https://github.com/epicecu/table/actions/workflows/unit_tests.yml)

## Information

The library uses modern cpp templating features to allocate the correct data size during compilation. The X and Y axis data is stored as integers and the table data is stored using the templated data type.

Note: Store the axis data in a linear fashion from smallest to largest starting from the origin position. This is to ensure that interpolation can be processed between axis data points.

```

Table<data type, xSzie, ySize> 3dTable;

Table<data type, xSzie> 2dTable;

```

## Exmaple

See the `/examples` folder for an Ardunio example.

## Maintainers

- [David Cedar](https://github.com/devvid)

## License

GPL V2 © [Programmor](https://github.com/epicecu/table)
