#pragma once
#include "Entities.hpp"

class Game {
public:
  Player player;
  AIPlayer aiBar;
  Ball balls[MAX_BALLS];
  Brick bricks[B_ROWS][B_COLUMNS];
  Bullet bullets[MAX_BULLETS];
  Star stars[MAX_STARS];

  GameScreen currentScreen;
  int score;
  bool victory;
  float speedMultiplier;
  int explosionTimer;
  Vector2 explosionPos;
  float explosionRadius;

  Game();

  void Init();
  void Update();
  void UpdateStars();
  void Draw();
  void UpdateDrawFrame();

private:
  void ResetPlayerPowerups();
};
