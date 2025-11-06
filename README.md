# C++ Snake Game

![Snake Game Icon](./images/csnake-icon-256.png)

A classic Snake game implementation in C++ for the console/terminal with smooth gameplay, colorful graphics, and cross-platform support.

## Features

- **Console Graphics**: Uses ANSI escape codes for colors and box-drawing characters
- **Adaptive Display**: Automatically detects terminal size and scales arena to fit
- **Smooth Gameplay**: Frame-rate controlled animation with increasing difficulty
- **Cross-Platform**: (Hopefully) Works on Windows, macOS, and Linux
  - I've only tested on MacOS with iterm2
- **Responsive Controls**: WASD or arrow key support with non-blocking input
- **Progressive Difficulty**: Game speeds up as you score more points
- **Score Tracking**: Keep track of your score and snake length
- **Pause Functionality**: Pause and resume gameplay anytime
- **Collision Detection**: Proper wall and self-collision detection
- **Welcome & Game Over Screens**: Polished UI with ASCII art
- **Alternate Screen Buffer**: Clean game display without scrollback contamination

## Screenshots

```
╔════════════════════════════════════════════╗
║        C++ SNAKE GAME                      ║
╚════════════════════════════════════════════╝

  Score: 5    Speed: 75

  ┌────────────────────────────────────────┐
  │                                        │
  │         ◆■■■                           │
  │                                        │
  │                    ●                   │
  │                                        │
  │                                        │
  └────────────────────────────────────────┘

  Controls: WASD or Arrow Keys | SPACE to pause | Q to quit
```

## Building and Running

### Prerequisites

- C++11 compatible compiler (g++, clang++, MSVC)
- CMake 3.10 or higher (optional, but recommended)
- Terminal with ANSI escape code support

### Option 1: Using CMake (Recommended)

```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .

# Run the game
./snake        # On macOS/Linux
snake.exe      # On Windows
```

### Option 2: Direct Compilation with clang++/g++

#### macOS:
```bash
clang++ -std=c++11 -O3 -o snake snake.cpp
./snake
```

#### Linux:
```bash
g++ -std=c++11 -O3 -o snake snake.cpp
./snake
```

#### Windows (MSVC):
```bash
cl /EHsc /O2 snake.cpp
snake.exe
```

#### Windows (MinGW):
```bash
g++ -std=c++11 -O3 -o snake.exe snake.cpp
snake.exe
```

### VS Code Users

A build task is included for clang++. Simply:
1. Open the project in VS Code
2. Press `Cmd+Shift+B` (macOS) or `Ctrl+Shift+B` (Windows/Linux)
3. Select "C/C++: clang++ build active file"
4. Run `./snake` from the terminal

## How to Play

1. **Start the Game**: Run the executable and press any key at the welcome screen
2. **Control the Snake**:
   - Use `W/↑` to move up
   - Use `S/↓` to move down
   - Use `A/←` to move left
   - Use `D/→` to move right
3. **Objective**: Eat the red food (●) to grow your snake and increase your score
4. **Avoid**:
   - Hitting the walls
   - Running into yourself
5. **Special Controls**:
   - Press `SPACE` to pause/resume
   - Press `Q` to quit
   - Press `R` to restart after game over

## Game Mechanics

- **Starting Length**: The snake starts with 3 segments
- **Growth**: Each food eaten adds one segment to the snake
- **Speed**: The game starts at a comfortable pace and speeds up with each food eaten
- **Scoring**: Each food eaten = 1 point
- **Difficulty**: Speed increases by 5ms per food, capped at maximum difficulty

## Technical Details

### Architecture

The game is built with clean object-oriented design:

- **SnakeGame Class**: Main game logic and state management
- **KeyboardInput Class**: Cross-platform keyboard input handling
- **Point Struct**: 2D coordinate representation
- **Direction Enum**: Movement direction states

### Cross-Platform Input Handling

- **Windows**: Uses `_kbhit()` and `_getch()` from `<conio.h>`
- **Unix/Linux/macOS**: Uses `termios` for non-canonical input and `fcntl` for non-blocking reads

### Terminal Size Detection

- **macOS/Linux**: Uses `ioctl(TIOCGWINSZ)` to detect terminal dimensions
- **Windows**: Uses `GetConsoleScreenBufferInfo()` for window size
- Arena automatically scales to fit available space (default 40×20, adjusts as needed)
- Displays warning when terminal is smaller than ideal size

### Rendering

- Uses alternate screen buffer (`\033[?1049h/l`) to avoid scrollback contamination
- Consistent `printf`-based rendering to prevent buffer mixing issues
- ANSI escape codes for:
  - Terminal clearing and cursor positioning
  - Text colors (red, green, yellow, cyan, etc.)
  - Bold text
  - Cursor visibility control

## Performance Notes

- The game runs at variable frame rates based on difficulty
- Initial speed: 150ms per frame
- Maximum speed: 50ms per frame (20 FPS)
- Uses `std::deque` for efficient snake body management

## Achievements

The game recognizes different skill levels:
- **Score < 15**: Good try!
- **Score 15-29**: Great job!
- **Score 30-49**: Amazing!
- **Score 50+**: Legendary Snake Master!

## Troubleshooting

### Colors not showing correctly
- Ensure your terminal supports ANSI escape codes
- On Windows, use Windows Terminal or enable ANSI support in Command Prompt
- macOS: Both iTerm2 and Terminal.app are supported

### Arena not rendering completely or scrolling
- Increase your terminal window size (game needs minimum ~40 columns × 30 rows)
- The game will auto-scale to fit, but very small terminals may have issues
- Check that alternate screen buffer is supported by your terminal

### Input lag or unresponsive controls
- Close other applications that might be using stdin
- Ensure your terminal window has focus
- On macOS, if using tmux/screen, ensure proper terminal emulation is set

### Compilation errors
- Verify you have a C++11 compatible compiler
- Check that all required headers are available on your system
- On macOS, Xcode Command Line Tools may be required: `xcode-select --install`

## Credits

Created as a demonstration of C++ console game programming with GitHub Copilot assistance. Special thanks to the AI pair programming tools that helped debug terminal rendering issues and cross-platform compatibility! 🤖🐍

## License

MIT License. This is a sample educational project. Feel free to use, modify, and distribute as needed.

## Future Enhancement Ideas

- High score persistence (save to file)
- Multiple difficulty levels
- Power-ups and special items
- Obstacles and maze levels
- Two-player mode
- Custom color themes
- Sound effects (if terminal supports beep)
- Wrap-around walls mode

Enjoy the game!
