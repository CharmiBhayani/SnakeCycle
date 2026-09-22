# Lab 4 — Group A__

> Copy this file to `lab4/REPORT.md`, fill it in, then paste the finished text into your
> pull request description. Keep the six headings exactly as they are — they are the
> marking scheme, in order. Delete this quote block and every `___` when you are done.

| | |
|---|---|
| Repository | ___ (your own snake game) |
| Base tag | `lab4-base` at commit `b9ea0e022dc4f928cd0c4006ce12b77319301cf3` |
| Pull request | ___ |

---

## 1. Five rules — [5]

Written before opening the source. Behaviour, with an observable outcome.

| # | Rule |
|---|---|
| 1 | Food only appears once in a board and the food disappears once eaten. |
| 2 | Eating fruit increases the score by exactly 10 points unless it's a special fruit which increases the score by 50 points. |
| 3 | Fruit never appears on a cell the snake occupies. |
| 4 | A game always starts with a new snake and a score of 0. |
| 5 | The game ends when the snake collides with its own body or the walls. |

If you could not state one of your own game's rules without going to look, say which and
why. It costs no marks.

We had to check the exact points for the special fruit to make sure it was correct. [Eating fruit increases the score by exactly 10 points unless it's a special fruit which increases the score by 50 points. I couldn't remember the exact score without looking at the code]

---

## 2. What you could test, and what stopped you — [10]

No source changes in this part. Every `file:line` below is a line in `lab4-base`.

| # | Rule | Test written? | Blocking dependency (`file:line` + what it is) |
|---|---|---|---|
| 1 | Food only appears once in a board and the food disappears once eaten. | No | `snakeCycle.cpp:571` — `foodEaten` is private in `Game` with no getter to observe, and `snakeCycle.cpp:437` — `eraseFood()` directly writes ANSI escape codes to the terminal. |
| 2 | Eating fruit increases the score by exactly 10 points unless it's a special fruit which increases the score by 50 points. | No | `snakeCycle.cpp:256` — `rand() % 10` is used inside `generateFood()` to choose fruit type with no way to seed or force it; `snakeCycle.cpp:563` — `score` is private in `Game` with no getter to verify points added. |
| 3 | Fruit never appears on a cell the snake occupies. | Yes | None. `Food::generateFood()` accepts the snake body vector, allowing deterministic testing on a 2x2 board where 3 cells are occupied. |
| 4 | A game always starts with a new snake and a score of 0. | No | `snakeCycle.cpp:563` — `score` is a private variable in `Game` with no getter; `snakeCycle.cpp:681` — `Game::Game()` calls `Console::setWindowSize()` which requires a real Windows console window. |
| 5 | The game ends when the snake collides with its own body or the walls. | No | `snakeCycle.cpp:565` — `gameOver` flag is private in `Game` with no getter; `snakeCycle.cpp:828` — `Game::run()` calls `Console::getch()` in a loop, blocking automated execution. |

> **Rules testable without modifying the source: 1 / 5**

"It needs user input" is not a blocking dependency. `main.cpp:214 — getch() called inside
the game loop` is.

Only Rule 3 could be fully tested as an end-to-end rule without modifying the source code, because `Food::generateFood()` takes the snake's body vector as an explicit argument and contains pure coordinate logic. The remaining four rules could not be verified at the game level because the `Game` class keeps its critical state (`score`, `gameOver`, `foodEaten`) in private fields with no getter functions, tightly couples the game loop to interactive console calls (`Console::getch()` and `Console::sleep()`), and uses non-deterministic `rand()` inside `Food::generateFood()`. While smaller isolated helper methods (like `Snake::checkSelfCollision()` and `GameBoard::isValidPosition()`) were testable on their own, the actual game outcomes stated in the rules could not be observed without modifying the source to add seams.

---

## 3. Coverage, and what it missed — [6]

| | |
|---|---|
| Line coverage | 53.2 % |
| Branch coverage | 43.0 % |
| Command used | `cmake --build build; .\build\runTests.exe; cd build/CMakeFiles/runTests.dir/tests; gcov -b -s ../../../.. *.gcno` |

**One rule that is executed by the suite but not verified by it:**

| | |
|---|---|
| Rule | Rule 2: Eating fruit increases the score by exactly 10 points unless it's a special fruit which increases the score by 50 points. |
| Line that runs | `snakeCycle.cpp:263` (`value = 10;` inside `Food::generateFood()`) |
| The assertion that is missing | `EXPECT_EQ(food.getValue(), 10);` (in `test_rule3_fruit_placement.cpp`, `generateFood()` runs and sets fruit value to 10 on line 263, but the test only asserts position and never checks the value). |

During `test_rule3_fruit_placement.cpp`, `food.generateFood(2, 2, snakeBody)` is called to test coordinate placement. While finding a free cell, this function also executes lines 260-264 assigning `value = 10;` and `symbol = '*';`. Because the test only asserts `food.getPosition()`, line 263 is marked as covered by gcov even though no assertion in that test checks whether the fruit's point value was set correctly.

---

## 4. The seam — [10]

| | |
|---|---|
| Rule made testable | Rule 2: Eating fruit increases the score by exactly 10 points unless it's a special fruit which increases the score by 50 points. |
| Commit 1 (seam) | `ae9ba46737596889e1daaa944d7279e8b5f2aecd` |
| Commit 2 (test) | `e65c14183170683df681f0b2cdcdf63d5572c13a` |
| Seam kind | object |
| Enabling point | `IRandom* rng = nullptr` parameter on `Food::generateFood()` (`snakeCycle.cpp:250`) |
| What production code gave up | Nothing. The default parameter value (`nullptr`) falls back to the existing `rand() % max` implementation, preserving 100% of runtime behavior, interface compatibility, and performance for the player with zero degradation. |

The seam cost nothing because it introduced an optional collaborator interface with a default fallback to standard `rand()`. All existing callers continue calling `generateFood()` without passing an argument, leaving game mechanics completely identical.

---

## 5. The double — [4]

| | |
|---|---|
| What you passed through the seam | stub |
| The method under test | `Food::generateFood` |

The collaborator was asked a question (`nextInt(10)` returns an integer) rather than told to do something (a `void` command). Because a method that returns a value supplies canned answers to feed indirect inputs into the SUT rather than asserting that an action happened, the double is a stub.

---

## 6. Two smells in your own tests — [5]

| | Smell | `file:line` | One-line fix |
|---|---|---|---|
| 1 | Mystery guest | `tests/test_rule1_food_appearance.cpp:14` | Pass a `StubRandom` via the `IRandom` seam so the input coordinate generation is visible directly in the test. |
| 2 | Assertion roulette | `tests/test_rule1_food_appearance.cpp:18` | Add an assertion message `<< "Coordinate (" << pos.x << "," << pos.y << ") out of bounds"` or check boundaries individually. |

Fixing them is optional. Finding them is not.

In `tests/test_rule1_food_appearance.cpp:14`, `generateFood()` secretly depends on the global, unseeded C-library `rand()` state outside the test, making the generated position an invisible "mystery guest." In line 18, `EXPECT_TRUE(pos.x >= 0 && pos.x < 30 && pos.y >= 0 && pos.y < 20)` is an assertion roulette: if it fails, GoogleTest only reports `Actual: false, Expected: true` with no message indicating which of the four boundary inequalities failed or what coordinates were evaluated.
