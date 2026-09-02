// SPDX-License-Identifier: MIT
/**
 * @file table.hpp
 * @brief Fixed-storage C++11 facade for the Table API.
 */
#ifndef TABLE_TABLE_HPP
#define TABLE_TABLE_HPP

#include "table/table.h"

#include <stddef.h>
#include <stdint.h>

#if __cplusplus < 201103L
#error "Table C++ facade requires C++11 or newer"
#endif

/** @brief Type-safe Table facade. */
namespace table {

/** @brief Type-safe table operation status. */
enum class Status : uint8_t {
  Ok = TABLE_OK,
  InvalidArgument = TABLE_INVALID_ARGUMENT,
  InvalidState = TABLE_INVALID_STATE,
  InvalidAxis = TABLE_INVALID_AXIS,
  IndexOutOfRange = TABLE_INDEX_OUT_OF_RANGE,
  TypeMismatch = TABLE_TYPE_MISMATCH
};

namespace detail {

/** @brief Converts a native operation status. @param value Native status. @return Type-safe status. */
inline Status status(table_status_t value) noexcept {
  return static_cast<Status>(value);
}

/** @brief Maps supported C++ scalar types to the C storage contract. */
template <typename T>
struct ScalarTraits;

/** @brief Signed 8-bit scalar mapping. */
template <>
struct ScalarTraits<int8_t> {
  /** @brief Returns the native storage type. @return Native storage type. */
  static table_scalar_type_t type() noexcept { return TABLE_SCALAR_I8; }
  /** @brief Wraps a C++ value. @param value Value to wrap. @return Native scalar. */
  static table_scalar_t make(int8_t value) noexcept {
    table_scalar_t result{};
    result.type = type();
    result.value.i8 = value;
    return result;
  }
  /** @brief Reads a wrapped value. @param value Native scalar. @return C++ value. */
  static int8_t read(table_scalar_t value) noexcept { return value.value.i8; }
};

/** @brief Unsigned 8-bit scalar mapping. */
template <>
struct ScalarTraits<uint8_t> {
  /** @brief Returns the native storage type. @return Native storage type. */
  static table_scalar_type_t type() noexcept { return TABLE_SCALAR_U8; }
  /** @brief Wraps a C++ value. @param value Value to wrap. @return Native scalar. */
  static table_scalar_t make(uint8_t value) noexcept {
    table_scalar_t result{};
    result.type = type();
    result.value.u8 = value;
    return result;
  }
  /** @brief Reads a wrapped value. @param value Native scalar. @return C++ value. */
  static uint8_t read(table_scalar_t value) noexcept { return value.value.u8; }
};

/** @brief Signed 16-bit scalar mapping. */
template <>
struct ScalarTraits<int16_t> {
  /** @brief Returns the native storage type. @return Native storage type. */
  static table_scalar_type_t type() noexcept { return TABLE_SCALAR_I16; }
  /** @brief Wraps a C++ value. @param value Value to wrap. @return Native scalar. */
  static table_scalar_t make(int16_t value) noexcept {
    table_scalar_t result{};
    result.type = type();
    result.value.i16 = value;
    return result;
  }
  /** @brief Reads a wrapped value. @param value Native scalar. @return C++ value. */
  static int16_t read(table_scalar_t value) noexcept { return value.value.i16; }
};

/** @brief Unsigned 16-bit scalar mapping. */
template <>
struct ScalarTraits<uint16_t> {
  /** @brief Returns the native storage type. @return Native storage type. */
  static table_scalar_type_t type() noexcept { return TABLE_SCALAR_U16; }
  /** @brief Wraps a C++ value. @param value Value to wrap. @return Native scalar. */
  static table_scalar_t make(uint16_t value) noexcept {
    table_scalar_t result{};
    result.type = type();
    result.value.u16 = value;
    return result;
  }
  /** @brief Reads a wrapped value. @param value Native scalar. @return C++ value. */
  static uint16_t read(table_scalar_t value) noexcept { return value.value.u16; }
};

/** @brief Signed 32-bit scalar mapping. */
template <>
struct ScalarTraits<int32_t> {
  /** @brief Returns the native storage type. @return Native storage type. */
  static table_scalar_type_t type() noexcept { return TABLE_SCALAR_I32; }
  /** @brief Wraps a C++ value. @param value Value to wrap. @return Native scalar. */
  static table_scalar_t make(int32_t value) noexcept {
    table_scalar_t result{};
    result.type = type();
    result.value.i32 = value;
    return result;
  }
  /** @brief Reads a wrapped value. @param value Native scalar. @return C++ value. */
  static int32_t read(table_scalar_t value) noexcept { return value.value.i32; }
};

/** @brief Unsigned 32-bit scalar mapping. */
template <>
struct ScalarTraits<uint32_t> {
  /** @brief Returns the native storage type. @return Native storage type. */
  static table_scalar_type_t type() noexcept { return TABLE_SCALAR_U32; }
  /** @brief Wraps a C++ value. @param value Value to wrap. @return Native scalar. */
  static table_scalar_t make(uint32_t value) noexcept {
    table_scalar_t result{};
    result.type = type();
    result.value.u32 = value;
    return result;
  }
  /** @brief Reads a wrapped value. @param value Native scalar. @return C++ value. */
  static uint32_t read(table_scalar_t value) noexcept { return value.value.u32; }
};

/** @brief Single-precision scalar mapping. */
template <>
struct ScalarTraits<float> {
  /** @brief Returns the native storage type. @return Native storage type. */
  static table_scalar_type_t type() noexcept { return TABLE_SCALAR_F32; }
  /** @brief Wraps a C++ value. @param value Value to wrap. @return Native scalar. */
  static table_scalar_t make(float value) noexcept {
    table_scalar_t result{};
    result.type = type();
    result.value.f32 = value;
    return result;
  }
  /** @brief Reads a wrapped value. @param value Native scalar. @return C++ value. */
  static float read(table_scalar_t value) noexcept { return value.value.f32; }
};

/** @brief Detects scalar types supported by the native core. */
template <typename T>
struct IsSupportedScalar {
  /** @brief Whether the type is supported. */
  static const bool value = false;
};

/** @brief Marks signed 8-bit storage as supported. */
template <>
struct IsSupportedScalar<int8_t> {
  /** @brief Whether the type is supported. */
  static const bool value = true;
};

/** @brief Marks unsigned 8-bit storage as supported. */
template <>
struct IsSupportedScalar<uint8_t> {
  /** @brief Whether the type is supported. */
  static const bool value = true;
};

/** @brief Marks signed 16-bit storage as supported. */
template <>
struct IsSupportedScalar<int16_t> {
  /** @brief Whether the type is supported. */
  static const bool value = true;
};

/** @brief Marks unsigned 16-bit storage as supported. */
template <>
struct IsSupportedScalar<uint16_t> {
  /** @brief Whether the type is supported. */
  static const bool value = true;
};

/** @brief Marks signed 32-bit storage as supported. */
template <>
struct IsSupportedScalar<int32_t> {
  /** @brief Whether the type is supported. */
  static const bool value = true;
};

/** @brief Marks unsigned 32-bit storage as supported. */
template <>
struct IsSupportedScalar<uint32_t> {
  /** @brief Whether the type is supported. */
  static const bool value = true;
};

/** @brief Marks single-precision storage as supported. */
template <>
struct IsSupportedScalar<float> {
  /** @brief Whether the type is supported. */
  static const bool value = true;
};

} // namespace detail

/**
 * @brief Owning fixed-size one-axis lookup curve.
 * @tparam ValueT Cell storage type.
 * @tparam XCount Number of points.
 * @tparam AxisT X-axis storage type.
 */
template <typename ValueT, size_t XCount, typename AxisT = int32_t>
class Curve final {
  static_assert(XCount > 0u, "a curve requires at least one point");
  static_assert(detail::IsSupportedScalar<ValueT>::value, "unsupported curve cell type");
  static_assert(detail::IsSupportedScalar<AxisT>::value, "unsupported curve axis type");

public:
  /** @brief Constructs an uninitialised curve with zeroed storage. */
  Curve() = default;
  Curve(const Curve &) = delete;
  Curve &operator=(const Curve &) = delete;
  Curve(Curve &&) = delete;
  Curve &operator=(Curve &&) = delete;

