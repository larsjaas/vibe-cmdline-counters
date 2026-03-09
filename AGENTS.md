# AGENTS.md

## Project Overview
This project is a small command-line utility written in C.

Goals:
- Fast startup time and low memory usage
- Simple, predictable CLI behavior
- Portable across Linux systems (including Raspberry Pi)
- No external dependencies

Target standard: **C11**

---

## Build Instructions

### Simple build (default)
gcc -std=c11 -Wall -Wextra -Werror -O2 -Iinclude -o bin/app src/*.c

### Debug build
gcc -std=c11 -Wall -Wextra -g -O0 -Iinclude -o bin/app src/*.c

### Notes
- Always ensure `bin/` exists before building
- Build must succeed with **zero warnings**

---

## Run Instructions

./bin/app [options] [arguments]

Example:
./bin/app --help

---

## Project Structure

/src        -> implementation files (.c)
/include    -> header files (.h)
/bin        -> compiled binary
/tests      -> optional test programs

Rules:
- Each `.c` file should have a corresponding `.h` file (if it exposes functions)
- Headers must be included using:
  #include "module.h"
- Do not place logic in header files

---

## CLI Design Rules

- Support `--help` and `--version`
- Use exit codes:
  - `0` = success
  - `1` = general error
  - `2` = invalid arguments

- All error messages go to `stderr`
- All normal output goes to `stdout`

Example:
fprintf(stderr, "Error: invalid input\n");

---

## Coding Guidelines

- Use **C11**
- Compile cleanly with:
  - `-Wall -Wextra -Werror`

### Naming
- Functions: `snake_case`
- Variables: `snake_case`
- Constants/macros: `UPPER_CASE`

### Style
- Keep functions under ~50 lines where possible
- Prefer early returns over deep nesting
- Avoid global variables unless necessary

---

## Argument Parsing

- Prefer simple manual parsing (`argc`, `argv`)
- Accept both:
  - short flags: `-h`
  - long flags: `--help`

- Do not use external libraries like getopt unless explicitly added later

---

## Memory Management

- Always check results of `malloc`, `calloc`, `realloc`
- Free all allocated memory
- Avoid leaks and double frees

---

## Error Handling

- Do not call `exit()` deep inside logic
- Return error codes and handle them in `main()`

---

## File Editing Rules (IMPORTANT)

When modifying code:
- Make **minimal, targeted changes**
- Do NOT rewrite entire files unless necessary
- Preserve formatting and structure
- Do not introduce unrelated refactoring

---

## Adding Features

When implementing a new feature:
1. Add function(s) in a new or existing module
2. Declare in header file if needed
3. Integrate into `main.c`
4. Ensure CLI behavior is consistent

---

## Testing

- After changes:
  - Build must succeed with zero warnings
  - Run the binary manually to verify behavior

Optional:
./bin/app --help

---

## Constraints

- No external dependencies
- No platform-specific code unless necessary
- Must compile with `gcc` on Linux
- Keep binary small and efficient

---

## Platform Notes (Raspberry Pi)

- Target: ARM Linux
- Avoid heavy allocations
- Prefer stack allocation when reasonable
- Keep CPU usage low

---

## Agent Behavior

When acting as a coding agent:

1. Understand the requested feature or bugfix
2. Identify minimal required file changes
3. Modify only relevant files
4. Ensure:
   - Code compiles cleanly
   - CLI behavior remains consistent
5. Add brief comments for non-obvious logic
6. Do NOT introduce unnecessary complexity

---

## Example Minimal Entry Point

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc > 1 && (strcmp(argv[1], "--help") == 0)) {
        printf("Usage: app [options]\n");
        return 0;
    }

    printf("Hello, world!\n");
    return 0;
}
