// SPDX-License-Identifier: MIT
//! Prost messages and conversions for the stable `table.v1` snapshots.

use std::fmt;

use crate::{Curve, Error, Map, Scalar, ScalarType};

/// Types generated from `proto/table/v1/table.proto`.
#[allow(clippy::doc_markdown, missing_docs)]
pub mod v1 {
    include!("generated/table.v1.rs");
}

/// A malformed or incompatible Protobuf snapshot.
#[derive(Debug, Clone, PartialEq, Eq)]
#[non_exhaustive]
pub enum SnapshotError {
    /// A required message-valued field was absent.
    MissingField(&'static str),
    /// The selected scalar value family contained no values.
    MissingValues(&'static str),
    /// Scalar metadata did not match the requested Rust owner type.
    ScalarTypeMismatch {
        /// Stable schema field name.
        field: &'static str,
        /// Rust owner's scalar representation.
        expected: ScalarType,
        /// Numeric Protobuf enum value found in the snapshot.
        actual: i32,
    },
    /// Values appeared in a family inconsistent with the declared scalar type.
    IncorrectValueFamily(&'static str),
    /// A 32-bit wire integer did not fit the requested narrower Rust type.
    ValueOutOfRange {
        /// Stable schema field name.
        field: &'static str,
        /// Zero-based value index.
        index: usize,
    },
    /// A float payload contained NaN or infinity.
    NonFiniteValue {
        /// Stable schema field name.
        field: &'static str,
        /// Zero-based value index.
        index: usize,
    },
    /// Native Table construction rejected the decoded snapshot.
    Table(Error),
}

impl fmt::Display for SnapshotError {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::MissingField(field) => write!(formatter, "snapshot field {field} is missing"),
            Self::MissingValues(field) => write!(formatter, "snapshot field {field} is empty"),
            Self::ScalarTypeMismatch {
                field,
                expected,
                actual,
            } => write!(
                formatter,
                "snapshot field {field} has scalar type {actual}, expected {expected:?}"
            ),
            Self::IncorrectValueFamily(field) => {
                write!(
                    formatter,
                    "snapshot field {field} uses the wrong value family"
                )
            }
            Self::ValueOutOfRange { field, index } => {
                write!(formatter, "snapshot field {field}[{index}] is out of range")
            }
            Self::NonFiniteValue { field, index } => {
                write!(formatter, "snapshot field {field}[{index}] is not finite")
            }
            Self::Table(error) => error.fmt(formatter),
        }
    }
}

impl std::error::Error for SnapshotError {
    fn source(&self) -> Option<&(dyn std::error::Error + 'static)> {
        match self {
            Self::Table(error) => Some(error),
            _ => None,
        }
    }
}

impl From<Error> for SnapshotError {
    fn from(value: Error) -> Self {
        Self::Table(value)
    }
}

/// Internal scalar-array conversion used by the generic snapshot adapters.
#[doc(hidden)]
#[allow(missing_docs)]
pub trait ProtoScalar: Scalar {
    fn encode(values: &[Self]) -> v1::ScalarArray;
    fn decode(array: v1::ScalarArray, field: &'static str) -> Result<Vec<Self>, SnapshotError>;
}

const fn proto_type(value: ScalarType) -> i32 {
    match value {
        ScalarType::I8 => v1::ScalarType::I8 as i32,
        ScalarType::U8 => v1::ScalarType::U8 as i32,
        ScalarType::I16 => v1::ScalarType::I16 as i32,
        ScalarType::U16 => v1::ScalarType::U16 as i32,
        ScalarType::I32 => v1::ScalarType::I32 as i32,
        ScalarType::U32 => v1::ScalarType::U32 as i32,
        ScalarType::F32 => v1::ScalarType::F32 as i32,
    }
}

fn check_array<T: Scalar>(
    array: &v1::ScalarArray,
    field: &'static str,
    family: ValueFamily,
) -> Result<(), SnapshotError> {
    if array.r#type != proto_type(T::TYPE) {
        return Err(SnapshotError::ScalarTypeMismatch {
            field,
            expected: T::TYPE,
            actual: array.r#type,
        });
    }
    let incorrect = match family {
        ValueFamily::Signed => !array.unsigned_values.is_empty() || !array.float_values.is_empty(),
        ValueFamily::Unsigned => !array.signed_values.is_empty() || !array.float_values.is_empty(),
        ValueFamily::Float => !array.signed_values.is_empty() || !array.unsigned_values.is_empty(),
    };
    if incorrect {
        return Err(SnapshotError::IncorrectValueFamily(field));
    }
    Ok(())
}

#[derive(Clone, Copy)]
enum ValueFamily {
    Signed,
    Unsigned,
    Float,
}

macro_rules! signed_scalar {
    ($type:ty) => {
        impl ProtoScalar for $type {
            fn encode(values: &[Self]) -> v1::ScalarArray {
                v1::ScalarArray {
                    r#type: proto_type(Self::TYPE),
                    signed_values: values.iter().copied().map(i32::from).collect(),
                    unsigned_values: Vec::new(),
                    float_values: Vec::new(),
                }
            }

            fn decode(
                array: v1::ScalarArray,
                field: &'static str,
            ) -> Result<Vec<Self>, SnapshotError> {
                check_array::<Self>(&array, field, ValueFamily::Signed)?;
                if array.signed_values.is_empty() {
                    return Err(SnapshotError::MissingValues(field));
                }
                array
                    .signed_values
                    .into_iter()
                    .enumerate()
                    .map(|(index, value)| {
                        Self::try_from(value)
                            .map_err(|_| SnapshotError::ValueOutOfRange { field, index })
                    })
                    .collect()
            }
        }
    };
}

macro_rules! unsigned_scalar {
    ($type:ty) => {
        impl ProtoScalar for $type {
            fn encode(values: &[Self]) -> v1::ScalarArray {
                v1::ScalarArray {
                    r#type: proto_type(Self::TYPE),
                    signed_values: Vec::new(),
                    unsigned_values: values.iter().copied().map(u32::from).collect(),
                    float_values: Vec::new(),
                }
            }

            fn decode(
                array: v1::ScalarArray,
                field: &'static str,
            ) -> Result<Vec<Self>, SnapshotError> {
                check_array::<Self>(&array, field, ValueFamily::Unsigned)?;
                if array.unsigned_values.is_empty() {
                    return Err(SnapshotError::MissingValues(field));
                }
                array
                    .unsigned_values
                    .into_iter()
                    .enumerate()
                    .map(|(index, value)| {
                        Self::try_from(value)
                            .map_err(|_| SnapshotError::ValueOutOfRange { field, index })
                    })
                    .collect()
            }
        }
    };
}

signed_scalar!(i8);
signed_scalar!(i16);
unsigned_scalar!(u8);
unsigned_scalar!(u16);

impl ProtoScalar for i32 {
    fn encode(values: &[Self]) -> v1::ScalarArray {
        v1::ScalarArray {
            r#type: proto_type(Self::TYPE),
            signed_values: values.to_vec(),
            unsigned_values: Vec::new(),
            float_values: Vec::new(),
        }
    }

