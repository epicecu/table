// SPDX-License-Identifier: MIT

#![cfg(feature = "prost")]
#![allow(clippy::float_cmp)]

use prost::Message as _;
use std::error::Error as _;
use table::{
    Curve, Error, Map,
    proto::{SnapshotError, v1},
};

#[test]
fn mixed_type_map_round_trips_and_matches_the_nanopb_wire_contract() {
    let map = Map::new(vec![-10_i16, 10], vec![0_u16, 100], vec![10_u8, 20, 30, 40]).unwrap();
    let message = v1::Map::from(&map);
    assert_eq!(message.x_axis.as_ref().unwrap().r#type, 3);
    assert_eq!(message.y_axis.as_ref().unwrap().r#type, 4);
    assert_eq!(message.cells.as_ref().unwrap().r#type, 2);

    let bytes = message.encode_to_vec();
    assert_eq!(
        bytes,
        [
            0x0a, 0x06, 0x08, 0x03, 0x12, 0x02, 0x13, 0x14, 0x12, 0x06, 0x08, 0x04, 0x1a, 0x02,
            0x00, 0x64, 0x1a, 0x08, 0x08, 0x02, 0x1a, 0x04, 0x0a, 0x14, 0x1e, 0x28,
        ]
    );
    let decoded = v1::Map::decode(bytes.as_slice()).unwrap();
    let restored = Map::<i16, u16, u8>::try_from(decoded).unwrap();
    assert_eq!(restored, map);
    assert_eq!(restored.lookup(0.0, 50.0).unwrap(), 25.0);
}

#[test]
fn curves_round_trip_every_scalar_family() {
    macro_rules! round_trip {
        ($axis:expr, $cells:expr, $axis_type:ty, $cell_type:ty) => {{
            let curve = Curve::<$axis_type, $cell_type>::new($axis, $cells).unwrap();
            let message = v1::Curve::from(&curve);
            let restored = Curve::<$axis_type, $cell_type>::try_from(message).unwrap();
            assert_eq!(restored, curve);
        }};
    }

    round_trip!(vec![-2_i8, 3], vec![-20_i32, 30], i8, i32);
    round_trip!(vec![2_u8, 3], vec![20_u32, 30], u8, u32);
    round_trip!(vec![-20_i16, 30], vec![20_u16, 30], i16, u16);
    round_trip!(vec![-2.5_f32, 3.5], vec![1.0_f32, 2.0], f32, f32);
}

#[test]
fn snapshots_reject_missing_fields_types_families_ranges_and_nonfinite_values() {
    assert_eq!(
        Curve::<i16, u8>::try_from(v1::Curve {
            x_axis: None,
            cells: None,
        }),
        Err(SnapshotError::MissingField("x_axis"))
    );

    let mut message = v1::Curve::from(&Curve::new(vec![0_i16, 10], vec![1_u8, 2]).unwrap());
    message.x_axis.as_mut().unwrap().r#type = v1::ScalarType::U16 as i32;
    assert!(matches!(
        Curve::<i16, u8>::try_from(message),
        Err(SnapshotError::ScalarTypeMismatch {
            field: "x_axis",
            ..
        })
    ));

    let mut message = v1::Curve::from(&Curve::new(vec![0_i16, 10], vec![1_u8, 2]).unwrap());
    message.x_axis.as_mut().unwrap().unsigned_values.push(10);
    assert_eq!(
        Curve::<i16, u8>::try_from(message),
        Err(SnapshotError::IncorrectValueFamily("x_axis"))
    );

    let mut message = v1::Curve::from(&Curve::new(vec![0_u16, 10], vec![1_u8, 2]).unwrap());
    message.cells.as_mut().unwrap().unsigned_values[1] = 300;
    assert_eq!(
        Curve::<u16, u8>::try_from(message),
        Err(SnapshotError::ValueOutOfRange {
            field: "cells",
            index: 1,
        })
    );

    let mut message =
        v1::Curve::from(&Curve::new(vec![0.0_f32, 10.0], vec![1.0_f32, 2.0]).unwrap());
    message.cells.as_mut().unwrap().float_values[1] = f32::NAN;
    assert_eq!(
        Curve::<f32, f32>::try_from(message),
        Err(SnapshotError::NonFiniteValue {
            field: "cells",
            index: 1,
        })
    );
}

#[test]
fn snapshots_validate_shape_and_axis_before_constructing_an_owner() {
    let mut message = v1::Curve::from(&Curve::new(vec![0_i16, 10], vec![1_u8, 2]).unwrap());
    message.cells.as_mut().unwrap().unsigned_values.pop();
    assert_eq!(
        Curve::<i16, u8>::try_from(message),
        Err(SnapshotError::Table(Error::DimensionMismatch))
    );

    let mut message =
        v1::Map::from(&Map::new(vec![0_i16, 10], vec![0_u16, 10], vec![1_u8, 2, 3, 4]).unwrap());
    message.x_axis.as_mut().unwrap().signed_values = vec![10, 0];
    assert_eq!(
        Map::<i16, u16, u8>::try_from(message),
        Err(SnapshotError::Table(Error::InvalidAxis))
    );
}

#[test]
fn snapshot_errors_are_descriptive_and_preserve_native_sources() {
    let errors = [
        SnapshotError::MissingField("x_axis"),
        SnapshotError::MissingValues("cells"),
        SnapshotError::ScalarTypeMismatch {
            field: "x_axis",
            expected: table::ScalarType::I16,
            actual: 4,
        },
        SnapshotError::IncorrectValueFamily("cells"),
        SnapshotError::ValueOutOfRange {
            field: "cells",
            index: 2,
        },
        SnapshotError::NonFiniteValue {
            field: "cells",
            index: 1,
        },
        SnapshotError::Table(Error::InvalidAxis),
    ];

    for error in &errors {
        assert!(!error.to_string().is_empty());
    }
    assert!(errors.last().unwrap().source().is_some());
    assert!(errors.first().unwrap().source().is_none());
}
