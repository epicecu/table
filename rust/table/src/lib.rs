// SPDX-License-Identifier: MIT
//! Safe owning Rust bindings for the Table C lookup library.
//!
//! The interpolation, validation, mutation, and replacement operations are
//! implemented by the bundled C core. Rust owns the backing arrays and keeps
//! their addresses stable for the lifetime of each table.

#![warn(missing_docs)]

mod scalar;

#[cfg(feature = "prost")]
pub mod proto;

use std::{fmt, mem::MaybeUninit};

pub use scalar::{Scalar, ScalarType};
use table_sys as sys;

/// A failed Table operation.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[non_exhaustive]
pub enum Error {
    /// A pointer, count, coordinate, or finite-value requirement was violated.
    InvalidArgument,
    /// A native descriptor was not initialised for the requested operation.
    InvalidState,
    /// An axis was empty, non-finite, unordered, or collided after float32 conversion.
    InvalidAxis,
    /// An axis or cell index exceeded the table dimensions.
    IndexOutOfRange,
    /// A value or replacement used a different scalar representation.
    TypeMismatch,
    /// Rust slices did not describe the required curve or map shape.
    DimensionMismatch,
    /// Multiplying map dimensions overflowed `usize`.
    SizeOverflow,
}

impl fmt::Display for Error {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        formatter.write_str(match self {
            Self::InvalidArgument => "invalid Table argument",
            Self::InvalidState => "Table is not in a valid state",
            Self::InvalidAxis => "Table axis is invalid",
            Self::IndexOutOfRange => "Table index is out of range",
            Self::TypeMismatch => "Table scalar type does not match",
            Self::DimensionMismatch => "Table dimensions do not match",
            Self::SizeOverflow => "Table dimensions overflow the platform size",
        })
    }
}

impl std::error::Error for Error {}

fn result(status: sys::table_status_t) -> Result<(), Error> {
    match status {
        sys::table_status_t::TABLE_OK => Ok(()),
        sys::table_status_t::TABLE_INVALID_ARGUMENT => Err(Error::InvalidArgument),
        sys::table_status_t::TABLE_INVALID_STATE => Err(Error::InvalidState),
        sys::table_status_t::TABLE_INVALID_AXIS => Err(Error::InvalidAxis),
        sys::table_status_t::TABLE_INDEX_OUT_OF_RANGE => Err(Error::IndexOutOfRange),
        sys::table_status_t::TABLE_TYPE_MISMATCH => Err(Error::TypeMismatch),
    }
}

/// An owning, dynamically sized one-axis lookup curve.
pub struct Curve<X: Scalar, C: Scalar> {
    x_axis: Box<[X]>,
    cells: Box<[C]>,
    native: sys::table_mutable_t,
}

impl<X: Scalar, C: Scalar> Curve<X, C> {
    /// Creates and validates a curve with one cell per X-axis point.
    pub fn new(x_axis: Vec<X>, cells: Vec<C>) -> Result<Self, Error> {
        if x_axis.len() != cells.len() {
            return Err(Error::DimensionMismatch);
        }
        let mut x_axis = x_axis.into_boxed_slice();
        let mut cells = cells.into_boxed_slice();
        let mut native = MaybeUninit::<sys::table_mutable_t>::uninit();
        // SAFETY: the slices are contiguous, correctly typed, and retained by
        // the returned owner. The C initializer writes the descriptor only on
        // success and validates every element before returning.
        let status = unsafe {
            sys::table_curve_init(
                native.as_mut_ptr(),
                x_axis.as_mut_ptr().cast(),
                x_axis.len(),
                X::NATIVE_TYPE,
                cells.as_mut_ptr().cast(),
                C::NATIVE_TYPE,
            )
        };
        result(status)?;
        // SAFETY: a successful initializer wrote the complete descriptor.
        let native = unsafe { native.assume_init() };
        Ok(Self {
            x_axis,
            cells,
            native,
        })
    }

    /// Returns the number of X-axis points and cells.
    pub fn len(&self) -> usize {
        self.x_axis.len()
    }

    /// Returns whether the curve has no points.
    ///
    /// Valid curves are never empty, but this complements [`Self::len`].
    pub fn is_empty(&self) -> bool {
        self.x_axis.is_empty()
    }