  /** @brief Copies and validates a complete curve. @param[in] xAxis Strictly increasing X-axis points. @param[in] cells Cell values. @return Operation status. */
  Status init(const AxisT (&xAxis)[XCount], const ValueT (&cells)[XCount]) noexcept {
    table_view_t candidate{};
    table_status_t result = table_curve_view_init(&candidate, xAxis, XCount, detail::ScalarTraits<AxisT>::type(), cells, detail::ScalarTraits<ValueT>::type());
    if (result != TABLE_OK)
      return detail::status(result);
    if (ready_)
      return detail::status(table_replace(&native_, &candidate));
    for (size_t index = 0u; index < XCount; ++index) {
      xAxis_[index] = xAxis[index];
      cells_[index] = cells[index];
    }
    result = table_curve_init(&native_, xAxis_, XCount, detail::ScalarTraits<AxisT>::type(), cells_, detail::ScalarTraits<ValueT>::type());
    ready_ = result == TABLE_OK;
    return detail::status(result);
  }

  /** @brief Tests whether initialisation succeeded. @return True when ready for lookup. */
  bool isReady() const noexcept { return ready_; }

  /** @brief Performs a clamped lookup. @param x Finite X coordinate. @param[out] output Result written only on success. @return Operation status. */
  Status lookup(float x, float &output) const noexcept {
    return ready_ ? detail::status(table_curve_lookup(&native_.view, x, &output)) : Status::InvalidState;
  }

