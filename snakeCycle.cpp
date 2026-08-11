
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <string>
#include <csignal>
#include <cstdio>
#include <cctype>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
    #include <sys/ioctl.h>
    #include <sys/select.h>
    #include <fcntl.h>
    #include <signal.h>
#endif

// ---------------------------- Console Colors ----------------------------
enum ConsoleColor {
    BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3, RED = 4, MAGENTA = 5, YELLOW = 6, WHITE = 7,
    GRAY = 8, LIGHT_BLUE = 9, LIGHT_GREEN = 10, LIGHT_CYAN = 11, LIGHT_RED = 12,
    LIGHT_MAGENTA = 13, LIGHT_YELLOW = 14, BRIGHT_WHITE = 15
};

// ---------------------------- Console Utilities ----------------------------
class Console {
private:
#ifndef _WIN32
    static struct termios oldSettings;
    static bool terminalConfigured;
#endif

    // Helper for Linux to set non-blocking canonical off
#ifndef _WIN32
    static void configureTerminal() {
        if (!terminalConfigured) {
            tcgetattr(STDIN_FILENO, &oldSettings);
            struct termios newSettings = oldSettings;
            newSettings.c_lflag &= ~ICANON; // turn off canonical mode
            newSettings.c_lflag &= ~ECHO;   // turn off echo
            newSettings.c_cc[VMIN] = 0;     // non-blocking read
            newSettings.c_cc[VTIME] = 0;
            tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
            terminalConfigured = true;
        }
    }

    static void restoreTerminal() {
        if (terminalConfigured) {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
            terminalConfigured = false;
        }
    }
#endif

public:
    // initialize (safe multi-call)
    static void initialize() {
#ifdef _WIN32
        // nothing to do
#else
        configureTerminal();
#endif
    }

    static void cleanup() {
#ifdef _WIN32
        // nothing to do
#else
        restoreTerminal();
        std::cout << "\033[0m"; // reset attributes
        std::cout.flush();
#endif
    }

    static void setColor(int color) {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, static_cast<WORD>(color));
#else
        const char* ansiColors[] = {
            "\033[0;30m", "\033[0;34m", "\033[0;32m", "\033[0;36m",
            "\033[0;31m", "\033[0;35m", "\033[0;33m", "\033[0;37m",
            "\033[1;30m", "\033[1;34m", "\033[1;32m", "\033[1;36m",
            "\033[1;31m", "\033[1;35m", "\033[1;33m", "\033[1;37m"
        };
        if (color >= 0 && color < 16) {
            std::cout << ansiColors[color];
        }
#endif
    }

    static void gotoxy(int x, int y) {
#ifdef _WIN32
        COORD coord{ static_cast<SHORT>(x), static_cast<SHORT>(y) };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
        // ANSI 1-based coordinates
        std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";
        std::cout.flush();
#endif
    }

    static void hideCursor() {
#ifdef _WIN32
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
#else
        std::cout << "\033[?25l";
        std::cout.flush();
#endif
    }

    static void showCursor() {
#ifdef _WIN32
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
        cursorInfo.bVisible = TRUE;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
#else
        std::cout << "\033[?25h";
        std::cout.flush();
#endif
    }

    static void clearScreen() {
#ifdef _WIN32
        system("cls");
#else
        std::cout << "\033[2J\033[1;1H";
        std::cout.flush();
#endif
    }

    static void setWindowSize(int width, int height) {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD bufferSize = { static_cast<SHORT>(width), static_cast<SHORT>(height) };
        SetConsoleScreenBufferSize(hOut, bufferSize);
        SMALL_RECT windowSize = { 0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(height - 1) };
        SetConsoleWindowInfo(hOut, TRUE, &windowSize);
#else
        // Hint to many terminals; harmless if ignored.
        std::cout << "\033[8;" << height << ";" << width << "t";
        std::cout.flush();
#endif
    }

    // Non-blocking keyboard check
    static int kbhit() {
#ifdef _WIN32
        return _kbhit();
#else
        configureTerminal();
        struct timeval tv = { 0, 0 };
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
#endif
    }

    // Non-blocking getch: returns -1 if none
    static int getch_nonblocking() {
#ifdef _WIN32
        if (_kbhit()) return _getch();
        return -1;
#else
        configureTerminal();
        unsigned char ch;
        ssize_t n = read(STDIN_FILENO, &ch, 1);
        if (n == 1) return static_cast<int>(ch);
        return -1;
#endif
    }

    // Blocking getch (waits for one char). For menus/welcome screen.
    static int getch_blocking() {
#ifdef _WIN32
        return _getch();
#else
        // Temporarily switch to blocking read for one char
        struct termios saved, blocking;
        tcgetattr(STDIN_FILENO, &saved);
        blocking = saved;
        blocking.c_lflag &= ~ECHO;
        blocking.c_lflag &= ~ICANON;
        blocking.c_cc[VMIN] = 1;
        blocking.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &blocking);
        unsigned char ch;
        ssize_t n = read(STDIN_FILENO, &ch, 1);
        tcsetattr(STDIN_FILENO, TCSANOW, &saved);
        if (n == 1) return static_cast<int>(ch);
        return -1;
