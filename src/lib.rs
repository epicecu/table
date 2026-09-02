// SPDX-License-Identifier: MIT
//! Raw foreign-function declarations for the bundled Table C library.
//!
//! Most applications should depend on the safe `table` crate instead.

#![no_std]
#![allow(non_camel_case_types)]
#![allow(clippy::pub_underscore_fields)]

use core::ffi::c_void;

#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum table_status_t {
    TABLE_OK = 0,
    TABLE_INVALID_ARGUMENT,
    TABLE_INVALID_STATE,
    TABLE_INVALID_AXIS,
    TABLE_INDEX_OUT_OF_RANGE,
    TABLE_TYPE_MISMATCH,
}

#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum table_scalar_type_t {
    TABLE_SCALAR_UNSPECIFIED = 0,
    TABLE_SCALAR_I8,
    TABLE_SCALAR_U8,
    TABLE_SCALAR_I16,
    TABLE_SCALAR_U16,
    TABLE_SCALAR_I32,
    TABLE_SCALAR_U32,
    TABLE_SCALAR_F32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub union table_scalar_value_t {
    pub i8: i8,
    pub u8: u8,
    pub i16: i16,
    pub u16: u16,
    pub i32: i32,
    pub u32: u32,
    pub f32: f32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct table_scalar_t {
    pub r#type: table_scalar_type_t,
    pub value: table_scalar_value_t,
}

#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum table_kind_t {
    TABLE_KIND_UNSPECIFIED = 0,
    TABLE_KIND_CURVE,
    TABLE_KIND_MAP,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct table_view_t {
    pub kind: table_kind_t,
    pub x_type: table_scalar_type_t,
    pub y_type: table_scalar_type_t,
    pub cell_type: table_scalar_type_t,
    pub x_count: usize,
    pub y_count: usize,
    pub x_axis: *const c_void,
    pub y_axis: *const c_void,
    pub cells: *const c_void,
    pub _state: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct table_mutable_t {
    pub view: table_view_t,
    pub x_axis: *mut c_void,
    pub y_axis: *mut c_void,
    pub cells: *mut c_void,
}

unsafe extern "C" {
    pub fn table_curve_view_init(
        view: *mut table_view_t,
        x_axis: *const c_void,
        x_count: usize,
        x_type: table_scalar_type_t,
        cells: *const c_void,
        cell_type: table_scalar_type_t,
    ) -> table_status_t;
    pub fn table_map_view_init(
        view: *mut table_view_t,
        x_axis: *const c_void,
        x_count: usize,
        x_type: table_scalar_type_t,
        y_axis: *const c_void,
        y_count: usize,
        y_type: table_scalar_type_t,
        cells: *const c_void,
        cell_type: table_scalar_type_t,
    ) -> table_status_t;
    pub fn table_curve_init(
        table: *mut table_mutable_t,
        x_axis: *mut c_void,
        x_count: usize,
        x_type: table_scalar_type_t,
        cells: *mut c_void,
        cell_type: table_scalar_type_t,
    ) -> table_status_t;
    pub fn table_map_init(
        table: *mut table_mutable_t,
        x_axis: *mut c_void,
        x_count: usize,
        x_type: table_scalar_type_t,
        y_axis: *mut c_void,
        y_count: usize,
        y_type: table_scalar_type_t,
        cells: *mut c_void,
        cell_type: table_scalar_type_t,
    ) -> table_status_t;
    pub fn table_validate(view: *const table_view_t) -> table_status_t;
    pub fn table_curve_lookup(
        view: *const table_view_t,
        x: f32,
        output: *mut f32,
    ) -> table_status_t;
    pub fn table_map_lookup(
        view: *const table_view_t,
        x: f32,
        y: f32,
        output: *mut f32,
    ) -> table_status_t;
    pub fn table_get_x(
        view: *const table_view_t,
        index: usize,
        value: *mut table_scalar_t,
    ) -> table_status_t;
    pub fn table_get_y(
        view: *const table_view_t,
        index: usize,
        value: *mut table_scalar_t,
    ) -> table_status_t;
    pub fn table_curve_get_cell(
        view: *const table_view_t,
        x_index: usize,
        value: *mut table_scalar_t,
    ) -> table_status_t;
    pub fn table_map_get_cell(
        view: *const table_view_t,
        x_index: usize,
        y_index: usize,
        value: *mut table_scalar_t,
    ) -> table_status_t;
    pub fn table_set_x(
        table: *mut table_mutable_t,
        index: usize,
        value: table_scalar_t,
    ) -> table_status_t;
    pub fn table_set_y(
        table: *mut table_mutable_t,
        index: usize,
        value: table_scalar_t,
    ) -> table_status_t;
    pub fn table_curve_set_cell(
        table: *mut table_mutable_t,
        x_index: usize,
        value: table_scalar_t,
    ) -> table_status_t;
    pub fn table_map_set_cell(
        table: *mut table_mutable_t,
        x_index: usize,
        y_index: usize,
        value: table_scalar_t,
    ) -> table_status_t;
    pub fn table_replace(
        table: *mut table_mutable_t,
        candidate: *const table_view_t,
    ) -> table_status_t;
}
