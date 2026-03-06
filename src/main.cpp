#include "Game.hpp"

int main() {
  InitWindow(screenWidth, screenHeight, "Aronkanoid - C++ / Raylib");
  SetTargetFPS(60);

  Game game;
  game.Init();

  while (!WindowShouldClose()) {
    game.UpdateDrawFrame();
  }

  CloseWindow();
  return 0;
}