#endif
    }

    static int getch() { return getch_nonblocking(); }
    static int getch_wait() { return getch_blocking(); }

    static void sleep(int milliseconds) {
#ifdef _WIN32
        Sleep(milliseconds);
#else
        usleep(milliseconds * 1000);
#endif
    }
};

#ifndef _WIN32
struct termios Console::oldSettings;
bool Console::terminalConfigured = false;
#endif

// Restore terminal on signal
static void sigintHandler(int) {
    Console::cleanup();
    std::exit(0);
}

// ---------------------------- Core Game Types ----------------------------
struct Position {
    int x, y;
    Position(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Position& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Position& other) const { return !(*this == other); }
};

enum Direction { UP, DOWN, LEFT, RIGHT, STOP };

// ---------------------------- Food ----------------------------
class Food {
private:
    Position position;
    char symbol;
    int color;
    int value;
public:
    Food() : position(0, 0), symbol('*'), color(LIGHT_RED), value(10) {}
    void generateFood(int width, int height, const std::vector<Position>& body1, const std::vector<Position>& body2) {
        do {
            position.x = rand() % width;
            position.y = rand() % height;
        } while (std::find(body1.begin(), body1.end(), position) != body1.end() ||
                 std::find(body2.begin(), body2.end(), position) != body2.end());

        if (rand() % 10 == 0) {
            symbol = '$';
            color = LIGHT_YELLOW;
            value = 50;
        } else {
            symbol = '*';
            color = LIGHT_RED;
            value = 10;
        }
    }
    Position getPosition() const { return position; }
    char getSymbol() const { return symbol; }
    int getColor() const { return color; }
    int getValue() const { return value; }
};

// ---------------------------- Snake ----------------------------
class Snake {
private:
    std::vector<Position> body;
    std::vector<Position> previousBody;
    Direction direction;
    bool growing;
    int startX;
    int startY;
public:
    Snake(int x = 10, int y = 10) : direction(STOP), growing(false), startX(x), startY(y) {
        reset();
    }

    void setDirection(Direction dir) {
        // Prevent immediate reverse
        if ((direction == UP && dir != DOWN) ||
            (direction == DOWN && dir != UP) ||
            (direction == LEFT && dir != RIGHT) ||
            (direction == RIGHT && dir != LEFT) ||
            direction == STOP) {
            direction = dir;
        }
    }

    Direction getDirection() const { return direction; }

    void move() {
        if (direction == STOP) return;
        previousBody = body;
        Position head = body[0];

        switch (direction) {
            case UP: head.y--; break;
            case DOWN: head.y++; break;
            case LEFT: head.x--; break;
            case RIGHT: head.x++; break;
            default: break;
        }

        body.insert(body.begin(), head);
        if (!growing) {
            body.pop_back();
        } else {
            growing = false;
        }
    }

