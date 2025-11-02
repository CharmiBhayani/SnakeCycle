#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <string>

enum ConsoleColor {
    BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3, RED = 4, MAGENTA = 5, YELLOW = 6, WHITE = 7,
    GRAY = 8, LIGHT_BLUE = 9, LIGHT_GREEN = 10, LIGHT_CYAN = 11, LIGHT_RED = 12,
    LIGHT_MAGENTA = 13, LIGHT_YELLOW = 14, BRIGHT_WHITE = 15
};

class Console {
public:
    static void setColor(int color) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
    }

    static void gotoxy(int x, int y) {
        COORD coord;
        coord.X = x;
        coord.Y = y;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    static void hideCursor() {
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    }

    static void showCursor() {
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    }

    static void clearScreen() {
        system("cls");
    }

    static void setWindowSize(int width, int height) {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD bufferSize = {static_cast<SHORT>(width), static_cast<SHORT>(height)};
        SetConsoleScreenBufferSize(hOut, bufferSize);

        SMALL_RECT windowSize = {0, 0, static_cast<SHORT>(width-1), static_cast<SHORT>(height-1)};
        SetConsoleWindowInfo(hOut, TRUE, &windowSize);
    }
};

struct Position {
    int x, y;

    Position(int x = 0, int y = 0) : x(x), y(y) {}

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

enum Direction {
    UP, DOWN, LEFT, RIGHT, STOP
};

class Food {
private:
    Position position;
    char symbol;
    int color;
    int value;

public:
    Food() : position(0, 0), symbol('*'), color(LIGHT_RED), value(10) {}