    /// Returns the typed X-axis storage.
    pub fn x_axis(&self) -> &[X] {
        &self.x_axis
    }

    /// Returns the typed cell storage.
    pub fn cells(&self) -> &[C] {
        &self.cells
    }

    /// Revalidates all native table invariants.
    pub fn validate(&self) -> Result<(), Error> {
        // SAFETY: the descriptor points to the retained boxed slices.
        result(unsafe { sys::table_validate(&raw const self.native.view) })
    }

    /// Performs a clamped linear lookup at `x`.
    pub fn lookup(&self, x: f32) -> Result<f32, Error> {
        let mut output = 0.0;
        // SAFETY: the descriptor and output pointer are valid for the call.
        result(unsafe {
            sys::table_curve_lookup(&raw const self.native.view, x, &raw mut output)
        })?;
        Ok(output)
    }

    /// Reads one X-axis point through the native API.
    pub fn x(&self, index: usize) -> Result<X, Error> {
        let mut value = MaybeUninit::<sys::table_scalar_t>::uninit();
        // SAFETY: C writes a scalar on success and the descriptor is valid.
        result(unsafe {
            sys::table_get_x(&raw const self.native.view, index, value.as_mut_ptr())
        })?;
        // SAFETY: success initialized the scalar with X's native type.
        Ok(unsafe { X::from_native(value.assume_init()) })
    }

    /// Reads one curve cell through the native API.
    pub fn cell(&self, index: usize) -> Result<C, Error> {
        let mut value = MaybeUninit::<sys::table_scalar_t>::uninit();
        // SAFETY: C writes a scalar on success and the descriptor is valid.
        result(unsafe {
            sys::table_curve_get_cell(&raw const self.native.view, index, value.as_mut_ptr())
        })?;
        // SAFETY: success initialized the scalar with C's native type.
        Ok(unsafe { C::from_native(value.assume_init()) })
    }

    /// Replaces one X-axis point while preserving strict ordering.
    pub fn set_x(&mut self, index: usize, value: X) -> Result<(), Error> {
        // SAFETY: exclusive Rust access prevents lookup or another mutation.
        result(unsafe { sys::table_set_x(&raw mut self.native, index, value.into_native()) })
    }

    /// Replaces one curve cell.
    pub fn set_cell(&mut self, index: usize, value: C) -> Result<(), Error> {
        // SAFETY: exclusive Rust access prevents lookup or another mutation.
        result(unsafe {
            sys::table_curve_set_cell(&raw mut self.native, index, value.into_native())
        })
    }

    /// Atomically validates and copies a complete same-sized curve snapshot.
    pub fn replace(&mut self, x_axis: &[X], cells: &[C]) -> Result<(), Error> {
        if x_axis.len() != self.len() || cells.len() != self.len() {
            return Err(Error::DimensionMismatch);
        }
        let mut candidate = MaybeUninit::<sys::table_view_t>::uninit();
        // SAFETY: the borrowed slices remain alive for initialization and the
        // immediately following copy into the owner's stable storage.
        result(unsafe {
            sys::table_curve_view_init(
                candidate.as_mut_ptr(),
                x_axis.as_ptr().cast(),
                x_axis.len(),
                X::NATIVE_TYPE,
                cells.as_ptr().cast(),
                C::NATIVE_TYPE,
            )
        })?;
        // SAFETY: initialization succeeded and exclusive access protects the
        // active table while C validates and copies the complete candidate.
        result(unsafe { sys::table_replace(&raw mut self.native, candidate.as_ptr()) })
    }
}

impl<X: Scalar, C: Scalar> Clone for Curve<X, C> {
    fn clone(&self) -> Self {
        Self::new(self.x_axis.to_vec(), self.cells.to_vec())
            .expect("a clone of a valid curve remains valid")
    }
}

impl<X: Scalar, C: Scalar> fmt::Debug for Curve<X, C> {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        formatter
            .debug_struct("Curve")
            .field("x_axis", &self.x_axis)
            .field("cells", &self.cells)
            .finish_non_exhaustive()
    }
}

impl<X: Scalar, C: Scalar> PartialEq for Curve<X, C> {
    fn eq(&self, other: &Self) -> bool {
        self.x_axis == other.x_axis && self.cells == other.cells
    }
}

