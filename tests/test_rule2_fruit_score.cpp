#include <gtest/gtest.h>
#include "snake_under_test.h"

// Test Double (Stub) passed through the IRandom object seam
class StubRandom : public IRandom {
private:
    int cannedValue;
public:
    explicit StubRandom(int value) : cannedValue(value) {}
    int nextInt(int max) override {
        return cannedValue % max;
    }
};

// Rule 2: Eating fruit increases the score by exactly 10 points unless it's a special fruit which increases the score by 50 points.
TEST(Rule2_FruitScore, Special_fruit_grants_fifty_points_when_rolled) {
    // Arrange: Stub returns 0, so nextInt(10) == 0 (special fruit condition)
    Food food;
    std::vector<Position> emptySnakeBody;
    StubRandom specialFruitStub(0);

    // Act
    food.generateFood(30, 20, emptySnakeBody, &specialFruitStub);

    // Assert
    EXPECT_EQ(food.getValue(), 50);
}

TEST(Rule2_FruitScore, Special_fruit_displays_dollar_symbol_when_rolled) {
    // Arrange: Stub returns 0, so nextInt(10) == 0 (special fruit condition)
    Food food;
    std::vector<Position> emptySnakeBody;
    StubRandom specialFruitStub(0);

    // Act
    food.generateFood(30, 20, emptySnakeBody, &specialFruitStub);

    // Assert
    EXPECT_EQ(food.getSymbol(), '$');
}

TEST(Rule2_FruitScore, Normal_fruit_grants_ten_points_when_rolled) {
    // Arrange: Stub returns 1, so nextInt(10) != 0 (normal fruit condition)
    Food food;
    std::vector<Position> emptySnakeBody;
    StubRandom normalFruitStub(1);

    // Act
    food.generateFood(30, 20, emptySnakeBody, &normalFruitStub);

    // Assert
    EXPECT_EQ(food.getValue(), 10);
}

TEST(Rule2_FruitScore, Normal_fruit_displays_asterisk_symbol_when_rolled) {
    // Arrange: Stub returns 1, so nextInt(10) != 0 (normal fruit condition)
    Food food;
    std::vector<Position> emptySnakeBody;
    StubRandom normalFruitStub(1);

    // Act
    food.generateFood(30, 20, emptySnakeBody, &normalFruitStub);

    // Assert
    EXPECT_EQ(food.getSymbol(), '*');
}