  /** @brief Replaces an X-axis point while preserving ordering. @param index Point index. @param value New value. @return Operation status. */
  Status setX(size_t index, AxisT value) noexcept {
    return ready_ ? detail::status(table_set_x(&native_, index, detail::ScalarTraits<AxisT>::make(value))) : Status::InvalidState;
  }

  /** @brief Replaces a cell. @param index Point index. @param value New value. @return Operation status. */
  Status setCell(size_t index, ValueT value) noexcept {
    return ready_ ? detail::status(table_curve_set_cell(&native_, index, detail::ScalarTraits<ValueT>::make(value))) : Status::InvalidState;
  }

  /** @brief Reads an X-axis point. @param index Point index. @param[out] value Value written only on success. @return Operation status. */
  Status x(size_t index, AxisT &value) const noexcept {
    table_scalar_t scalar{};
    if (!ready_)
      return Status::InvalidState;
    const table_status_t result = table_get_x(&native_.view, index, &scalar);
    if (result == TABLE_OK)
      value = detail::ScalarTraits<AxisT>::read(scalar);
    return detail::status(result);
  }

  /** @brief Reads a cell. @param index Point index. @param[out] value Value written only on success. @return Operation status. */
  Status cell(size_t index, ValueT &value) const noexcept {
    table_scalar_t scalar{};
    if (!ready_)
      return Status::InvalidState;
    const table_status_t result = table_curve_get_cell(&native_.view, index, &scalar);
    if (result == TABLE_OK)
      value = detail::ScalarTraits<ValueT>::read(scalar);
    return detail::status(result);
  }

  /** @brief Accesses the immutable native view. @return View pointer, or null before initialisation. */
  const table_view_t *nativeView() const noexcept { return ready_ ? &native_.view : nullptr; }

private:
  AxisT xAxis_[XCount]{};
  ValueT cells_[XCount]{};
  table_mutable_t native_{};
  bool ready_{false};
};

/**
 * @brief Owning fixed-size two-axis lookup map.
 * @tparam ValueT Cell storage type.
 * @tparam XCount Number of X-axis points.
 * @tparam YCount Number of Y-axis points.
 * @tparam XAxisT X-axis storage type.
 * @tparam YAxisT Y-axis storage type.
 */
template <typename ValueT, size_t XCount, size_t YCount, typename XAxisT = int32_t, typename YAxisT = int32_t>
class Map final {
  static_assert(XCount > 0u, "a map requires at least one X-axis point");
  static_assert(YCount > 0u, "a map requires at least one Y-axis point");
  static_assert(XCount <= static_cast<size_t>(-1) / YCount, "map dimensions overflow size_t");
  static_assert(detail::IsSupportedScalar<ValueT>::value, "unsupported map cell type");
  static_assert(detail::IsSupportedScalar<XAxisT>::value, "unsupported map X-axis type");
  static_assert(detail::IsSupportedScalar<YAxisT>::value, "unsupported map Y-axis type");

public:
  /** @brief Constructs an uninitialised map with zeroed storage. */
  Map() = default;
  Map(const Map &) = delete;
  Map &operator=(const Map &) = delete;
  Map(Map &&) = delete;
  Map &operator=(Map &&) = delete;

  /** @brief Copies and validates a complete row-major map. @param[in] xAxis Strictly increasing X-axis points. @param[in] yAxis Strictly increasing Y-axis points. @param[in] cells Flat row-major cells. @return Operation status. */
  Status init(const XAxisT (&xAxis)[XCount], const YAxisT (&yAxis)[YCount], const ValueT (&cells)[XCount * YCount]) noexcept {
    return initData(xAxis, yAxis, cells);
  }

  /** @brief Copies and validates a complete two-dimensional row-major map. @param[in] xAxis Strictly increasing X-axis points. @param[in] yAxis Strictly increasing Y-axis points. @param[in] cells Cells indexed as `[y][x]`. @return Operation status. */
  Status init(const XAxisT (&xAxis)[XCount], const YAxisT (&yAxis)[YCount], const ValueT (&cells)[YCount][XCount]) noexcept {
    return initData(xAxis, yAxis, &cells[0][0]);
  }