    fn decode(array: v1::ScalarArray, field: &'static str) -> Result<Vec<Self>, SnapshotError> {
        check_array::<Self>(&array, field, ValueFamily::Signed)?;
        if array.signed_values.is_empty() {
            return Err(SnapshotError::MissingValues(field));
        }
        Ok(array.signed_values)
    }
}

impl ProtoScalar for u32 {
    fn encode(values: &[Self]) -> v1::ScalarArray {
        v1::ScalarArray {
            r#type: proto_type(Self::TYPE),
            signed_values: Vec::new(),
            unsigned_values: values.to_vec(),
            float_values: Vec::new(),
        }
    }

    fn decode(array: v1::ScalarArray, field: &'static str) -> Result<Vec<Self>, SnapshotError> {
        check_array::<Self>(&array, field, ValueFamily::Unsigned)?;
        if array.unsigned_values.is_empty() {
            return Err(SnapshotError::MissingValues(field));
        }
        Ok(array.unsigned_values)
    }
}

impl ProtoScalar for f32 {
    fn encode(values: &[Self]) -> v1::ScalarArray {
        v1::ScalarArray {
            r#type: proto_type(Self::TYPE),
            signed_values: Vec::new(),
            unsigned_values: Vec::new(),
            float_values: values.to_vec(),
        }
    }

    fn decode(array: v1::ScalarArray, field: &'static str) -> Result<Vec<Self>, SnapshotError> {
        check_array::<Self>(&array, field, ValueFamily::Float)?;
        if array.float_values.is_empty() {
            return Err(SnapshotError::MissingValues(field));
        }
        for (index, value) in array.float_values.iter().enumerate() {
            if !value.is_finite() {
                return Err(SnapshotError::NonFiniteValue { field, index });
            }
        }
        Ok(array.float_values)
    }
}

impl<X: ProtoScalar, C: ProtoScalar> From<&Curve<X, C>> for v1::Curve {
    fn from(value: &Curve<X, C>) -> Self {
        Self {
            x_axis: Some(X::encode(value.x_axis())),
            cells: Some(C::encode(value.cells())),
        }
    }
}

impl<X: ProtoScalar, C: ProtoScalar> TryFrom<v1::Curve> for Curve<X, C> {
    type Error = SnapshotError;

    fn try_from(value: v1::Curve) -> Result<Self, Self::Error> {
        let x_axis = X::decode(
            value.x_axis.ok_or(SnapshotError::MissingField("x_axis"))?,
            "x_axis",
        )?;
        let cells = C::decode(
            value.cells.ok_or(SnapshotError::MissingField("cells"))?,
            "cells",
        )?;
        Self::new(x_axis, cells).map_err(Into::into)
    }
}

impl<X: ProtoScalar, Y: ProtoScalar, C: ProtoScalar> From<&Map<X, Y, C>> for v1::Map {
    fn from(value: &Map<X, Y, C>) -> Self {
        Self {
            x_axis: Some(X::encode(value.x_axis())),
            y_axis: Some(Y::encode(value.y_axis())),
            cells: Some(C::encode(value.cells())),
        }
    }
}

impl<X: ProtoScalar, Y: ProtoScalar, C: ProtoScalar> TryFrom<v1::Map> for Map<X, Y, C> {
    type Error = SnapshotError;

    fn try_from(value: v1::Map) -> Result<Self, Self::Error> {
        let x_axis = X::decode(
            value.x_axis.ok_or(SnapshotError::MissingField("x_axis"))?,
            "x_axis",
        )?;
        let y_axis = Y::decode(
            value.y_axis.ok_or(SnapshotError::MissingField("y_axis"))?,
            "y_axis",
        )?;
        let cells = C::decode(
            value.cells.ok_or(SnapshotError::MissingField("cells"))?,
            "cells",
        )?;
        Self::new(x_axis, y_axis, cells).map_err(Into::into)
    }
}
