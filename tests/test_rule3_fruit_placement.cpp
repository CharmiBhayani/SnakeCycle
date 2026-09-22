#include <gtest/gtest.h>
#include "snake_under_test.h"

// Rule 3: Fruit never appears on a cell the snake occupies.
// Fully testable without modifying source by constraining available cells.
TEST(Rule3_FruitPlacement, Fruit_never_appears_on_cell_occupied_by_snake) {
    // Arrange: 2x2 board with 3 cells occupied by snake body
    Food food;
    std::vector<Position> snakeBody = { Position(0, 0), Position(0, 1), Position(1, 0) };

    // Act: only (1, 1) is free
    food.generateFood(2, 2, snakeBody);

    // Assert: fruit is forced to land on the unoccupied cell
    EXPECT_EQ(food.getPosition(), Position(1, 1));
}
