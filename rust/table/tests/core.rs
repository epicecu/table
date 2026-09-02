// SPDX-License-Identifier: MIT

#![allow(clippy::float_cmp)]

use table::{Curve, Error, Map, Scalar};

fn assert_send_sync<T: Send + Sync>() {}

fn moved_curve() -> Curve<i16, u8> {
    Curve::new(vec![0, 10, 20], vec![0, 20, 40]).unwrap()
}

fn exercise_scalar<T>(first: T, second: T)
where
    T: Scalar,
{
    let mut curve = Curve::new(vec![first], vec![first]).unwrap();
    curve.set_x(0, second).unwrap();
    curve.set_cell(0, second).unwrap();
    assert_eq!(curve.x(0).unwrap(), second);
    assert_eq!(curve.cell(0).unwrap(), second);
}

#[test]
fn curve_delegates_lookup_clamping_access_and_mutation_to_c() {
    let mut curve = moved_curve();
    assert_eq!(curve.lookup(15.0).unwrap(), 30.0);
    assert_eq!(curve.lookup(-10.0).unwrap(), 0.0);
    assert_eq!(curve.lookup(100.0).unwrap(), 40.0);
    assert_eq!(curve.x_axis(), &[0, 10, 20]);
    assert_eq!(curve.cells(), &[0, 20, 40]);
    assert_eq!(curve.x(1).unwrap(), 10);
    assert_eq!(curve.cell(2).unwrap(), 40);

    curve.set_x(1, 12).unwrap();
    curve.set_cell(1, 30).unwrap();
    assert_eq!(curve.lookup(12.0).unwrap(), 30.0);
    assert_eq!(curve.set_x(1, 20), Err(Error::InvalidAxis));
    assert_eq!(curve.x(1).unwrap(), 12);
}

#[test]
fn map_uses_row_major_bilinear_lookup_and_checked_mutation() {
    let mut map = Map::new(
        vec![10_i16, 20, 30, 40],
        vec![10_u16, 20, 30, 40],
        vec![
            5_u8, 40, 45, 80, 10, 35, 50, 75, 15, 30, 55, 70, 20, 25, 60, 65,
        ],
    )
    .unwrap();
    assert_eq!(map.dimensions(), (4, 4));
    assert_eq!(map.lookup(15.0, 15.0).unwrap(), 22.5);
    assert_eq!(map.lookup(10.0, 15.0).unwrap(), 7.5);
    assert_eq!(map.lookup(35.0, 30.0).unwrap(), 62.5);
    assert_eq!(map.x(2).unwrap(), 30);
    assert_eq!(map.y(3).unwrap(), 40);
    assert_eq!(map.cell(1, 2).unwrap(), 30);

    map.set_x(2, 32).unwrap();
    map.set_y(2, 32).unwrap();
    map.set_cell(1, 2, 31).unwrap();
    assert_eq!(map.x_axis()[2], 32);
    assert_eq!(map.y_axis()[2], 32);
    assert_eq!(map.cell(1, 2).unwrap(), 31);
}

#[test]
fn constructors_reject_malformed_shapes_axes_and_values() {
    assert_eq!(
        Curve::<i16, u8>::new(vec![0, 10], vec![1]),
        Err(Error::DimensionMismatch)
    );
    assert_eq!(
        Curve::<i16, u8>::new(vec![10, 0], vec![1, 2]),
        Err(Error::InvalidAxis)
    );
    assert_eq!(
        Curve::<i32, u8>::new(vec![16_777_216, 16_777_217], vec![1, 2]),
        Err(Error::InvalidAxis)
    );
    assert_eq!(
        Curve::<f32, f32>::new(vec![0.0, f32::INFINITY], vec![1.0, 2.0]),
        Err(Error::InvalidAxis)
    );
    assert_eq!(
        Curve::<f32, f32>::new(vec![0.0, 1.0], vec![1.0, f32::NAN]),
        Err(Error::InvalidArgument)
    );
    assert_eq!(
        Map::<i16, i16, u8>::new(vec![0, 1], vec![0, 1], vec![1, 2, 3]),
        Err(Error::DimensionMismatch)
    );
}

