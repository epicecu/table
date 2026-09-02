// SPDX-License-Identifier: MIT

fn main() {
    println!("cargo:rerun-if-changed=src/table/table.c");
    println!("cargo:rerun-if-changed=src/table/table.h");

    cc::Build::new()
        .file("src/table/table.c")
        .include("src")
        .std("c11")
        .warnings(true)
        .extra_warnings(true)
        .compile("table");
}
