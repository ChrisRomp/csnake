# C++ Snake Game - AI Coding Assistant Instructions

## Project Overview
This is a single-file C++ console game demonstrating cross-platform terminal programming with ANSI escape codes, non-blocking keyboard input, and OOP design. The entire game logic lives in `snake.cpp` (~800 lines).

The git repository is at: https://github.com/ChrisRomp/csnake

## Architecture
- **Single compilation unit**: All code in `snake.cpp` - no separate headers
- **Class structure**: 
  - `SnakeGame` - main game logic, rendering, and state management (manages ownership of `KeyboardInput`)
  - `KeyboardInput` - platform-specific terminal input abstraction (RAII pattern for terminal state)
- **Key structs**: `Point` (2D coordinates with value semantics), `Direction` enum
- **Data structures**: `std::deque<Point>` for snake body (O(1) head push/tail pop, cache-friendly iteration for collision checks)
- **Ownership model**: `SnakeGame` owns `KeyboardInput` via `std::unique_ptr` (late initialization after welcome screen), value semantics elsewhere

## Platform Handling
Cross-platform support uses preprocessor directives (`#ifdef _WIN32`, `#else`):
- **Windows**: `<conio.h>`, `<windows.h>` for `_kbhit()`, `_getch()`, console API
- **Unix/macOS**: `<termios.h>`, `<fcntl.h>` for non-blocking input via terminal attributes
- **Terminal size**: `ioctl(TIOCGWINSZ)` on Unix, `GetConsoleScreenBufferInfo()` on Windows

When adding platform-specific code, always provide both Windows and Unix implementations in parallel `#ifdef` blocks.

## Rendering System
Uses `printf` exclusively for ANSI escape sequences (not `std::cout`) to prevent stream buffer mixing issues:
- **Alternate screen buffer**: `\033[?1049h` (enable) / `\033[?1049l` (disable) - prevents scrollback contamination
- **Cursor control**: `\033[H` (home), `\033[?25l` (hide), `\033[?25h` (show)
- **Colors**: ANSI codes defined as constants (`RED`, `GREEN`, `CYAN`, etc.) at file top
- **Board rendering**: Unicode box-drawing characters (`┌─┐│└┘╔═╗║╚╝`) for arena borders

Always call `fflush(stdout)` after `printf` sequences to ensure immediate rendering.

## Build System
**Two build paths** (document both when making build changes):
1. **CMake** (recommended): `CMakeLists.txt` with platform-specific compiler flags
2. **Direct compilation**: Single command - `clang++ -std=c++11 -O3 -o snake snake.cpp`

VS Code task (`cppbuild: C/C++: clang++ build active file`) builds via clang++ directly. No external dependencies beyond C++11 stdlib.

## Game Mechanics
- **Speed progression**: Starts at 150ms/frame, decreases by 5ms per food, minimum 50ms
- **Board scaling**: Adapts to terminal size (default 40×20, minimum 12×8)
- **Collision detection**: Wall check via boundary test, self-collision via deque iteration
- **Input buffering**: `nextDirection` prevents 180° turns mid-frame

## Key Workflows
**Testing changes**:
```bash
clang++ -std=c++11 -g -o snake snake.cpp && ./snake
```

**Common modifications**:
- Adjust speed: Change `INITIAL_SPEED`, `SPEED_INCREMENT`, or minimum in `moveSnake()`
- Board size: Modify `DEFAULT_WIDTH`/`DEFAULT_HEIGHT` constants
- Colors: Update ANSI code constants or rendering in `render()`

## Code Conventions
- **RAII**: `KeyboardInput` constructor sets terminal mode, destructor restores it - exception-safe cleanup even if game crashes
- **Constants**: All magic numbers defined as `const int` at file top for easy tuning
- **Error handling**: Minimal - terminal size fallback to defaults, no exception throwing (C++11 without exceptions)
- **Memory**: Stack allocation preferred; `std::unique_ptr` only for `KeyboardInput` (delayed construction)
- **Value semantics**: `Point` uses defaulted `operator==`, copy-by-value for simplicity
- **State management**: Separate `direction` (current) and `nextDirection` (queued) prevents illegal 180° turns

## Performance Considerations
- **Rendering**: `printf` + `fflush` is faster than `std::cout` for escape sequences (no stream buffer overhead)
- **Collision checks**: Linear scan of `std::deque` acceptable for small snake size (<200 segments typical)
- **Frame timing**: `std::this_thread::sleep_for` provides cross-platform frame rate limiting
- **No optimization needed**: Game loop bounded by human input speed, not CPU

## Terminal Quirks
- **Input lag**: Always flush stdin buffer before reading (see `showWelcomeScreen()`, `showGameOverScreen()`) - mixing blocking/non-blocking modes leaves garbage
- **Arrow keys**: Escape sequences differ (`\033[A` on Unix vs. special codes on Windows) - handled in `processInput()`
- **Mode transitions**: 200ms delay after switching terminal modes prevents input corruption (termios state propagation)
- **Alternate screen**: Must disable before exit or terminal stays corrupted - RAII in `run()` ensures cleanup

## Common Pitfalls
- **Don't mix `std::cout` and `printf`** for escape sequences - stream buffering causes race conditions with immediate `printf` output
- **Don't modify terminal state without restoring** - always use RAII (`KeyboardInput` destructor) or manual restore before early returns
- **Test terminal size < minimum** - game should scale gracefully, not crash (see `updateBoardDimensions()` clamping logic)
- **Windows arrow keys** are two-byte sequences starting with `-32` or `0`, not escape codes - check both in conditional
- **Avoid blocking I/O after `KeyboardInput` init** - terminal is in non-canonical mode, `std::getline` will fail