#[test]
fn lookup_and_access_report_invalid_inputs_without_exposing_output_storage() {
    let curve = moved_curve();
    assert_eq!(curve.lookup(f32::NAN), Err(Error::InvalidArgument));
    assert_eq!(curve.x(3), Err(Error::IndexOutOfRange));
    assert_eq!(curve.cell(3), Err(Error::IndexOutOfRange));

    let map = Map::new(vec![0_i8], vec![0_u8], vec![2.5_f32]).unwrap();
    assert_eq!(map.lookup(500.0, -500.0).unwrap(), 2.5);
    assert_eq!(map.y(1), Err(Error::IndexOutOfRange));
    assert_eq!(map.cell(1, 0), Err(Error::IndexOutOfRange));
}

#[test]
fn replacement_is_atomic_and_requires_the_original_shape() {
    let mut curve = moved_curve();
    curve.replace(&[-10, 10, 30], &[1, 2, 3]).unwrap();
    assert_eq!(curve.x_axis(), &[-10, 10, 30]);
    assert_eq!(curve.cells(), &[1, 2, 3]);

    assert_eq!(
        curve.replace(&[10, 0, 30], &[4, 5, 6]),
        Err(Error::InvalidAxis)
    );
    assert_eq!(curve.x_axis(), &[-10, 10, 30]);
    assert_eq!(curve.cells(), &[1, 2, 3]);
    assert_eq!(
        curve.replace(&[0, 1], &[1, 2]),
        Err(Error::DimensionMismatch)
    );

    let mut map = Map::new(vec![0_i16, 10], vec![0_u16, 20], vec![1_f32, 2.0, 3.0, 4.0]).unwrap();
    map.replace(&[-10, 10], &[5, 30], &[6.0, 7.0, 8.0, 9.0])
        .unwrap();
    assert_eq!(map.lookup(-10.0, 30.0).unwrap(), 8.0);
    assert_eq!(
        map.replace(&[-10, 10], &[5], &[1.0, 2.0]),
        Err(Error::DimensionMismatch)
    );
}

#[test]
fn moves_and_custom_clones_preserve_native_pointer_validity() {
    let curve = moved_curve();
    assert_eq!(curve.lookup(15.0).unwrap(), 30.0);
    let cloned = curve.clone();
    drop(curve);
    assert_eq!(cloned.lookup(15.0).unwrap(), 30.0);
    assert_eq!(cloned, moved_curve());

    let map = Map::new(vec![0_i16, 10], vec![0_u16, 10], vec![0_u8, 10, 20, 30]).unwrap();
    let cloned_map = map.clone();
    drop(map);
    assert_eq!(cloned_map.lookup(5.0, 5.0).unwrap(), 15.0);
}

#[test]
fn every_supported_scalar_round_trips_through_native_accessors() {
    exercise_scalar::<i8>(-2, 3);
    exercise_scalar::<u8>(2, 3);
    exercise_scalar::<i16>(-20, 30);
    exercise_scalar::<u16>(20, 30);
    exercise_scalar::<i32>(-200, 300);
    exercise_scalar::<u32>(200, 300);
    exercise_scalar::<f32>(-2.5, 3.5);
}

#[test]
fn owners_are_send_and_sync() {
    assert_send_sync::<Curve<i16, u8>>();
    assert_send_sync::<Map<i16, u16, f32>>();
}

#[test]
fn errors_validation_and_debug_output_are_useful() {
    let errors = [
        (Error::InvalidArgument, "invalid Table argument"),
        (Error::InvalidState, "Table is not in a valid state"),
        (Error::InvalidAxis, "Table axis is invalid"),
        (Error::IndexOutOfRange, "Table index is out of range"),
        (Error::TypeMismatch, "Table scalar type does not match"),
        (Error::DimensionMismatch, "Table dimensions do not match"),
        (
            Error::SizeOverflow,
            "Table dimensions overflow the platform size",
        ),
    ];
    for (error, expected) in errors {
        assert_eq!(error.to_string(), expected);
    }

    let curve = moved_curve();
    assert!(!curve.is_empty());
    curve.validate().unwrap();
    assert_eq!(
        format!("{curve:?}"),
        "Curve { x_axis: [0, 10, 20], cells: [0, 20, 40], .. }"
    );

    let map = Map::new(vec![0_i16], vec![0_u16], vec![1_u8]).unwrap();
    map.validate().unwrap();
    assert_eq!(
        format!("{map:?}"),
        "Map { x_axis: [0], y_axis: [0], cells: [1], .. }"
    );
    assert_eq!(map.lookup(f32::NAN, 0.0), Err(Error::InvalidArgument));
}