    void grow() { growing = true; }
    Position getHead() const { return body.empty() ? Position(0, 0) : body[0]; }
    const std::vector<Position>& getBody() const { return body; }
    const std::vector<Position>& getPreviousBody() const { return previousBody; }

    bool checkSelfCollision() const {
        if (body.size() <= 1) return false;
        Position head = body[0];
        for (size_t i = 1; i < body.size(); i++) {
            if (head == body[i]) return true;
        }
        return false;
    }

    int getLength() const { return static_cast<int>(body.size()); }

    void reset() {
        body.clear();
        body.emplace_back(startX, startY);
        body.emplace_back(startX - 1, startY);
        body.emplace_back(startX - 2, startY);
        previousBody = body;
        direction = STOP;
        growing = false;
    }
};

// ---------------------------- GameBoard (visual heavy) ----------------------------
class GameBoard {
private:
    int width, height;
    bool borderDrawn;
    bool headerDrawn;
    int lastScore1;
    int lastScore2;
    int lastHighScore;
    int lastLength1;
    int lastLength2;
    std::string lastLevel;
    bool wasPaused;

public:
    GameBoard(int w = 30, int h = 20)
        : width(w), height(h), borderDrawn(false), headerDrawn(false),
          lastScore1(-1), lastScore2(-1), lastHighScore(-1),
          lastLength1(-1), lastLength2(-1), lastLevel(""), wasPaused(false) {}

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void drawBorder() {
        if (borderDrawn) return;
        Console::setColor(CYAN);
        Console::gotoxy(0, 3);
        std::cout << "+";
        for (int i = 0; i < width; i++) std::cout << "-";
        std::cout << "+";

        for (int i = 0; i < height; i++) {
            Console::gotoxy(0, 4 + i);
            std::cout << "|";
            Console::gotoxy(width + 1, 4 + i);
            std::cout << "|";
        }

        Console::gotoxy(0, 4 + height);
        std::cout << "+";
        for (int i = 0; i < width; i++) std::cout << "-";
        std::cout << "+";
        borderDrawn = true;
    }

    void drawSnake(const Snake& snake) {
        const std::vector<Position>& currentBody = snake.getBody();
        const std::vector<Position>& prevBody = snake.getPreviousBody();

        for (const Position& pos : prevBody) {
            if (std::find(currentBody.begin(), currentBody.end(), pos) == currentBody.end()) {
                if (isValidPosition(pos)) {
                    Console::gotoxy(pos.x + 1, pos.y + 4);
                    std::cout << ' ';
                }
            }
        }

        if (!currentBody.empty()) {
            Position head = currentBody[0];
            if (isValidPosition(head)) {
                Console::gotoxy(head.x + 1, head.y + 4);
                Console::setColor(LIGHT_GREEN);
                switch (snake.getDirection()) {
                    case UP: std::cout << '^'; break;
                    case DOWN: std::cout << 'v'; break;
                    case LEFT: std::cout << '<'; break;
                    case RIGHT: std::cout << '>'; break;
                    default: std::cout << '@'; break;
                }
            }
        }

        Console::setColor(GREEN);
        for (size_t i = 1; i < currentBody.size(); i++) {
            Position segment = currentBody[i];
            if (isValidPosition(segment)) {
                Console::gotoxy(segment.x + 1, segment.y + 4);
                std::cout << 'o';
            }
        }
    }

    void drawFood(const Food& food) {
        Position pos = food.getPosition();
        if (isValidPosition(pos)) {
            Console::gotoxy(pos.x + 1, pos.y + 4);
            Console::setColor(food.getColor());
            std::cout << food.getSymbol();
        }
    }

    void eraseFood(const Position& pos) {
        if (isValidPosition(pos)) {
            Console::gotoxy(pos.x + 1, pos.y + 4);
            std::cout << ' ';
        }
    }

