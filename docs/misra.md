# MISRA C analysis

The `quality:misra` task applies the open-source Cppcheck MISRA C:2012 addon,
including its Amendment 1 and Amendment 2 checks, to the core C implementation.
It is a regression gate and does not constitute a formal MISRA compliance
claim. Cppcheck provides partial automated coverage; project-level compliance
also requires the licensed guidelines, manual review, tool justification, and
a compliance process.

## Scope

The gate analyses `src/table/table.c` and the core headers it includes under
Cppcheck's 32-bit and 64-bit Unix data models. The C++ and Rust facades,
optional Nanopb adapter, generated sources, examples, tests, and dependencies
are outside this analysis scope.

The repository does not distribute proprietary MISRA rule text. Diagnostics
therefore use stable rule identifiers. A developer with authorised rule text
may use it locally with Cppcheck without committing it to this repository.

## Accepted findings

The suppressions in `config/cppcheck/misra-suppressions.txt` are limited to the
following reviewed cases:

- `8.7`: Public API functions require external linkage. A standalone library
  scan cannot observe downstream translation units that call them.
- `11.5`: The generic C storage contract accepts `void` pointers, validates the
  scalar tag, and then accesses the corresponding fixed-width array type.
- `15.5`: This advisory guideline is disapplied. Guard clauses keep invalid
  arguments and error propagation adjacent to their checks and reduce nesting.
- `19.2`: `table_scalar_t` is an intentional tagged union. Its adjacent type
  field selects the active member, and library access is centralised in typed
  scalar helpers.

Any additional suppression requires an update to this register with a bounded
scope and rationale. Unknown or newly reported rule identifiers fail the task.
