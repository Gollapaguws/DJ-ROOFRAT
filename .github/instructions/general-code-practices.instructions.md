---
applyTo: "**/*.{h,hpp,c,cc,cpp,cxx,cmake}"
description: "General coding practices adapted from awesome-copilot-instructions/github."
---

# General Code Practices

- Follow existing project naming, formatting, and architectural patterns.
- Keep functions short, focused, and single-purpose.
- Prefer meaningful names for classes, functions, and variables.
- Avoid duplicated logic by extracting reusable helpers.
- Use comments sparingly and only to explain intent or non-obvious behavior.
- Refactor deeply nested conditionals into clearer helper functions when readability improves.
- Preserve existing behavior unless a requested change explicitly requires behavior updates.
- Keep edits scoped to the user request and avoid unrelated rewrites.