    void displayHeader(int score1, int score2, int highScore, int length1, int length2, const std::string& level) {
        if (!headerDrawn) {
            Console::setColor(LIGHT_CYAN);
            Console::gotoxy(0, 0);
            std::cout << "+========================================================+";
            Console::gotoxy(0, 1);
            std::cout << "|                    SNAKE GAME                          |";
            Console::gotoxy(0, 2);
            std::cout << "+========================================================+";

            Console::setColor(YELLOW);
            Console::gotoxy(width + 5, 4);
            std::cout << "+----------- STATS ----------+";
            Console::gotoxy(width + 5, 11);
            std::cout << "+----------------------------+";

            Console::setColor(LIGHT_MAGENTA);
            Console::gotoxy(width + 5, 13);
            std::cout << "+--------- CONTROLS ---------+";
            Console::setColor(WHITE);
            Console::gotoxy(width + 5, 14);
            std::cout << "| P1 (Arrows)  P2 (WASD)     |";
            Console::gotoxy(width + 5, 15);
            std::cout << "| P - Pause Game             |";
            Console::gotoxy(width + 5, 16);
            std::cout << "| Q - Quit Game              |";
            Console::setColor(LIGHT_MAGENTA);
            Console::gotoxy(width + 5, 17);
            std::cout << "+----------------------------+";

            Console::setColor(CYAN);
            Console::gotoxy(width + 5, 19);
            std::cout << "+------ FOOD TYPES ------+";
            Console::gotoxy(width + 5, 20);
            Console::setColor(LIGHT_RED);
            std::cout << "| * ";
            Console::setColor(WHITE);
            std::cout << "- Normal Food (+10) |";
            Console::gotoxy(width + 5, 21);
            Console::setColor(LIGHT_YELLOW);
            std::cout << "| $ ";
            Console::setColor(WHITE);
            std::cout << "- Special Food (+50) |";
            Console::setColor(CYAN);
            Console::gotoxy(width + 5, 22);
            std::cout << "+------------------------+";

            headerDrawn = true;
        }

        if (score1 != lastScore1) {
            Console::gotoxy(width + 5, 5);
            Console::setColor(WHITE);
            std::cout << "| P1 Score: " << std::setw(14) << score1 << " |";
            lastScore1 = score1;
        }

        if (length1 != lastLength1) {
            Console::gotoxy(width + 5, 6);
            Console::setColor(WHITE);
            std::cout << "| P1 Length: " << std::setw(13) << length1 << " |";
            lastLength1 = length1;
        }

        if (score2 != lastScore2) {
            Console::gotoxy(width + 5, 7);
            Console::setColor(WHITE);
            std::cout << "| P2 Score: " << std::setw(14) << score2 << " |";
            lastScore2 = score2;
        }

        if (length2 != lastLength2) {
            Console::gotoxy(width + 5, 8);
            Console::setColor(WHITE);
            std::cout << "| P2 Length: " << std::setw(13) << length2 << " |";
            lastLength2 = length2;
        }

        if (highScore != lastHighScore) {
            Console::gotoxy(width + 5, 9);
            Console::setColor(WHITE);
            std::cout << "| High Score: " << std::setw(12) << highScore << " |";
            lastHighScore = highScore;
        }

        if (level != lastLevel) {
            Console::gotoxy(width + 5, 10);
            Console::setColor(WHITE);
            std::cout << "| Level: " << std::setw(17) << level << " |";
            lastLevel = level;
        }
    }

    void displayPauseMessage(bool paused) {
        if (paused && !wasPaused) {
            Console::setColor(LIGHT_YELLOW);
            Console::gotoxy(width / 2 - 3, height / 2 + 4);
            std::cout << "PAUSED";
            wasPaused = true;
        } else if (!paused && wasPaused) {
            Console::gotoxy(width / 2 - 3, height / 2 + 4);
            std::cout << "      "; // clear
            wasPaused = false;
        }
    }

