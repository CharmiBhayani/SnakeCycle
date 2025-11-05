Snake Game in C++

A simple console-based Snake game written in C++ with support for **Windows and Linux**.  
Move the snake, collect food, grow longer, and avoid hitting walls or yourself.

Features
- Works on Windows and Linux
- Real-time keyboard input (WASD / Arrow keys)
- Score increases as you eat food
- Bonus food appears sometimes
- Pause and Quit options

Controls
| Key | Action |
|-----|--------|
| W or ↑ | Move Up |
| S or ↓ | Move Down |
| A or ← | Move Left |
| D or → | Move Right |
| P | Pause |
| Q | Quit |

How to Compile and Run

On Linux (Ubuntu / WSL)
```bash
g++ snake.cpp -o snake
./snake

On windows
g++ snake.cpp -o snake
snake.exe

