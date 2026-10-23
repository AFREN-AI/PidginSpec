# Annotation protocol

This protocol defines fields for a future reviewed benchmark. Public fixtures remain synthetic placeholders; no real Nigerian Pidgin or code-switched corpus is included.

## Case annotation

For each brief, preserve the source text exactly as approved for use and record its language class and task type. Annotators independently identify atomic requirements supported by explicit evidence spans. Label a requirement critical only when omission would violate a safety, security, data-handling, or explicit acceptance constraint for the stated task. Use the category vocabulary in `schema/case.schema.json`.

For a system response, split extracted requirements into distinct predictions. Link a prediction to at most one gold requirement when it is semantically equivalent and supported by the brief. Mark unsupported additions as false positives and missed gold requirements as false negatives. Resolve ambiguous links through an independent adjudicator and keep disagreement notes outside public data where they could reveal private content.

## Review and reporting

Use at least two independent reviewers for any future non-synthetic release. Report reviewer language competence, instructions, agreement before adjudication, adjudication process, and missing/excluded items. Engage fluent Nigerian Pidgin and relevant code-switching reviewers; English-only review is not sufficient to validate linguistic quality. Never label text as natural speaker-authored data without fluent review.

Each score run should have a manifest following `schema/run-manifest.schema.json`, including system and dataset revisions, split, prompt revision, seed when applicable, and timestamp. Publish aggregate results with uncertainty and subgroup denominators. A benchmark score is not evidence that a system is safe or suitable for deployment.

Any authentic corpus requires separate provenance, rights, consent, privacy, community-impact, and explicit release review. The schema is not a substitute for those approvals.
