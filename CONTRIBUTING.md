# Contributing

Contributions should improve reproducibility, annotation clarity, or scorer correctness.

- Add tests for scoring behavior and malformed inputs.
- Keep the C++ and Haskell implementations aligned; CI compares their summaries.
- Use synthetic examples unless a future corpus has completed its own review and release approval.
- Do not submit private briefs, personal data, or unlicensed content in issues, pull requests, or fixtures.
- Document metric definitions, assumptions, and undefined cases.
- Discuss substantial schema changes before implementing them.

## Local development

The C++20 scorer uses CMake and `nlohmann-json`. The reference implementation uses GHC and Cabal. CI runs builds and tests on isolated GitHub-hosted runners.
