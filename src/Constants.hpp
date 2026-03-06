#pragma once
#include "raylib.h"

const int screenWidth = 800;
const int screenHeight = 600;

const int B_COLUMNS = 10;
const int B_ROWS = 5;
const int MAX_BALLS = 5;
const int MAX_BULLETS = 10;

enum ImprovementType {
  NONE,
  BIGGER_BAR,
  STICKY_BAR,
  Y_FREE,
  BULLETS,
  EXTRA_LIFE,
  EXTRA_BALL,
  AI_BAR,
  FIREBALL,
  SLOW_DOWN,
  MULTI_BALL,
  LASER,
  SHIELD,
  SHRINK_BAR
};

enum GameScreen { MENU, GAMEPLAY, ENDING };
