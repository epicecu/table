// SPDX-License-Identifier: MIT

use table::Curve;

fn main() -> Result<(), table::Error> {
    let throttle = Curve::new(vec![500_u16, 1_500, 2_500, 4_500], vec![0_u8, 25, 55, 100])?;

    let percent = throttle.lookup(2_000.0)?;
    println!("Throttle: {percent:.1}%");
    Ok(())
}
