---
applyTo: "**/*.{h,hpp,c,cc,cpp,cxx}"
description: "C++ coding conventions adapted from awesome-copilot-instructions/cpp-programming-guidelines."
---

# C++ File Instructions

## Core Practices
- Use English for code and documentation.
- Prefer explicit, descriptive names and types.
- Keep functions focused and short.
- Prefer early returns to reduce nesting.
- Avoid magic numbers; use named constants.
- Prefer C++ standard library facilities before custom utilities.

## Naming
- `PascalCase` for classes/structs.
- `camelCase` for functions, methods, and variables.
- `ALL_CAPS` for macros and constants.
- `snake_case` for file and directory names.
- Use boolean names that read as predicates (`isReady`, `hasLoop`, `canSeek`).

## Data And Safety
- Prefer composite/domain types over loosely related primitives.
- Use `const`, `constexpr`, and immutability where practical.
- Use `std::optional` or `std::expected` for expected failures.
- Use smart pointers and RAII for ownership/lifetime.
- Prefer standard containers over raw arrays.

## Class Design
- Prefer composition over inheritance.
- Keep classes focused and cohesive.
- Keep member data private and preserve invariants.
- Follow Rule of Zero/Rule of Five for resource-managing types.
- Keep method signatures const-correct.

## Error Handling
- Use exceptions for unexpected failures.
- Use `std::optional`, `std::expected`, or explicit error returns for expected failures.
- Add context at catch boundaries when rethrowing or translating errors.

## Project Structure
- Keep interface and implementation separate (`.h` / `.cpp`).
- Keep module boundaries clear (`audio`, `crowdAI`, `gameplay`, `input`, `visuals`).
- Keep build compatibility with CMake-based workflows.

## Standard Library And Concurrency
- Prefer `std::string`, `std::vector`, `std::array`, `std::unordered_map`, `std::filesystem`, and `std::chrono` where appropriate.
- Use synchronization primitives (`std::mutex`, `std::lock_guard`, `std::atomic`) when shared mutable state is unavoidable.

## Testing Guidance
- Use Arrange-Act-Assert structure.
- Add unit tests for public behavior where test scaffolding exists.
- Add integration tests for behavior spanning multiple modules.
