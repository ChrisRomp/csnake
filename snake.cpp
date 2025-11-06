#include <iostream>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <algorithm>
#include <memory>
#include <cstdio>
#include <sstream>
#include <string>

#ifndef _WIN32
    #include <sys/ioctl.h>
#endif

// Platform-specific includes for keyboard input
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
#endif

// ANSI Color Codes
const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";
const std::string BOLD = "\033[1m";
const std::string BG_RED = "\033[41m";
const std::string BG_GREEN = "\033[42m";
const std::string BG_YELLOW = "\033[43m";

// Game Constants
const int INITIAL_SPEED = 150; // milliseconds per frame
const int SPEED_INCREMENT = 5; // speed increase per food eaten
const int DEFAULT_WIDTH = 40;
const int DEFAULT_HEIGHT = 20;
const int MIN_WIDTH = 12;
const int MIN_HEIGHT = 8;

// Point structure for coordinates
struct Point {
    int x, y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

// Direction enum
enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
};

// Cross-platform keyboard input handling
class KeyboardInput {
public:
    KeyboardInput() {
        #ifndef _WIN32
        tcflush(STDIN_FILENO, TCIFLUSH);
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
        #endif
    }

    ~KeyboardInput() {
        #ifndef _WIN32
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);
        #endif
    }

    bool kbhit() {
        #ifdef _WIN32
        return _kbhit();
        #else
        struct timeval tv = {0, 0};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
        #endif
    }

    char getch() {
        #ifdef _WIN32
        return _getch();
        #else
        char c;
        return read(STDIN_FILENO, &c, 1) > 0 ? c : 0;
        #endif
    }

private:
    #ifndef _WIN32
    struct termios oldt, newt;
    #endif
};

// Snake Game Class
class SnakeGame {
private:
    std::deque<Point> snake;
    Point food;
    Direction direction;
    Direction nextDirection;
    bool gameOver;
    bool paused;
    int score;
    int speed;
    std::unique_ptr<KeyboardInput> keyboard;
    int boardWidth;
    int boardHeight;
    int terminalWidth;
    int terminalHeight;
    bool sizeWarning;
    std::string sizeWarningMessage;

    void spawnFood() {
        bool validPosition;
        if (static_cast<int>(snake.size()) >= boardWidth * boardHeight) {
            gameOver = true;
            return;
        }
        do {
            validPosition = true;
            food.x = rand() % boardWidth;
            food.y = rand() % boardHeight;

            // Make sure food doesn't spawn on snake
            for (const auto& segment : snake) {
                if (segment == food) {
                    validPosition = false;
                    break;
                }
            }
        } while (!validPosition);
    }

    void moveSnake() {
        // Update direction (prevent 180-degree turns)
        if ((direction == UP && nextDirection != DOWN) ||
            (direction == DOWN && nextDirection != UP) ||
            (direction == LEFT && nextDirection != RIGHT) ||
            (direction == RIGHT && nextDirection != LEFT)) {
            direction = nextDirection;
        }

        // Calculate new head position
        Point newHead = snake.front();
        switch (direction) {
            case UP:    newHead.y--; break;
            case DOWN:  newHead.y++; break;
            case LEFT:  newHead.x--; break;
            case RIGHT: newHead.x++; break;
            case NONE:  return;
        }

        // Check wall collision
        if (newHead.x < 0 || newHead.x >= boardWidth ||
            newHead.y < 0 || newHead.y >= boardHeight) {
            gameOver = true;
            return;
        }

        // Check self collision
        for (const auto& segment : snake) {
            if (segment == newHead) {
                gameOver = true;
                return;
            }
        }

        // Add new head
        snake.push_front(newHead);

        // Check if food is eaten
        if (newHead == food) {
            score++;
            speed = std::max(50, INITIAL_SPEED - (score * SPEED_INCREMENT));
            spawnFood();
        } else {
            // Remove tail if no food eaten
            snake.pop_back();
        }
    }

    void clearScreen() {
        #ifdef _WIN32
        system("cls");
        #else
        printf("\033[2J\033[1;1H");  // Clear entire screen and home cursor
        fflush(stdout);
        #endif
    }