  /** @brief Tests whether initialisation succeeded. @return True when ready for lookup. */
  bool isReady() const noexcept { return ready_; }

  /** @brief Performs a clamped bilinear lookup. @param x Finite X coordinate. @param y Finite Y coordinate. @param[out] output Result written only on success. @return Operation status. */
  Status lookup(float x, float y, float &output) const noexcept {
    return ready_ ? detail::status(table_map_lookup(&native_.view, x, y, &output)) : Status::InvalidState;
  }

  /** @brief Replaces an X-axis point while preserving ordering. @param index Point index. @param value New value. @return Operation status. */
  Status setX(size_t index, XAxisT value) noexcept {
    return ready_ ? detail::status(table_set_x(&native_, index, detail::ScalarTraits<XAxisT>::make(value))) : Status::InvalidState;
  }

  /** @brief Replaces a Y-axis point while preserving ordering. @param index Point index. @param value New value. @return Operation status. */
  Status setY(size_t index, YAxisT value) noexcept {
    return ready_ ? detail::status(table_set_y(&native_, index, detail::ScalarTraits<YAxisT>::make(value))) : Status::InvalidState;
  }

  /** @brief Replaces a map cell. @param xIndex X index. @param yIndex Y index. @param value New value. @return Operation status. */
  Status setCell(size_t xIndex, size_t yIndex, ValueT value) noexcept {
    return ready_ ? detail::status(table_map_set_cell(&native_, xIndex, yIndex, detail::ScalarTraits<ValueT>::make(value))) : Status::InvalidState;
  }

  /** @brief Reads an X-axis point. @param index Point index. @param[out] value Value written only on success. @return Operation status. */
  Status x(size_t index, XAxisT &value) const noexcept {
    table_scalar_t scalar{};
    if (!ready_)
      return Status::InvalidState;
    const table_status_t result = table_get_x(&native_.view, index, &scalar);
    if (result == TABLE_OK)
      value = detail::ScalarTraits<XAxisT>::read(scalar);
    return detail::status(result);
  }

  /** @brief Reads a Y-axis point. @param index Point index. @param[out] value Value written only on success. @return Operation status. */
  Status y(size_t index, YAxisT &value) const noexcept {
    table_scalar_t scalar{};
    if (!ready_)
      return Status::InvalidState;
    const table_status_t result = table_get_y(&native_.view, index, &scalar);
    if (result == TABLE_OK)
      value = detail::ScalarTraits<YAxisT>::read(scalar);
    return detail::status(result);
  }

  /** @brief Reads a map cell. @param xIndex X index. @param yIndex Y index. @param[out] value Value written only on success. @return Operation status. */
  Status cell(size_t xIndex, size_t yIndex, ValueT &value) const noexcept {
    table_scalar_t scalar{};
    if (!ready_)
      return Status::InvalidState;
    const table_status_t result = table_map_get_cell(&native_.view, xIndex, yIndex, &scalar);
    if (result == TABLE_OK)
      value = detail::ScalarTraits<ValueT>::read(scalar);
    return detail::status(result);
  }

  /** @brief Accesses the immutable native view. @return View pointer, or null before initialisation. */
  const table_view_t *nativeView() const noexcept { return ready_ ? &native_.view : nullptr; }

private:
  Status initData(const XAxisT (&xAxis)[XCount], const YAxisT (&yAxis)[YCount], const ValueT *cells) noexcept {
    table_view_t candidate{};
    table_status_t result = table_map_view_init(&candidate, xAxis, XCount, detail::ScalarTraits<XAxisT>::type(), yAxis, YCount, detail::ScalarTraits<YAxisT>::type(), cells, detail::ScalarTraits<ValueT>::type());
    if (result != TABLE_OK)
      return detail::status(result);
    if (ready_)
      return detail::status(table_replace(&native_, &candidate));
    for (size_t index = 0u; index < XCount; ++index)
      xAxis_[index] = xAxis[index];
    for (size_t index = 0u; index < YCount; ++index)
      yAxis_[index] = yAxis[index];
    for (size_t index = 0u; index < XCount * YCount; ++index)
      cells_[index] = cells[index];
    result = table_map_init(&native_, xAxis_, XCount, detail::ScalarTraits<XAxisT>::type(), yAxis_, YCount, detail::ScalarTraits<YAxisT>::type(), cells_, detail::ScalarTraits<ValueT>::type());
    ready_ = result == TABLE_OK;
    return detail::status(result);
  }

  XAxisT xAxis_[XCount]{};
  YAxisT yAxis_[YCount]{};
  ValueT cells_[XCount * YCount]{};
  table_mutable_t native_{};
  bool ready_{false};
};

} // namespace table

#endif
