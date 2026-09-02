// SPDX-License-Identifier: MIT

use std::fmt::Debug;

use table_sys as sys;

mod private {
    pub trait Sealed {}
}

/// A scalar representation supported by the native Table core.
pub trait Scalar: private::Sealed + Copy + Debug + PartialEq + Send + Sync + 'static {
    /// The portable scalar representation.
    const TYPE: ScalarType;
    #[doc(hidden)]
    const NATIVE_TYPE: sys::table_scalar_type_t;

    #[doc(hidden)]
    fn into_native(self) -> sys::table_scalar_t;

    /// Reads the active union member selected by this implementation.
    ///
    /// # Safety
    ///
    /// `value` must contain the native scalar type represented by `Self`.
    #[doc(hidden)]
    unsafe fn from_native(value: sys::table_scalar_t) -> Self;
}

/// A supported scalar storage representation.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[non_exhaustive]
pub enum ScalarType {
    /// Signed 8-bit integer.
    I8,
    /// Unsigned 8-bit integer.
    U8,
    /// Signed 16-bit integer.
    I16,
    /// Unsigned 16-bit integer.
    U16,
    /// Signed 32-bit integer.
    I32,
    /// Unsigned 32-bit integer.
    U32,
    /// IEEE-754 single-precision floating point.
    F32,
}

macro_rules! scalar {
    ($rust:ty, $kind:ident, $native:ident, $member:ident) => {
        impl private::Sealed for $rust {}

        impl Scalar for $rust {
            const TYPE: ScalarType = ScalarType::$kind;
            const NATIVE_TYPE: sys::table_scalar_type_t = sys::table_scalar_type_t::$native;

            fn into_native(self) -> sys::table_scalar_t {
                sys::table_scalar_t {
                    r#type: Self::NATIVE_TYPE,
                    value: sys::table_scalar_value_t { $member: self },
                }
            }

            unsafe fn from_native(value: sys::table_scalar_t) -> Self {
                // SAFETY: guaranteed by the caller and this implementation's
                // fixed correspondence between type tag and union member.
                unsafe { value.value.$member }
            }
        }
    };
}

scalar!(i8, I8, TABLE_SCALAR_I8, i8);
scalar!(u8, U8, TABLE_SCALAR_U8, u8);
scalar!(i16, I16, TABLE_SCALAR_I16, i16);
scalar!(u16, U16, TABLE_SCALAR_U16, u16);
scalar!(i32, I32, TABLE_SCALAR_I32, i32);
scalar!(u32, U32, TABLE_SCALAR_U32, u32);
scalar!(f32, F32, TABLE_SCALAR_F32, f32);
