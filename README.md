# PidginSpec

PidginSpec is an evaluation toolkit for measuring how well a system turns Nigerian Pidgin or code-switched briefs into faithful, evidence-supported software requirements. Version 0.1 provides a C++20 scorer and an independently implemented Haskell reference scorer.

This repository evaluates requirement extraction and planning quality. It does not claim that Nigerian Pidgin lacks benchmarks: AfroBench includes Pidgin. PidginSpec focuses on software brief understanding tasks such as constraint fidelity, critical requirement recall, and unsupported additions. IrokoBench does not currently cover Nigerian Pidgin. See [DATA.md](DATA.md) for scope and corpus policy.

## What it scores

For each adjudicated case, annotators link a prediction to a gold requirement only when its meaning is correct and supported by the source brief. An unsupported or unmatched prediction counts as a false positive. An unmatched gold requirement counts as a false negative. The scorer reports counts, precision, recall, F1, and critical requirement recall. It does not judge semantic equivalence itself; human or separately validated adjudication must supply the links.

The scorer input remains backward compatible with the minimal JSON Lines format below. Rich reviewed cases can additionally use language, task type, source text, requirement categories, evidence spans, and provenance from `schema/case.schema.json`. Score-run provenance is recorded separately using `schema/run-manifest.schema.json`.

Input is JSON Lines:

```json
{"case_id":"synthetic-1","gold_requirements":[{"id":"r1","critical":true}],"judgments":[{"prediction_id":"p1","gold_id":"r1","evidence_supported":true}]}
```

Build with CMake and a system-provided `nlohmann-json` package, then run:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/pidginspec score --input tests/fixtures/synthetic.jsonl --output summary.tsv
```

The Haskell reference implementation lives in `reference/`. Both implementations produce the same deterministic TSV summary; CI compares them. Use Cabal to run its tests and scorer.

## Data and ethics

The initial release contains synthetic fixtures only. It contains no collected briefs, personal data, or consented corpus. Any future corpus requires documented provenance, item-level consent and licensing review, community review, privacy review, and a separate approval before release. See [DATA.md](DATA.md) and [ANNOTATION.md](ANNOTATION.md).

## Limitations

The metric depends on gold annotations and adjudication quality. A high score does not establish that a system is safe, fair, useful, or suitable for deployment. This early toolkit is not a complete benchmark suite and has not been validated on a representative corpus.

## Development

CI builds C++20 and Haskell on isolated GitHub-hosted runners. No compiler or SDK is needed on a contributor's laptop to use the published workflow.

## Contributors

See [CONTRIBUTORS.md](CONTRIBUTORS.md).