    bool isValidPosition(const Position& pos) const {
        return pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height;
    }

    void resetDrawnFlags() {
        borderDrawn = false;
        headerDrawn = false;
        lastScore1 = -1;
        lastScore2 = -1;
        lastHighScore = -1;
        lastLength1 = -1;
        lastLength2 = -1;
        lastLevel = "";
        wasPaused = false;
    }
};

// ---------------------------- Game (full UI, input fixed) ----------------------------
class Game {
private:
    Snake snake1;
    Snake snake2;
    Food food;
    GameBoard board;
    int score1;
    int score2;
    int highScore;
    bool gameOver;
    bool gameRunning;
    bool paused;
    int gameSpeed;
    std::string currentLevel;
    Position oldFoodPos;
    bool foodEaten;
    int lostPlayer;

    void loadHighScore() {
        // stub: could read from a file. Keep 0 if none.
        highScore = 0;
    }

    void saveHighScore() {
        int maxScore = std::max(score1, score2);
        if (maxScore > highScore) highScore = maxScore;
        // stub: could write to a file
    }

    void updateGameSpeed() {
        int maxScore = std::max(score1, score2);
        int level = maxScore / 100 + 1;
        gameSpeed = std::max(50, 200 - (level * 15));
        currentLevel = "Level " + std::to_string(level);
    }

    void showWelcomeScreen() {
        Console::clearScreen();
        Console::hideCursor();
        Console::setColor(LIGHT_CYAN);
        Console::gotoxy(15, 5);
        std::cout << "+================================================+";
        Console::gotoxy(15, 6);
        std::cout << "|                                                |";
        Console::gotoxy(15, 7);
        std::cout << "|          WELCOME TO SNAKE GAME                 |";
        Console::gotoxy(15, 8);
        std::cout << "|                                                |";
        Console::gotoxy(15, 9);
        std::cout << "|                                                |";
        Console::gotoxy(15, 10);
        std::cout << "|                                                |";
        Console::gotoxy(15, 11);
        std::cout << "+================================================+";

        Console::setColor(WHITE);
        Console::gotoxy(15, 12);
        std::cout << "| INSTRUCTIONS:                                  |";
        Console::gotoxy(15, 13);
        std::cout << "| * Use WASD or Arrow Keys to control snake      |";
        Console::gotoxy(15, 14);
        std::cout << "| * Eat food (*) to grow and gain points         |";
        Console::gotoxy(15, 15);
        std::cout << "| * Special food ($) gives bonus points          |";
        Console::gotoxy(15, 16);
        std::cout << "| * Avoid hitting walls or yourself              |";
        Console::gotoxy(15, 17);
        std::cout << "| * Press P to pause, Q to quit                  |";
        Console::gotoxy(15, 18);
        std::cout << "| * Game speed increases with your score!        |";

        Console::setColor(LIGHT_CYAN);
        Console::gotoxy(15, 19);
        std::cout << "+================================================+";
        Console::gotoxy(15, 20);
        std::cout << "|                                                |";
        Console::setColor(LIGHT_YELLOW);
        Console::gotoxy(15, 21);
        std::cout << "|      Press any key to start playing!          |";
        Console::setColor(LIGHT_CYAN);
        Console::gotoxy(15, 22);
        std::cout << "|                                                |";
        Console::gotoxy(15, 23);
        std::cout << "+================================================+";

        while (Console::kbhit()) Console::getch(); // flush
        // blocking wait for key
        Console::getch_wait();
    }

