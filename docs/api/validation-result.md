# OpenAPI validation report

- Date: 2026-10-18
- File: `openapi.yaml` (v0.9.0-review)
- Check: parse YAML; verify local `$ref` resolvable; unique `operationId`; all operations have tags, responses, security; path parameters declared; build-priority metadata.
- Full official OpenAPI meta-schema validation: **NOT RUN** (validator not installed).
- Live HTTP endpoint tests / Spring Boot runtime: **NOT RUN**.

Execute `python validate_openapi.py` for structural checks. This is a draft contract pending cross-team review, not evidence that the application implements these endpoints.