    void generateFood(int width, int height, const std::vector<Position>& snakeBody) {
        do {
            position.x = rand() % width;
            position.y = rand() % height;
        } while (std::find(snakeBody.begin(), snakeBody.end(), position) != snakeBody.end());

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

class Snake {
private:
    std::vector<Position> body;
    std::vector<Position> previousBody;  // Store previous frame's body
    Direction direction;
    bool growing;

public:
    Snake() : direction(STOP), growing(false) {
        body.push_back(Position(10, 10));
        body.push_back(Position(9, 10));
        body.push_back(Position(8, 10));
        previousBody = body;
    }

    void setDirection(Direction dir) {
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

        // Save current body as previous
        previousBody = body;

        Position head = body[0];

        switch (direction) {
            case UP:    head.y--; break;
            case DOWN:  head.y++; break;
            case LEFT:  head.x--; break;
            case RIGHT: head.x++; break;
        }

        body.insert(body.begin(), head);

        if (!growing) {
            body.pop_back();
        } else {
            growing = false;
        }
    }

    void grow() {
        growing = true;
    }

    Position getHead() const {
        return body.empty() ? Position(0, 0) : body[0];
    }

    Position getTail() const {
        return body.empty() ? Position(0, 0) : body[body.size() - 1];
    }

    const std::vector<Position>& getBody() const {
        return body;
    }

    const std::vector<Position>& getPreviousBody() const {
        return previousBody;
    }

    bool checkSelfCollision() const {
        if (body.size() <= 1) return false;

        Position head = body[0];
        for (size_t i = 1; i < body.size(); i++) {
            if (head == body[i]) {
                return true;
            }
        }
        return false;
    }

    int getLength() const {
        return static_cast<int>(body.size());
    }

    void reset() {
        body.clear();
        body.push_back(Position(10, 10));
        body.push_back(Position(9, 10));
        body.push_back(Position(8, 10));
        previousBody = body;
        direction = STOP;
        growing = false;
    }
};

class GameBoard {
private:
    int width, height;
    bool borderDrawn;
    int lastScore;
    int lastHighScore;
    int lastLength;
    std::string lastLevel;

public:
    GameBoard(int w = 30, int h = 20) : width(w), height(h), borderDrawn(false),
                                         lastScore(-1), lastHighScore(-1), lastLength(-1), lastLevel("") {}

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

        // Erase positions that are no longer part of the snake
        for (const Position& pos : prevBody) {
            if (std::find(currentBody.begin(), currentBody.end(), pos) == currentBody.end()) {
                if (isValidPosition(pos)) {
                    Console::gotoxy(pos.x + 1, pos.y + 4);
                    std::cout << ' ';
                }
            }
        }

        // Draw head
        if (!currentBody.empty()) {
            Position head = currentBody[0];
            if (isValidPosition(head)) {
                Console::gotoxy(head.x + 1, head.y + 4);
                Console::setColor(LIGHT_GREEN);
                switch (snake.getDirection()) {
                    case UP:    std::cout << '^'; break;
                    case DOWN:  std::cout << 'v'; break;
                    case LEFT:  std::cout << '<'; break;
                    case RIGHT: std::cout << '>'; break;
                    default:    std::cout << '@'; break;
                }
            }
        }

        // Draw body segments
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

    void displayHeader(int score, int highScore, int length, const std::string& level) {
        // Only draw static elements once
        static bool headerDrawn = false;
        if (!headerDrawn) {
            Console::setColor(LIGHT_CYAN);
            Console::gotoxy(0, 0);
            std::cout << "+========================================================+";
            Console::gotoxy(0, 1);
            std::cout << "|                    SNAKE GAME                        |";
            Console::gotoxy(0, 2);
            std::cout << "+========================================================+";

            Console::setColor(YELLOW);
            Console::gotoxy(width + 5, 5);
            std::cout << "+----------- STATS -----------+";
            Console::gotoxy(width + 5, 10);
            std::cout << "+----------------------------+";

            Console::setColor(LIGHT_MAGENTA);
            Console::gotoxy(width + 5, 12);
            std::cout << "+--------- CONTROLS ---------+";
            Console::setColor(WHITE);
            Console::gotoxy(width + 5, 13);
            std::cout << "| W/UP    - Move Up          |";
            Console::gotoxy(width + 5, 14);
            std::cout << "| S/DOWN  - Move Down        |";
            Console::gotoxy(width + 5, 15);
            std::cout << "| A/LEFT  - Move Left        |";
            Console::gotoxy(width + 5, 16);
            std::cout << "| D/RIGHT - Move Right       |";
            Console::gotoxy(width + 5, 17);
            std::cout << "| P       - Pause Game       |";
            Console::gotoxy(width + 5, 18);
            std::cout << "| Q       - Quit Game        |";
            Console::setColor(LIGHT_MAGENTA);
            Console::gotoxy(width + 5, 19);
            std::cout << "+----------------------------+";

            Console::setColor(CYAN);
            Console::gotoxy(width + 5, 21);
            std::cout << "+------ FOOD TYPES ------+";
            Console::gotoxy(width + 5, 22);
            Console::setColor(LIGHT_RED);
            std::cout << "| * ";
            Console::setColor(WHITE);
            std::cout << "- Normal Food (+10)  |";
            Console::gotoxy(width + 5, 23);
            Console::setColor(LIGHT_YELLOW);
            std::cout << "| $ ";
            Console::setColor(WHITE);
            std::cout << "- Special Food (+50) |";
            Console::setColor(CYAN);
            Console::gotoxy(width + 5, 24);
            std::cout << "+------------------------+";

            headerDrawn = true;
        }

        // Update only changed stats
        if (score != lastScore) {
            Console::gotoxy(width + 5, 6);
            Console::setColor(WHITE);
            std::cout << "| Score:      " << std::setw(10) << score << "       |";
            lastScore = score;
        }

        if (highScore != lastHighScore) {
            Console::gotoxy(width + 5, 7);
            Console::setColor(WHITE);
            std::cout << "| High Score: " << std::setw(10) << highScore << "       |";
            lastHighScore = highScore;
        }

        if (length != lastLength) {
            Console::gotoxy(width + 5, 8);
            Console::setColor(WHITE);
            std::cout << "| Length:     " << std::setw(10) << length << "       |";
            lastLength = length;
        }

        if (level != lastLevel) {
            Console::gotoxy(width + 5, 9);
            Console::setColor(WHITE);
            std::cout << "| Level:      " << std::setw(10) << level << "       |";
            lastLevel = level;
        }
    }

    bool isValidPosition(const Position& pos) const {
        return pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height;
    }

    void resetDrawnFlags() {
        borderDrawn = false;
        lastScore = -1;
        lastHighScore = -1;
        lastLength = -1;
        lastLevel = "";
    }
};

class Game {
private:
    Snake snake;
    Food food;
    GameBoard board;
    int score;
    int highScore;
    bool gameOver;
    bool gameRunning;
    bool paused;
    int gameSpeed;
    std::string currentLevel;
    Position oldFoodPos;
    bool foodEaten;

    void loadHighScore() {
        highScore = 0;
    }

    void saveHighScore() {
        if (score > highScore) {
            highScore = score;
        }
    }

    void updateGameSpeed() {
        int level = score / 100 + 1;
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
        std::cout << "|             WELCOME TO SNAKE                  |";
        Console::gotoxy(15, 8);
        std::cout << "|                                                |";
        Console::gotoxy(15, 9);
        std::cout << "|            Enhanced Version v2.1              |";
        Console::gotoxy(15, 10);
        std::cout << "|                                                |";
        Console::gotoxy(15, 11);
        std::cout << "+================================================+";

        Console::setColor(WHITE);
        Console::gotoxy(15, 12);
        std::cout << "|  INSTRUCTIONS:                                 |";
        Console::gotoxy(15, 13);
        std::cout << "|  * Use WASD or Arrow Keys to control snake    |";
        Console::gotoxy(15, 14);
        std::cout << "|  * Eat food (*) to grow and gain points       |";
        Console::gotoxy(15, 15);
        std::cout << "|  * Special food ($) gives bonus points        |";
        Console::gotoxy(15, 16);
        std::cout << "|  * Avoid hitting walls or yourself            |";
        Console::gotoxy(15, 17);
        std::cout << "|  * Press P to pause, Q to quit                |";
        Console::gotoxy(15, 18);
        std::cout << "|  * Game speed increases with your score!      |";

        Console::setColor(LIGHT_CYAN);
        Console::gotoxy(15, 19);
        std::cout << "+================================================+";
        Console::gotoxy(15, 20);
        std::cout << "|                                                |";
        Console::setColor(LIGHT_YELLOW);
        Console::gotoxy(15, 21);
        std::cout << "|        Press any key to start playing!        |";
        Console::setColor(LIGHT_CYAN);
        Console::gotoxy(15, 22);
        std::cout << "|                                                |";
        Console::gotoxy(15, 23);
        std::cout << "+================================================+";

        while (_kbhit()) _getch(); // Clear input buffer
        _getch();
    }

    void showGameOverScreen() {
        int boardWidth = board.getWidth();
        int boardHeight = board.getHeight();

        Console::setColor(LIGHT_RED);
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 2);
        std::cout << "+==================+";
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 3);
        std::cout << "|    GAME  OVER!   |";
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 4);
        std::cout << "+==================+";
        Console::setColor(WHITE);
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 5);
        std::cout << "| Final Score: " << std::setw(3) << score << " |";
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 6);
        std::cout << "| High Score:  " << std::setw(3) << highScore << " |";
        Console::setColor(LIGHT_RED);
        Console::gotoxy(boardWidth / 2 - 10, boardHeight / 2 + 7);
        std::cout << "+==================+";

        Console::setColor(YELLOW);
        Console::gotoxy(boardWidth / 2 - 15, boardHeight / 2 + 9);
        std::cout << "Press 'R' to restart or 'Q' to quit";
    }

