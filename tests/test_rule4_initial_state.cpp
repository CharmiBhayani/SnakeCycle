#include <gtest/gtest.h>
#include "snake_under_test.h"

// Rule 4: A game always starts with a new snake and a score of 0.
// Verifies snake component spawns with initial 3 segments, stopped, and at default coords.
// Score starting at 0 cannot be asserted on Game because score is private with no getter.
TEST(Rule4_InitialState, New_snake_starts_with_three_body_segments) {
    // Arrange
    Snake snake;

    // Act
    int length = snake.getLength();

    // Assert
    EXPECT_EQ(length, 3);
}

TEST(Rule4_InitialState, New_snake_starts_at_initial_head_coordinates) {
    // Arrange
    Snake snake;

    // Act
    Position head = snake.getHead();

    // Assert
    EXPECT_EQ(head, Position(10, 10));
}

TEST(Rule4_InitialState, New_snake_starts_in_stopped_state) {
    // Arrange
    Snake snake;

    // Act
    Direction direction = snake.getDirection();

    // Assert
    EXPECT_EQ(direction, STOP);
}
