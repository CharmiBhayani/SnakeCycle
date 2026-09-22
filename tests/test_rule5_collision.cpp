#include <gtest/gtest.h>
#include "snake_under_test.h"

// Rule 5: The game ends when the snake collides with its own body or the walls.
// Verifies self-collision detection and boundary validation logic.
// Overall game termination (gameOver flag) cannot be asserted because Game::gameOver is private.
TEST(Rule5_Collision, Snake_detects_self_collision_when_head_hits_own_body) {
    // Arrange: Snake grows and loops back into its body
    Snake snake;
    snake.grow();
    snake.setDirection(UP);
    snake.move();
    snake.grow();
    snake.setDirection(LEFT);
    snake.move();
    snake.setDirection(DOWN);
    snake.move();

    // Act
    bool collided = snake.checkSelfCollision();

    // Assert
    EXPECT_TRUE(collided);
}

TEST(Rule5_Collision, Snake_reports_no_collision_when_body_is_unintersected) {
    // Arrange
    Snake snake;

    // Act
    bool collided = snake.checkSelfCollision();

    // Assert
    EXPECT_FALSE(collided);
}

TEST(Rule5_Collision, Board_detects_wall_collision_when_head_exceeds_boundary) {
    // Arrange
    GameBoard board(30, 20);
    Position outOfBounds(-1, 5);

    // Act
    bool isWithinBounds = board.isValidPosition(outOfBounds);

    // Assert
    EXPECT_FALSE(isWithinBounds);
}

TEST(Rule5_Collision, Board_confirms_position_is_valid_within_walls) {
    // Arrange
    GameBoard board(30, 20);
    Position inBounds(15, 10);

    // Act
    bool isWithinBounds = board.isValidPosition(inBounds);

    // Assert
    EXPECT_TRUE(isWithinBounds);
}
