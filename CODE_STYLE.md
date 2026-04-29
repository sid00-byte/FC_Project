# FRE6883 Team Project Code Style

> Reference: Google C++ Style Guide

---
This document defines the baseline coding conventions for the team. New files should follow these rules from the start, and existing files should be cleaned up gradually when they are touched.

## 1. Naming Conventions

- Use PascalCase for classes, structs, and type names: `Stock`, `MarketManager`
- Use snake_case for functions, variables, parameters, and data members: `group_sector()`, `stock_map`
- Use `kCamelCase` for constants: `kDefaultSampleSize`

## 2. Braces and Indentation

- Use Allman style braces
- Use 4 spaces for indentation (x discard)
- Do not use tabs (x discard)
- Use Tabs (4 sapces) for indentation
- Write only one statement per line

```cpp
if (condition)
{
    do_work();
}
```

## 3. Self-Contained Headers

- Every header must compile on its own
- Include everything you use
- Do not rely on another file including a dependency first

## 4. Include Order

In `.cpp` files, use the following include order:

1. The matching header for the current source file
2. C system headers
3. C++ standard library headers
4. Third-party library headers
5. Other project headers

## 5. Keep Headers Lightweight

- Only place very short inline code in header files
- Simple getters may stay in `.h` files
- Complex logic, loops, and exception handling belong in `.cpp` files

## 6. Use `const` Aggressively

- Mark member functions as `const` when they do not modify object state
- Pass large objects as `const&` when they are read-only
- Prefer read-only interfaces when mutation is not required

## 7. Prefer References Over Pointers

- Use references when an object must exist
- Use pointers only when null is a meaningful state or optional semantics are required
- Use `nullptr`, never `NULL`

## 8. Do Not Use `using namespace std;`

- Write `std::vector`, `std::string`, and other standard library names explicitly
- This avoids collisions and is safer in a large collaborative codebase

## 9. Comment the Why, Not the Obvious

- Comments should explain business reasoning, design intent, or edge cases
- Avoid comments that simply restate the code

Preferred:

```cpp
// Reuse benchmark returns by announcement date to avoid duplicated fetches.
```

Avoid:

```cpp
// Find the benchmark return.
```

## 10. Expose Errors Early

- Fail fast on invalid arguments or inconsistent state
- Throw exceptions or report errors clearly when assumptions are violated
- Error messages should include useful context such as ticker, group, or date

## 11. Do Not Mix Formatting and Logic Changes

- Keep functional changes separate from large formatting changes
- Run broad formatting cleanups in dedicated commits
- This keeps code review focused and much easier to read

## 12. Apply the Rules Incrementally

- New files must follow this guide strictly
- Existing files should be aligned gradually when they are modified
- Do not reformat the entire project at once unless the team agrees to a dedicated cleanup pass