    void updateBoardDimensions() {
        #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            terminalWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            terminalHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        } else {
            terminalWidth = DEFAULT_WIDTH + 4;
            terminalHeight = DEFAULT_HEIGHT + 8;
        }
        #else
        struct winsize ws{};
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
            terminalWidth = ws.ws_col;
            terminalHeight = ws.ws_row;
        } else {
            terminalWidth = DEFAULT_WIDTH + 4;
            terminalHeight = DEFAULT_HEIGHT + 8;
        }
        #endif

        const int horizontalPadding = 4;  // Two leading spaces plus side walls
        // Title(3) + Score(1) + Warning(1) + TopBorder(1) + BottomBorder(1) + Controls(1) = 9 lines
        const int verticalPadding = 9;

        int availableWidth = (terminalWidth > 0) ? terminalWidth - horizontalPadding : DEFAULT_WIDTH;
        int availableHeight = (terminalHeight > 0) ? terminalHeight - verticalPadding : DEFAULT_HEIGHT;

        if (availableWidth <= 0) availableWidth = DEFAULT_WIDTH;
        if (availableHeight <= 0) availableHeight = DEFAULT_HEIGHT;

        int newBoardWidth = std::min(DEFAULT_WIDTH, availableWidth);
        int newBoardHeight = std::min(DEFAULT_HEIGHT, availableHeight);

        if (availableWidth < 5) {
            newBoardWidth = std::max(availableWidth, 1);
        } else if (newBoardWidth < MIN_WIDTH && availableWidth >= MIN_WIDTH) {
            newBoardWidth = MIN_WIDTH;
        }

        if (availableHeight < 5) {
            newBoardHeight = std::max(availableHeight, 1);
        } else if (newBoardHeight < MIN_HEIGHT && availableHeight >= MIN_HEIGHT) {
            newBoardHeight = MIN_HEIGHT;
        }

        newBoardWidth = std::max(newBoardWidth, 1);
        newBoardHeight = std::max(newBoardHeight, 1);

        bool widthTrimmed = newBoardWidth < DEFAULT_WIDTH;
        bool heightTrimmed = newBoardHeight < DEFAULT_HEIGHT;

        boardWidth = newBoardWidth;
        boardHeight = newBoardHeight;

        sizeWarning = widthTrimmed || heightTrimmed;
        sizeWarningMessage.clear();

        if (sizeWarning) {
            std::ostringstream warn;
            warn << "Arena scaled to " << boardWidth << "x" << boardHeight
                 << " (ideal " << DEFAULT_WIDTH << "x" << DEFAULT_HEIGHT << ")";
            sizeWarningMessage = warn.str();

            int maxHudWidth = boardWidth + 4;
            if (static_cast<int>(sizeWarningMessage.size()) > maxHudWidth) {
                std::ostringstream shortWarn;
                shortWarn << "Arena " << boardWidth << "x" << boardHeight;
                sizeWarningMessage = shortWarn.str();
            }
        }
    }

    void initTerminal() {
        // Use printf consistently for escape sequences
        printf("\033[?1049h");      // Enable alternate screen buffer
        printf("\033[2J");          // Clear screen
        printf("\033[1;1H");        // Home cursor
        printf("\033[?25l");        // Hide cursor
        fflush(stdout);
    }

    void restoreTerminal() {
        printf("\033[?25h");        // Show cursor
        printf("\033[2J");          // Clear screen
        printf("\033[1;1H");        // Home cursor
        printf("\033[?1049l");      // Disable alternate screen buffer
        fflush(stdout);
    }

    void render() {
        // Just home cursor - we're in alternate buffer so no scrolling
        printf("\033[H");
        
        // Title
        printf("\033[1m\033[36m╔");
        for (int i = 0; i < boardWidth + 2; ++i) printf("═");
        printf("╗\n");

        printf("║");
        int titlePad = (boardWidth + 2 - 16) / 2;  // 16 = length of "C++ SNAKE GAME"
        for (int i = 0; i < titlePad; ++i) printf(" ");
        printf("\033[33mC++ SNAKE GAME\033[36m");
        for (int i = 0; i < boardWidth + 2 - 16 - titlePad; ++i) printf(" ");
        printf("║\n");

        printf("╚");
        for (int i = 0; i < boardWidth + 2; ++i) printf("═");
        printf("╝\033[0m\n");

        // HUD
        int displaySpeed = std::max(0, INITIAL_SPEED - speed + 50);
        printf("  \033[32mScore: \033[1m%d\033[0m  \033[35mSpeed: \033[1m%d\033[0m\n", score, displaySpeed);

        if (sizeWarning && !sizeWarningMessage.empty()) {
            printf("  \033[33m%s\033[0m\n", sizeWarningMessage.c_str());
        }

        // Top border
        printf("  \033[36m┌");
        for (int i = 0; i < boardWidth; ++i) printf("─");
        printf("┐\033[0m\n");

        // Game board
        for (int y = 0; y < boardHeight; ++y) {
            printf("  \033[36m│\033[0m");

            for (int x = 0; x < boardWidth; ++x) {
                bool drawn = false;

                if (x == food.x && y == food.y) {
                    printf("\033[31m●\033[0m");
                    drawn = true;
                } else if (!snake.empty() && x == snake.front().x && y == snake.front().y) {
                    printf("\033[32m\033[1m◆\033[0m");
                    drawn = true;
                } else if (!snake.empty()) {
                    for (size_t i = 1; i < snake.size() && !drawn; ++i) {
                        if (x == snake[i].x && y == snake[i].y) {
                            printf("\033[32m■\033[0m");
                            drawn = true;
                        }
                    }
                }

                if (!drawn) {
                    printf(" ");
                }
            }

            printf("\033[36m│\033[0m\n");
        }

        // Bottom border
        printf("  \033[36m└");
        for (int i = 0; i < boardWidth; ++i) printf("─");
        printf("┘\033[0m\n");

        // Controls
        if (paused) {
            printf("  \033[33m\033[1m⏸  PAUSED - Press SPACE to resume\033[0m");
        } else {
            printf("  \033[37mControls: WASD or Arrow Keys | SPACE to pause | Q to quit\033[0m");
        }

        printf("\033[J");  // Clear to end of screen
        fflush(stdout);
    }

    void processInput() {
        if (!keyboard) return;
        if (keyboard->kbhit()) {
            char key = keyboard->getch();

            // Handle arrow keys (escape sequences on Unix)
            #ifndef _WIN32
            if (key == 27) { // ESC
                if (keyboard->kbhit() && keyboard->getch() == '[') {
                    if (keyboard->kbhit()) {
                        key = keyboard->getch();
                        switch (key) {
                            case 'A': key = 'w'; break; // Up arrow
                            case 'B': key = 's'; break; // Down arrow
                            case 'C': key = 'd'; break; // Right arrow
                            case 'D': key = 'a'; break; // Left arrow
                        }
                    }
                }
            }
            #else
            // Windows arrow keys
            if (key == -32 || key == 0) {
                key = keyboard->getch();
                switch (key) {
                    case 72: key = 'w'; break; // Up arrow
                    case 80: key = 's'; break; // Down arrow
                    case 77: key = 'd'; break; // Right arrow
                    case 75: key = 'a'; break; // Left arrow
                }
            }
            #endif

            // Process key
            switch (key) {
                case 'w':
                case 'W':
                    if (direction == NONE || direction != DOWN) {
                        nextDirection = UP;
                        if (direction == NONE) direction = UP;
                    }
                    break;
                case 's':
                case 'S':
                    if (direction == NONE || direction != UP) {
                        nextDirection = DOWN;
                        if (direction == NONE) direction = DOWN;
                    }
                    break;
                case 'a':
                case 'A':
                    if (direction == NONE || direction != RIGHT) {
                        nextDirection = LEFT;
                        if (direction == NONE) direction = LEFT;
                    }
                    break;
                case 'd':
                case 'D':
                    if (direction == NONE || direction != LEFT) {
                        nextDirection = RIGHT;
                        if (direction == NONE) direction = RIGHT;
                    }
                    break;
                case ' ':
                    paused = !paused;
                    break;
                case 'q':
                case 'Q':
                    gameOver = true;
                    break;
            }
        }
    }

