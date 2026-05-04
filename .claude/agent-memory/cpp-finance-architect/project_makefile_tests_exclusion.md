---
name: Makefile must exclude tests.cpp from APP_SRC
description: tests.cpp has its own main(); the wildcard glob will silently sweep it into the app binary and fail at link time once both files have a real main.
type: project
---

The Makefile uses `APP_SRC = $(wildcard src/*.cpp)`. `src/tests.cpp` has its own `main()` for the test runner, so the glob must filter it out:

```
APP_SRC = $(filter-out src/tests.cpp,$(wildcard src/*.cpp))
```

Why: prior to Checkpoint D, `tests.cpp` was an empty stub with no `main`, so the glob worked by accident. Once `tests.cpp` was filled in, `make` failed with `duplicate symbol _main`. The `test` target lists its sources explicitly and is unaffected.

How to apply: if you ever switch the glob back to plain `wildcard`, or add another file with its own `main()` under `src/`, the link will break. Keep the `filter-out` in place. Any new test-runner-style `.cpp` that has its own `main()` must also be added to the filter.
