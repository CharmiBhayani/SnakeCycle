#pragma once

#ifndef SNAKE_UNDER_TEST_H
#define SNAKE_UNDER_TEST_H

// Make production main inline and renamed to prevent multiple definition errors across test files
#define main inline snake_cycle_production_main
#include "../snakeCycle.cpp"
#undef main

#endif // SNAKE_UNDER_TEST_H