public:
    SnakeGame()
        : direction(NONE),
          nextDirection(NONE),
          gameOver(false),
          paused(false),
          score(0),
          speed(INITIAL_SPEED),
          boardWidth(DEFAULT_WIDTH),
          boardHeight(DEFAULT_HEIGHT),
          terminalWidth(0),
          terminalHeight(0),
          sizeWarning(false) {
        srand(static_cast<unsigned>(time(nullptr)));
    }

    void reset() {
        updateBoardDimensions();

        snake.clear();
        int centerX = boardWidth / 2;
        int centerY = boardHeight / 2;

        snake.push_back({centerX, centerY});
        if (boardWidth > 1) {
            snake.push_back({std::max(0, centerX - 1), centerY});
        }
        if (boardWidth > 2) {
            snake.push_back({std::max(0, centerX - 2), centerY});
        }

        direction = NONE;
        nextDirection = NONE;
        gameOver = false;
        paused = false;
        score = 0;
        speed = INITIAL_SPEED;

        if (keyboard) {
            while (keyboard->kbhit()) {
                keyboard->getch();
            }
        }

        spawnFood();
    }

    void showWelcomeScreen() {
        clearScreen();
        std::cout << "\n\n";
        std::cout << GREEN << BOLD;
        std::cout << "    ███████╗███╗   ██╗ █████╗ ██╗  ██╗███████╗\n";
        std::cout << "    ██╔════╝████╗  ██║██╔══██╗██║ ██╔╝██╔════╝\n";
        std::cout << "    ███████╗██╔██╗ ██║███████║█████╔╝ █████╗  \n";
        std::cout << "    ╚════██║██║╚██╗██║██╔══██║██╔═██╗ ██╔══╝  \n";
        std::cout << "    ███████║██║ ╚████║██║  ██║██║  ██╗███████╗\n";
        std::cout << "    ╚══════╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝\n";
        std::cout << RESET << "\n\n";

        std::cout << CYAN << "  ╔════════════════════════════════════════╗\n";
        std::cout << "  ║  " << WHITE << "Classic Snake Game in C++        " << CYAN << "     ║\n";
        std::cout << "  ╚════════════════════════════════════════╝" << RESET << "\n\n";

        std::cout << YELLOW << "  How to Play:\n" << RESET;
        std::cout << "  • Use " << GREEN << "WASD" << RESET << " or " << GREEN << "Arrow Keys" << RESET << " to move\n";
        std::cout << "  • Eat the " << RED << "red food" << RESET << " to grow\n";
        std::cout << "  • Don't hit walls or yourself!\n";
        std::cout << "  • Press " << MAGENTA << "SPACE" << RESET << " to pause\n";
        std::cout << "  • The game speeds up as you score!\n\n";

        std::cout << BOLD << "  Press ENTER to start..." << RESET << std::flush;

        // Use simple getline before keyboard is initialized
        std::string dummy;
        std::getline(std::cin, dummy);

        // Clear cin state and flush any remaining input
        std::cin.clear();
        std::cin.sync();

        // Now initialize keyboard input (switches to non-blocking mode)
        keyboard.reset(new KeyboardInput());

        // Clear any buffered input after mode switch
        if (keyboard) {
            while (keyboard->kbhit()) {
                keyboard->getch();
            }
        }

        // Small delay to ensure terminal is ready
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    bool showGameOverScreen() {
        printf("\033[2J\033[1;1H");  // Clear screen and home cursor
        fflush(stdout);
        std::cout << "\n\n";
        std::cout << RED << BOLD;
        std::cout << "    ╔═══════════════════════════════════════╗\n";
        std::cout << "    ║                                       ║\n";
        std::cout << "    ║               GAME OVER!              ║\n";
        std::cout << "    ║                                       ║\n";
        std::cout << "    ╚═══════════════════════════════════════╝\n";
        std::cout << RESET << "\n";

        std::cout << YELLOW << "    Final Score: " << BOLD << score << RESET << "\n";
        std::cout << MAGENTA << "    Snake Length: " << BOLD << snake.size() << RESET << "\n\n";

        if (score >= 50) {
            std::cout << GREEN << BOLD << "    🏆 LEGENDARY! You're a Snake Master! 🏆\n" << RESET;
        } else if (score >= 30) {
            std::cout << CYAN << BOLD << "    ⭐ AMAZING! Excellent skills! ⭐\n" << RESET;
        } else if (score >= 15) {
            std::cout << BLUE << BOLD << "    👍 Great job! Keep practicing!\n" << RESET;
        } else {
            std::cout << WHITE << "    Good try! Practice makes perfect!\n" << RESET;
        }

        std::cout << "\n    " << WHITE << "Press " << GREEN << "R" << WHITE << " to play again or " << RED << "Q" << WHITE << " to quit..." << RESET << std::flush;

        // Clear any buffered input first
        while (keyboard->kbhit()) {
            keyboard->getch();
        }
        
        char key = 0;
        do {
            if (keyboard->kbhit()) {
                key = keyboard->getch();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } while (key != 'r' && key != 'R' && key != 'q' && key != 'Q');

        if (key == 'r' || key == 'R') {
            return true;
        }

        return false;
    }

    void run() {
        bool keepPlaying = true;

        while (keepPlaying) {
            initTerminal();
            reset();

            while (!gameOver) {
                processInput();

                if (!paused) {
                    moveSnake();
                }

                render();

                std::this_thread::sleep_for(std::chrono::milliseconds(speed));
            }

            // Show game over screen while still in alternate buffer
            keepPlaying = showGameOverScreen();
            
            // Exit alternate buffer before next iteration or exit
            restoreTerminal();
            
            // Small delay before re-initializing if replaying
            if (keepPlaying) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
};

int main() {
    SnakeGame game;
    game.showWelcomeScreen();
    game.run();

    std::cout << "\n\n  " << CYAN << "Thanks for playing! 🐍\n\n" << RESET;

    return 0;
}
