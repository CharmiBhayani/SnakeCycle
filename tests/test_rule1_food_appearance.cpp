#include <gtest/gtest.h>
#include "snake_under_test.h"

// Rule 1: Food only appears once in a board and the food disappears once eaten.
// Verifies food occupies a single position on the board.
// Full rule ("disappears once eaten") cannot be asserted because Game::update()
// and foodEaten are private, and eraseFood directly calls Console terminal I/O.
TEST(Rule1_FoodAppearance, Food_occupies_single_valid_board_coordinate) {
    // Arrange
    Food food;
    std::vector<Position> emptySnakeBody;

    // Act
    food.generateFood(30, 20, emptySnakeBody);

    // Assert
    Position pos = food.getPosition();
    EXPECT_TRUE(pos.x >= 0 && pos.x < 30 && pos.y >= 0 && pos.y < 20);
}
