// SPDX-License-Identifier: MIT

use table::Map;

fn main() -> Result<(), table::Error> {
    let fuel = Map::new(
        vec![1_000_u16, 2_000, 3_000],
        vec![20_u8, 100],
        vec![2.0_f32, 2.5, 3.0, 4.0, 5.0, 6.0],
    )?;

    let value = fuel.lookup(2_500.0, 60.0)?;
    println!("Fuel value: {value:.3}");
    Ok(())
}