// SAFETY: native pointers address immutable locations inside owned boxes.
// Mutation requires `&mut self`; the C core has no shared mutable state.
unsafe impl<X: Scalar, C: Scalar> Send for Curve<X, C> {}
// SAFETY: shared methods only read the retained storage and C descriptor.
unsafe impl<X: Scalar, C: Scalar> Sync for Curve<X, C> {}

/// An owning, dynamically sized two-axis row-major lookup map.
pub struct Map<X: Scalar, Y: Scalar, C: Scalar> {
    x_axis: Box<[X]>,
    y_axis: Box<[Y]>,
    cells: Box<[C]>,
    native: sys::table_mutable_t,
}

impl<X: Scalar, Y: Scalar, C: Scalar> Map<X, Y, C> {
    /// Creates and validates a row-major map.
    pub fn new(x_axis: Vec<X>, y_axis: Vec<Y>, cells: Vec<C>) -> Result<Self, Error> {
        let cell_count = x_axis
            .len()
            .checked_mul(y_axis.len())
            .ok_or(Error::SizeOverflow)?;
        if cells.len() != cell_count {
            return Err(Error::DimensionMismatch);
        }
        let mut x_axis = x_axis.into_boxed_slice();
        let mut y_axis = y_axis.into_boxed_slice();
        let mut cells = cells.into_boxed_slice();
        let mut native = MaybeUninit::<sys::table_mutable_t>::uninit();
        // SAFETY: all slices are contiguous, correctly typed, and retained by
        // the returned owner. Their dimensions were checked above.
        let status = unsafe {
            sys::table_map_init(
                native.as_mut_ptr(),
                x_axis.as_mut_ptr().cast(),
                x_axis.len(),
                X::NATIVE_TYPE,
                y_axis.as_mut_ptr().cast(),
                y_axis.len(),
                Y::NATIVE_TYPE,
                cells.as_mut_ptr().cast(),
                C::NATIVE_TYPE,
            )
        };
        result(status)?;
        // SAFETY: a successful initializer wrote the complete descriptor.
        let native = unsafe { native.assume_init() };
        Ok(Self {
            x_axis,
            y_axis,
            cells,
            native,
        })
    }

    /// Returns `(x_count, y_count)`.
    pub fn dimensions(&self) -> (usize, usize) {
        (self.x_axis.len(), self.y_axis.len())
    }

    /// Returns the typed X-axis storage.
    pub fn x_axis(&self) -> &[X] {
        &self.x_axis
    }

    /// Returns the typed Y-axis storage.
    pub fn y_axis(&self) -> &[Y] {
        &self.y_axis
    }

    /// Returns flat row-major cell storage indexed as `[y][x]`.
    pub fn cells(&self) -> &[C] {
        &self.cells
    }

    /// Revalidates all native table invariants.
    pub fn validate(&self) -> Result<(), Error> {
        // SAFETY: the descriptor points to the retained boxed slices.
        result(unsafe { sys::table_validate(&raw const self.native.view) })
    }

    /// Performs a clamped bilinear lookup at `(x, y)`.
    pub fn lookup(&self, x: f32, y: f32) -> Result<f32, Error> {
        let mut output = 0.0;
        // SAFETY: the descriptor and output pointer are valid for the call.
        result(unsafe {
            sys::table_map_lookup(&raw const self.native.view, x, y, &raw mut output)
        })?;
        Ok(output)
    }

    /// Reads one X-axis point through the native API.
    pub fn x(&self, index: usize) -> Result<X, Error> {
        let mut value = MaybeUninit::<sys::table_scalar_t>::uninit();
        // SAFETY: C writes a scalar on success and the descriptor is valid.
        result(unsafe {
            sys::table_get_x(&raw const self.native.view, index, value.as_mut_ptr())
        })?;
        // SAFETY: success initialized the scalar with X's native type.
        Ok(unsafe { X::from_native(value.assume_init()) })
    }

    /// Reads one Y-axis point through the native API.
    pub fn y(&self, index: usize) -> Result<Y, Error> {
        let mut value = MaybeUninit::<sys::table_scalar_t>::uninit();
        // SAFETY: C writes a scalar on success and the descriptor is valid.
        result(unsafe {
            sys::table_get_y(&raw const self.native.view, index, value.as_mut_ptr())
        })?;
        // SAFETY: success initialized the scalar with Y's native type.
        Ok(unsafe { Y::from_native(value.assume_init()) })
    }

