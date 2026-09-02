# Contributing

## Contribution model

This project uses a maintainer-curated contribution model. Issues are open for
bug reports, feature requests, integration feedback, and proposed changes.

Users may include example code, patches, or pull requests to demonstrate a fix
or improvement. Submissions are treated as proposals and may not be merged
directly. If a proposal is accepted, the maintainer may adapt or reimplement it
to preserve the library's API, portability, safety, style, and compatibility
requirements.

Opening an issue or submitting a change does not guarantee acceptance. Where
practical, accepted contributions will be credited to the original contributor.

## Development requirements

Use `task quick` while developing and run `task all` before submitting a pull
request. Changes to public behaviour require matching C and C++ tests. Changes
to the Protobuf contract require regeneration checks and compatibility tests.
Rust binding changes additionally require `task rust:all`; use
`task rust:proto` only after editing the canonical `table.v1` schema.
Core C changes additionally require `task quality:misra`; any new suppression
requires a bounded rationale in `docs/misra.md`.

Follow [the repository source style](docs/style.md). Do not manually edit or
format generated Nanopb files. Keep hardware-specific code outside the portable
core and preserve the no-heap contract.

Report security issues privately as described in [SECURITY.md](SECURITY.md).