public:
    Game() : score(0), highScore(0), gameOver(false), gameRunning(true),
             paused(false), gameSpeed(150), currentLevel("Level 1"),
             oldFoodPos(0, 0), foodEaten(false) {
        srand(static_cast<unsigned int>(time(0)));
        loadHighScore();
        Console::setWindowSize(80, 30);
        food.generateFood(board.getWidth(), board.getHeight(), snake.getBody());
        oldFoodPos = food.getPosition();
    }

    void processInput() {
        if (_kbhit()) {
            int key = _getch();

            // Handle extended keys (arrow keys, function keys, etc.)
            if (key == 0 || key == 224) { // Extended key prefix
                key = _getch(); // Get the actual key code
                switch (key) {
                    case 72: snake.setDirection(UP); break;    // Up arrow
                    case 80: snake.setDirection(DOWN); break;  // Down arrow
                    case 75: snake.setDirection(LEFT); break;  // Left arrow
                    case 77: snake.setDirection(RIGHT); break; // Right arrow
                    default: break;
                }
            } else {
                key = tolower(key);
                switch (key) {
                    case 'w': snake.setDirection(UP); break;
                    case 's': snake.setDirection(DOWN); break;
                    case 'a': snake.setDirection(LEFT); break;
                    case 'd': snake.setDirection(RIGHT); break;
                    case 'p': paused = !paused; break;
                    case 'q': gameRunning = false; break;
                    default: break;
                }
            }
        }
    }

    void update() {
        if (gameOver || paused) return;

        foodEaten = false;

        snake.move();
        updateGameSpeed();

        Position head = snake.getHead();
        if (!board.isValidPosition(head)) {
            gameOver = true;
            return;
        }

        if (snake.checkSelfCollision()) {
            gameOver = true;
            return;
        }

        if (head == food.getPosition()) {
            score += food.getValue();
            snake.grow();
            foodEaten = true;
            oldFoodPos = food.getPosition();
            food.generateFood(board.getWidth(), board.getHeight(), snake.getBody());
        }
    }

    void render() {
        board.drawBorder();

        // Erase old food if eaten
        if (foodEaten) {
            board.eraseFood(oldFoodPos);
        }

        // Draw snake (handles erasing old positions automatically)
        board.drawSnake(snake);

        board.drawFood(food);
        board.displayHeader(score, highScore, snake.getLength(), currentLevel);

        if (paused) {
            Console::setColor(LIGHT_YELLOW);
            Console::gotoxy(board.getWidth() / 2 - 3, board.getHeight() / 2 + 4);
            std::cout << "PAUSED";
        }

        if (gameOver) {
            showGameOverScreen();
        }
    }

    void handleGameOver() {
        if (!gameOver) return;

        saveHighScore();

        while (_kbhit()) _getch(); // Clear input buffer

        char choice = _getch();
        choice = tolower(choice);

        if (choice == 'r') {
            restart();
        } else if (choice == 'q') {
            gameRunning = false;
        }
    }

    void restart() {
        snake.reset();
        score = 0;
        gameOver = false;
        paused = false;
        gameSpeed = 150;
        currentLevel = "Level 1";
        food.generateFood(board.getWidth(), board.getHeight(), snake.getBody());
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
                Sleep(gameSpeed);
            } else {
                Sleep(100);
            }
        }

        Console::clearScreen();
        Console::setColor(LIGHT_CYAN);
        Console::gotoxy(25, 10);
        std::cout << "Thanks for playing Snake Game!";
        Console::gotoxy(25, 11);
        std::cout << "Final Score: " << score;
        Console::gotoxy(25, 12);
        std::cout << "High Score: " << highScore;
        Console::gotoxy(0, 15);
        Console::showCursor();
        Console::setColor(WHITE);
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