    /// Reads one row-major map cell through the native API.
    pub fn cell(&self, x_index: usize, y_index: usize) -> Result<C, Error> {
        let mut value = MaybeUninit::<sys::table_scalar_t>::uninit();
        // SAFETY: C writes a scalar on success and the descriptor is valid.
        result(unsafe {
            sys::table_map_get_cell(
                &raw const self.native.view,
                x_index,
                y_index,
                value.as_mut_ptr(),
            )
        })?;
        // SAFETY: success initialized the scalar with C's native type.
        Ok(unsafe { C::from_native(value.assume_init()) })
    }

    /// Replaces one X-axis point while preserving strict ordering.
    pub fn set_x(&mut self, index: usize, value: X) -> Result<(), Error> {
        // SAFETY: exclusive Rust access prevents lookup or another mutation.
        result(unsafe { sys::table_set_x(&raw mut self.native, index, value.into_native()) })
    }

    /// Replaces one Y-axis point while preserving strict ordering.
    pub fn set_y(&mut self, index: usize, value: Y) -> Result<(), Error> {
        // SAFETY: exclusive Rust access prevents lookup or another mutation.
        result(unsafe { sys::table_set_y(&raw mut self.native, index, value.into_native()) })
    }

    /// Replaces one row-major map cell.
    pub fn set_cell(&mut self, x_index: usize, y_index: usize, value: C) -> Result<(), Error> {
        // SAFETY: exclusive Rust access prevents lookup or another mutation.
        result(unsafe {
            sys::table_map_set_cell(&raw mut self.native, x_index, y_index, value.into_native())
        })
    }

    /// Atomically validates and copies a complete same-sized map snapshot.
    pub fn replace(&mut self, x_axis: &[X], y_axis: &[Y], cells: &[C]) -> Result<(), Error> {
        if x_axis.len() != self.x_axis.len()
            || y_axis.len() != self.y_axis.len()
            || cells.len() != self.cells.len()
        {
            return Err(Error::DimensionMismatch);
        }
        let mut candidate = MaybeUninit::<sys::table_view_t>::uninit();
        // SAFETY: the borrowed slices remain alive for initialization and the
        // immediately following copy into the owner's stable storage.
        result(unsafe {
            sys::table_map_view_init(
                candidate.as_mut_ptr(),
                x_axis.as_ptr().cast(),
                x_axis.len(),
                X::NATIVE_TYPE,
                y_axis.as_ptr().cast(),
                y_axis.len(),
                Y::NATIVE_TYPE,
                cells.as_ptr().cast(),
                C::NATIVE_TYPE,
            )
        })?;
        // SAFETY: initialization succeeded and exclusive access protects the
        // active table while C validates and copies the complete candidate.
        result(unsafe { sys::table_replace(&raw mut self.native, candidate.as_ptr()) })
    }
}

impl<X: Scalar, Y: Scalar, C: Scalar> Clone for Map<X, Y, C> {
    fn clone(&self) -> Self {
        Self::new(
            self.x_axis.to_vec(),
            self.y_axis.to_vec(),
            self.cells.to_vec(),
        )
        .expect("a clone of a valid map remains valid")
    }
}

impl<X: Scalar, Y: Scalar, C: Scalar> fmt::Debug for Map<X, Y, C> {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        formatter
            .debug_struct("Map")
            .field("x_axis", &self.x_axis)
            .field("y_axis", &self.y_axis)
            .field("cells", &self.cells)
            .finish_non_exhaustive()
    }
}

impl<X: Scalar, Y: Scalar, C: Scalar> PartialEq for Map<X, Y, C> {
    fn eq(&self, other: &Self) -> bool {
        self.x_axis == other.x_axis && self.y_axis == other.y_axis && self.cells == other.cells
    }
}

// SAFETY: native pointers address immutable locations inside owned boxes.
// Mutation requires `&mut self`; the C core has no shared mutable state.
unsafe impl<X: Scalar, Y: Scalar, C: Scalar> Send for Map<X, Y, C> {}
// SAFETY: shared methods only read the retained storage and C descriptor.
unsafe impl<X: Scalar, Y: Scalar, C: Scalar> Sync for Map<X, Y, C> {}