    void showGameOverScreen() {
        int boardWidth = board.getWidth();
        int boardHeight = board.getHeight();

        Console::setColor(LIGHT_RED);
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 2);
        std::cout << "+==================+";
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 3);
        std::cout << "|   GAME OVER!     |";
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 4);
        if (lostPlayer == 1) {
            std::cout << "|  Player 1 Lost!  |";
        } else if (lostPlayer == 2) {
            std::cout << "|  Player 2 Lost!  |";
        } else {
            std::cout << "| Both Players Lost|";
        }
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 5);
        std::cout << "+==================+";

        Console::setColor(WHITE);
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 6);
        std::cout << "| P1 Score:    " << std::setw(3) << score1 << " |";
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 7);
        std::cout << "| P2 Score:    " << std::setw(3) << score2 << " |";
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 8);
        std::cout << "| High Score:  " << std::setw(3) << highScore << " |";

        Console::setColor(LIGHT_RED);
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 9);
        std::cout << "+==================+";

        Console::setColor(YELLOW);
        Console::gotoxy(boardWidth / 2 - 15, boardHeight / 2 + 11);
        std::cout << "Press 'R' to restart or 'Q' to quit";
    }

public:
    Game() : snake1(10, 10),
             snake2(20, 10),
             board(30, 20),
             score1(0), score2(0), highScore(0), gameOver(false), gameRunning(true),
             paused(false), gameSpeed(150), currentLevel("Level 1"),
             oldFoodPos(0,0), foodEaten(false), lostPlayer(0) {
        srand(static_cast<unsigned>(time(nullptr)));
        loadHighScore();
        Console::initialize();
        std::signal(SIGINT, sigintHandler);
        std::signal(SIGTERM, sigintHandler);

        Console::setWindowSize(80, 30);
        food.generateFood(board.getWidth(), board.getHeight(), snake1.getBody(), snake2.getBody());
        oldFoodPos = food.getPosition();
    }

    ~Game() {
        Console::cleanup();
    }

    void processInput() {
        if (!Console::kbhit()) return;
        int key = Console::getch(); // non-blocking read

#ifdef _WIN32
        if (key == 0 || key == 224) {
            int k2 = Console::getch_wait();
            switch (k2) {
                case 72: snake1.setDirection(UP); break;
                case 80: snake1.setDirection(DOWN); break;
                case 75: snake1.setDirection(LEFT); break;
                case 77: snake1.setDirection(RIGHT); break;
            }
        } else {
            key = std::tolower(key);
            switch (key) {
                case 'w': snake2.setDirection(UP); break;
                case 's': snake2.setDirection(DOWN); break;
                case 'a': snake2.setDirection(LEFT); break;
                case 'd': snake2.setDirection(RIGHT); break;
                case 'p': paused = !paused; break;
                case 'q': gameRunning = false; break;
            }
        }
#else
        // On Unix arrow keys are sequences: 27, '[', 'A'/'B'/'C'/'D'
        if (key == 27) {
            // small wait to allow sequence bytes to arrive
            Console::sleep(8);
            if (Console::kbhit()) {
                int next = Console::getch();
                if (next == '[') {
                    // wait a bit for final byte
                    Console::sleep(4);
                    if (Console::kbhit()) {
                        int code = Console::getch();
                        if (code == 'A') snake1.setDirection(UP);
                        else if (code == 'B') snake1.setDirection(DOWN);
                        else if (code == 'C') snake1.setDirection(RIGHT);
                        else if (code == 'D') snake1.setDirection(LEFT);
                    }
                }
            }
        } else {
            key = std::tolower(key);
            switch (key) {
                case 'w': snake2.setDirection(UP); break;
                case 's': snake2.setDirection(DOWN); break;
                case 'a': snake2.setDirection(LEFT); break;
                case 'd': snake2.setDirection(RIGHT); break;
                case 'p': paused = !paused; break;
                case 'q': gameRunning = false; break;
            }
        }
#endif
    }

    void update() {
        if (gameOver || paused) return;

        foodEaten = false;
        snake1.move();
        snake2.move();
        updateGameSpeed();

        Position head1 = snake1.getHead();
        Position head2 = snake2.getHead();

        bool p1Lost = false;
        bool p2Lost = false;

        // 1. Wall collision checks
        if (!board.isValidPosition(head1)) {
            p1Lost = true;
        }
        if (!board.isValidPosition(head2)) {
            p2Lost = true;
        }

        // 2. Self collision checks
        if (snake1.checkSelfCollision()) {
            p1Lost = true;
        }
        if (snake2.checkSelfCollision()) {
            p2Lost = true;
        }

        // 3. Cross collision checks
        const auto& body1 = snake1.getBody();
        const auto& body2 = snake2.getBody();

        for (const auto& segment : body2) {
            if (head1 == segment) {
                p1Lost = true;
                break;
            }
        }

        for (const auto& segment : body1) {
            if (head2 == segment) {
                p2Lost = true;
                break;
            }
        }

        if (head1 == head2) {
            p1Lost = true;
            p2Lost = true;
        }

        if (p1Lost || p2Lost) {
            gameOver = true;
            if (p1Lost && p2Lost) {
                lostPlayer = 3;
            } else if (p1Lost) {
                lostPlayer = 1;
            } else {
                lostPlayer = 2;
            }
            return;
        }

        // 4. Food collision checks
        if (head1 == food.getPosition()) {
            score1 += food.getValue();
            snake1.grow();
            foodEaten = true;
            oldFoodPos = food.getPosition();
            food.generateFood(board.getWidth(), board.getHeight(), snake1.getBody(), snake2.getBody());
        } else if (head2 == food.getPosition()) {
            score2 += food.getValue();
            snake2.grow();
            foodEaten = true;
            oldFoodPos = food.getPosition();
            food.generateFood(board.getWidth(), board.getHeight(), snake1.getBody(), snake2.getBody());
        }
    }

    void render() {
        board.drawBorder();

        if (foodEaten) {
            board.eraseFood(oldFoodPos);
        }

        board.drawSnake(snake1);
        board.drawSnake(snake2);
        board.drawFood(food);
        board.displayHeader(score1, score2, highScore, snake1.getLength(), snake2.getLength(), currentLevel);
        board.displayPauseMessage(paused);

        if (gameOver) {
            showGameOverScreen();
        }
    }

    void handleGameOver() {
        if (!gameOver) return;

        saveHighScore();
        while (Console::kbhit()) Console::getch();

        int choice = -1;
        while (true) {
            if (Console::kbhit()) {
                choice = Console::getch();
                if (choice == -1) { Console::sleep(50); continue; }
                choice = std::tolower(choice);
                if (choice == 'r') { restart(); break; }
                if (choice == 'q') { gameRunning = false; break; }
            } else Console::sleep(50);
        }
    }

    void restart() {
        snake1.reset();
        snake2.reset();
        score1 = 0;
        score2 = 0;
        gameOver = false;
        paused = false;
        gameSpeed = 150;
        currentLevel = "Level 1";
        lostPlayer = 0;
        food.generateFood(board.getWidth(), board.getHeight(), snake1.getBody(), snake2.getBody());
        Console::clearScreen();
        board.resetDrawnFlags();
    }

    bool isRunning() const { return gameRunning; }

    void run() {
        showWelcomeScreen();
        Console::clearScreen();

        while (gameRunning) {
            processInput();
            update();
            render();

            if (gameOver) {
                handleGameOver();
            } else if (!paused) {
                Console::sleep(gameSpeed);
            } else {
                Console::sleep(100);
            }
        }

        Console::clearScreen();
        Console::setColor(LIGHT_CYAN);
        Console::gotoxy(25, 10);
        std::cout << "Thanks for playing Snake Game!";
        Console::gotoxy(25, 11);
        std::cout << "Player 1 Score: " << score1;
        Console::gotoxy(25, 12);
        std::cout << "Player 2 Score: " << score2;
        Console::gotoxy(25, 13);
        std::cout << "High Score: " << highScore;
        Console::gotoxy(0, 15);
        Console::showCursor();
        Console::setColor(WHITE);
    }
};

// ---------------------------- main ----------------------------
int main() {
    std::signal(SIGINT, sigintHandler);
    std::signal(SIGTERM, sigintHandler);
    Game game;
    game.run();
    return 0;
}
