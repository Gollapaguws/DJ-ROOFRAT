---
applyTo: "**/*.{h,hpp,c,cc,cpp,cxx,cmake}"
description: "Quality guardrails adapted from awesome-copilot-instructions/code-guidelines."
---

# Quality Guardrails

- Verify assumptions against the current repository state before coding.
- Keep changes scoped to the requested task and preserve unrelated behavior.
- Prefer explicit, descriptive identifiers for readability.
- Prioritize performance and security when introducing new logic.
- Handle edge cases explicitly and return clear errors for failure paths.
- Replace hardcoded values with named constants when practical.
- Add or update tests when behavior changes and test scaffolding exists.