#include "raylib.h"
#include <cmath>

// Settings
const int screenWidth = 800;
const int screenHeight = 600;

const int B_COLUMNS = 10;
const int B_ROWS = 5;

struct Player {
  Vector2 position;
  Vector2 size;
  int life;
};

struct Ball {
  Vector2 position;
  Vector2 speed;
  int radius;
  bool active;
};

struct Brick {
  Vector2 position;
  Vector2 size;
  bool active;
  bool falling;
  Vector2 speed;
  Color color;
};

enum GameScreen { MENU, GAMEPLAY, ENDING };

// Global variables
Player player = {0};
Ball ball = {0};
Brick bricks[B_ROWS][B_COLUMNS] = {0};
GameScreen currentScreen = MENU;
int score = 0;
bool victory = false;
float speedMultiplier = 1.0f;

// Functions
void InitGame();
void UpdateGame();
void DrawGame();
void UpdateDrawFrame();

int main() {
  InitWindow(screenWidth, screenHeight, "Aronkanoid - C++ / Raylib");
  SetTargetFPS(60);

  InitGame();

  while (!WindowShouldClose()) {
    UpdateDrawFrame();
  }

  CloseWindow();
  return 0;
}

void InitGame() {
  // Init Player
  player.size = (Vector2){100, 20};
  player.position = (Vector2){screenWidth / 2 - player.size.x / 2,
                              screenHeight - player.size.y * 2};
  player.life = 3;

  // Init Ball
  ball.radius = 10;
  ball.position = (Vector2){player.position.x + player.size.x / 2,
                            player.position.y - ball.radius * 2};
  ball.speed = (Vector2){0, 0};
  ball.active = false;

  // Init Bricks
  int initialDownPosition = 50;
  int initialLeftPosition = 40;
  float brickWidth = (screenWidth - initialLeftPosition * 2.0f) / B_COLUMNS;
  float brickHeight = 30;

  for (int i = 0; i < B_ROWS; i++) {
    for (int j = 0; j < B_COLUMNS; j++) {
      bricks[i][j].position = (Vector2){initialLeftPosition + j * brickWidth,
                                        initialDownPosition + i * brickHeight};
      bricks[i][j].size =
          (Vector2){brickWidth - 2, brickHeight - 2}; // slight padding
      bricks[i][j].active = true;
      bricks[i][j].falling = false;
      bricks[i][j].speed = (Vector2){0, 0};

      // Alternate colors
      switch (i % 3) {
      case 0:
        bricks[i][j].color = RED;
        break;
      case 1:
        bricks[i][j].color = ORANGE;
        break;
      case 2:
        bricks[i][j].color = YELLOW;
        break;
      }
    }
  }
  score = 0;
  victory = false;
  speedMultiplier = 1.0f;
  currentScreen = MENU;
}

void UpdateGame() {
  if (currentScreen == MENU) {
    if (IsKeyPressed(KEY_ENTER))
      currentScreen = GAMEPLAY;
  } else if (currentScreen == GAMEPLAY) {
    // Player movement
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
      player.position.x -= 12.0f;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
      player.position.x += 12.0f;

    if (player.position.x <= 0)
      player.position.x = 0;
    if (player.position.x + player.size.x >= screenWidth)
      player.position.x = screenWidth - player.size.x;

    // Ball launch
    if (!ball.active) {
      ball.position = (Vector2){player.position.x + player.size.x / 2,
                                player.position.y - ball.radius * 2};
      if (IsKeyPressed(KEY_SPACE)) {
        ball.active = true;
        ball.speed = (Vector2){4.0f * speedMultiplier,
                               -5.0f * speedMultiplier}; // Initial velocity
      }
    } else {
      // Ball movement
      ball.position.x += ball.speed.x;
      ball.position.y += ball.speed.y;

      // Wall collision
      if (ball.position.x + ball.radius >= screenWidth ||
          ball.position.x - ball.radius <= 0) {
        ball.speed.x *= -1;
      }
      if (ball.position.y - ball.radius <= 0) {
        ball.speed.y *= -1;
      }

      // Floor collision (Death)
      if (ball.position.y + ball.radius >= screenHeight) {
        ball.active = false;
        player.life--;
        if (player.life <= 0) {
          currentScreen = ENDING;
          victory = false;
        }
      }

      // Player collision
      if (CheckCollisionCircleRec(ball.position, ball.radius,
                                  (Rectangle){player.position.x,
                                              player.position.y, player.size.x,
                                              player.size.y})) {
        if (ball.speed.y > 0) {
          ball.speed.y *= -1;
          // Add some spin/english based on where it hit the paddle
          float hitFactor =
              (ball.position.x - (player.position.x + player.size.x / 2)) /
              (player.size.x / 2);
          ball.speed.x = hitFactor * 6.0f * speedMultiplier;
        }
      }
    }

    // Falling Bricks Update
    for (int i = 0; i < B_ROWS; i++) {
      for (int j = 0; j < B_COLUMNS; j++) {
        if (bricks[i][j].falling) {
          bricks[i][j].position.x += bricks[i][j].speed.x;
          bricks[i][j].position.y += bricks[i][j].speed.y;
          bricks[i][j].speed.y += 0.15f; // Gravity

          Rectangle bRec = {bricks[i][j].position.x, bricks[i][j].position.y,
                            bricks[i][j].size.x, bricks[i][j].size.y};
          Rectangle pRec = {player.position.x, player.position.y, player.size.x,
                            player.size.y};

          // Player paddle collision
          if (bricks[i][j].speed.y > 0 && CheckCollisionRecs(bRec, pRec)) {
            bricks[i][j].speed.y = -6.0f; // Bounce up

            // Horizontal bounce based on paddle hit location
            float hitFactor =
                ((bricks[i][j].position.x + bricks[i][j].size.x / 2.0f) -
                 (player.position.x + player.size.x / 2.0f)) /
                (player.size.x / 2.0f);
            bricks[i][j].speed.x = hitFactor * 5.0f;

            bricks[i][j].position.y = player.position.y - bricks[i][j].size.y;
            bRec.y = bricks[i][j].position.y;
            bRec.x = bricks[i][j].position.x;
          }

          // Active bricks collision
          for (int m = 0; m < B_ROWS; m++) {
            for (int n = 0; n < B_COLUMNS; n++) {
              if (bricks[m][n].active) {
                Rectangle aRec = {bricks[m][n].position.x,
                                  bricks[m][n].position.y, bricks[m][n].size.x,
                                  bricks[m][n].size.y};
                if (CheckCollisionRecs(bRec, aRec)) {
                  // Resolve collision
                  float intersectY = std::fmin(
                      bricks[i][j].position.y + bricks[i][j].size.y - aRec.y,
                      aRec.y + aRec.height - bricks[i][j].position.y);
                  float intersectX = std::fmin(
                      bricks[i][j].position.x + bricks[i][j].size.x - aRec.x,
                      aRec.x + aRec.width - bricks[i][j].position.x);

                  if (intersectY < intersectX) {
                    if (bricks[i][j].speed.y > 0 &&
                        bricks[i][j].position.y < aRec.y) {
                      bricks[i][j].position.y = aRec.y - bricks[i][j].size.y;
                      bricks[i][j].speed.y = 0;
                      // Decelerate horizontal movement when resting
                      bricks[i][j].speed.x *= 0.5f;
                    } else if (bricks[i][j].speed.y < 0 &&
                               bricks[i][j].position.y > aRec.y) {
                      bricks[i][j].position.y = aRec.y + aRec.height;
                      bricks[i][j].speed.y *= -0.5f; // bounce off the bottom
                    }
                  } else {
                    if (bricks[i][j].position.x < aRec.x) {
                      bricks[i][j].position.x = aRec.x - bricks[i][j].size.x;
                      bricks[i][j].speed.x *= -0.5f;
                    } else {
                      bricks[i][j].position.x = aRec.x + aRec.width;
                      bricks[i][j].speed.x *= -0.5f;
                    }
                  }
                  bRec.x = bricks[i][j].position.x;
                  bRec.y = bricks[i][j].position.y;
                }
              }
            }
          }

          // Wall collision
          if (bricks[i][j].position.x <= 0) {
            bricks[i][j].position.x = 0;
            bricks[i][j].speed.x *= -1;
          } else if (bricks[i][j].position.x + bricks[i][j].size.x >=
                     screenWidth) {
            bricks[i][j].position.x = screenWidth - bricks[i][j].size.x;
            bricks[i][j].speed.x *= -1;
          }

          // Despawn
          if (bricks[i][j].position.y > screenHeight + 50) {
            bricks[i][j].falling = false;
          }
        }
      }
    }

    // Falling Bricks self-collision
    for (int i = 0; i < B_ROWS; i++) {
      for (int j = 0; j < B_COLUMNS; j++) {
        if (!bricks[i][j].falling)
          continue;
        Rectangle b1 = {bricks[i][j].position.x, bricks[i][j].position.y,
                        bricks[i][j].size.x, bricks[i][j].size.y};

        for (int m = i; m < B_ROWS; m++) {
          for (int n = (m == i ? j + 1 : 0); n < B_COLUMNS; n++) {
            if (!bricks[m][n].falling)
              continue;
            Rectangle b2 = {bricks[m][n].position.x, bricks[m][n].position.y,
                            bricks[m][n].size.x, bricks[m][n].size.y};

            if (CheckCollisionRecs(b1, b2)) {
              // Elastic velocity swap
              Vector2 temp = bricks[i][j].speed;
              bricks[i][j].speed = bricks[m][n].speed;
              bricks[m][n].speed = temp;

              // Push apart to prevent intersection clipping
              float intersectX =
                  std::fmin(bricks[i][j].position.x + bricks[i][j].size.x -
                                bricks[m][n].position.x,
                            bricks[m][n].position.x + bricks[m][n].size.x -
                                bricks[i][j].position.x);
              float intersectY =
                  std::fmin(bricks[i][j].position.y + bricks[i][j].size.y -
                                bricks[m][n].position.y,
                            bricks[m][n].position.y + bricks[m][n].size.y -
                                bricks[i][j].position.y);

              if (intersectX < intersectY) {
                if (bricks[i][j].position.x < bricks[m][n].position.x) {
                  bricks[i][j].position.x -= intersectX / 2.0f;
                  bricks[m][n].position.x += intersectX / 2.0f;
                } else {
                  bricks[i][j].position.x += intersectX / 2.0f;
                  bricks[m][n].position.x -= intersectX / 2.0f;
                }
              } else {
                if (bricks[i][j].position.y < bricks[m][n].position.y) {
                  bricks[i][j].position.y -= intersectY / 2.0f;
                  bricks[m][n].position.y += intersectY / 2.0f;
                } else {
                  bricks[i][j].position.y += intersectY / 2.0f;
                  bricks[m][n].position.y -= intersectY / 2.0f;
                }
              }
            }
          }
        }
      }
    }

    // Bricks collision with ball
    if (ball.active) {
      for (int i = 0; i < B_ROWS; i++) {
        for (int j = 0; j < B_COLUMNS; j++) {
          if (bricks[i][j].active || bricks[i][j].falling) {
            Rectangle brickRec = {bricks[i][j].position.x,
                                  bricks[i][j].position.y, bricks[i][j].size.x,
                                  bricks[i][j].size.y};

            if (CheckCollisionCircleRec(ball.position, ball.radius, brickRec)) {
              if (bricks[i][j].active) {
                bricks[i][j].active = false;
                bricks[i][j].falling = true;
                bricks[i][j].speed = (Vector2){0, 2.0f}; // initial break speed
                score += 10;
                speedMultiplier *= 1.02f;
                ball.speed.x *= 1.02f;
                ball.speed.y *= 1.02f;
              }

              // Basic reflection logic -> very simplistic
              float dTop = std::abs(brickRec.y - ball.position.y);
              float dBot =
                  std::abs((brickRec.y + brickRec.height) - ball.position.y);
              float dLeft = std::abs(brickRec.x - ball.position.x);
              float dRight =
                  std::abs((brickRec.x + brickRec.width) - ball.position.x);

              float minDist =
                  std::fmin(std::fmin(dTop, dBot), std::fmin(dLeft, dRight));

              if (minDist == dTop) {
                ball.speed.y = -std::abs(ball.speed.y);
                ball.position.y = brickRec.y - ball.radius;
              } else if (minDist == dBot) {
                ball.speed.y = std::abs(ball.speed.y);
                ball.position.y = brickRec.y + brickRec.height + ball.radius;
              } else if (minDist == dLeft) {
                ball.speed.x = -std::abs(ball.speed.x);
                ball.position.x = brickRec.x - ball.radius;
              } else {
                ball.speed.x = std::abs(ball.speed.x);
                ball.position.x = brickRec.x + brickRec.width + ball.radius;
              }

              // Let's break to only handle one collision per frame effectively
              goto CollisionHandled;
            }
          }
        }
      }
    CollisionHandled:;
    }

    bool winCheck = true;
    for (int i = 0; i < B_ROWS; i++) {
      for (int j = 0; j < B_COLUMNS; j++) {
        if (bricks[i][j].active) {
          winCheck = false;
          break;
        }
      }
      if (!winCheck)
        break;
    }

    if (winCheck) {
      currentScreen = ENDING;
      victory = true;
    }
  } else if (currentScreen == ENDING) {
    if (IsKeyPressed(KEY_ENTER)) {
      InitGame();
    }
  }
}

void DrawGame() {
  BeginDrawing();
  ClearBackground(BLACK);

  if (currentScreen == MENU) {
    DrawText("ARONKANOID", screenWidth / 2 - MeasureText("ARONKANOID", 40) / 2,
             screenHeight / 2 - 40, 40, BLACK);
    DrawText("PRESS ENTER to START",
             screenWidth / 2 - MeasureText("PRESS ENTER to START", 20) / 2,
             screenHeight / 2 + 20, 20, DARKGRAY);
  } else if (currentScreen == GAMEPLAY) {
    // Drop shadow
    DrawRectangle(player.position.x + 4, player.position.y + 4, player.size.x,
                  player.size.y, Fade(BLACK, 0.4f));
    // Main body
    DrawRectangleV(player.position, player.size, LIGHTGRAY);
    // 3D Bevels
    DrawRectangle(player.position.x, player.position.y, player.size.x, 4,
                  WHITE); // Top edge
    DrawRectangle(player.position.x, player.position.y, 4, player.size.y,
                  WHITE); // Left edge
    DrawRectangle(player.position.x, player.position.y + player.size.y - 4,
                  player.size.x, 4, GRAY); // Bottom edge
    DrawRectangle(player.position.x + player.size.x - 4, player.position.y, 4,
                  player.size.y, GRAY); // Right edge

    // Draw Ball
    DrawCircleV(ball.position, ball.radius, WHITE);

    // Draw Bricks
    for (int i = 0; i < B_ROWS; i++) {
      for (int j = 0; j < B_COLUMNS; j++) {
        if (bricks[i][j].active || bricks[i][j].falling) {
          Rectangle bRec = {bricks[i][j].position.x, bricks[i][j].position.y,
                            bricks[i][j].size.x, bricks[i][j].size.y};
          // Drop shadow
          DrawRectangle(bRec.x + 3, bRec.y + 3, bRec.width, bRec.height,
                        Fade(BLACK, 0.4f));
          Color baseColor = bricks[i][j].color;
          if (bricks[i][j].falling) {
            // Darken color to look "turned off"
            baseColor.r = (unsigned char)(baseColor.r * 0.4f);
            baseColor.g = (unsigned char)(baseColor.g * 0.4f);
            baseColor.b = (unsigned char)(baseColor.b * 0.4f);
          }

          // Body
          DrawRectangleRec(bRec, baseColor);

          // 3D Bevel Effects
          if (bricks[i][j].active) {
            // Illuminated / Bright edges
            DrawRectangle(bRec.x, bRec.y, bRec.width, 3,
                          Fade(WHITE, 0.6f)); // Top
            DrawRectangle(bRec.x, bRec.y, 3, bRec.height,
                          Fade(WHITE, 0.6f)); // Left
            DrawRectangle(bRec.x, bRec.y + bRec.height - 3, bRec.width, 3,
                          Fade(BLACK, 0.4f)); // Bottom
            DrawRectangle(bRec.x + bRec.width - 3, bRec.y, 3, bRec.height,
                          Fade(BLACK, 0.4f)); // Right
          } else {
            // Turned off edges
            DrawRectangle(bRec.x, bRec.y, bRec.width, 3,
                          Fade(BLACK, 0.3f)); // Top
            DrawRectangle(bRec.x, bRec.y, 3, bRec.height,
                          Fade(BLACK, 0.3f)); // Left
            DrawRectangle(bRec.x, bRec.y + bRec.height - 3, bRec.width, 3,
                          Fade(BLACK, 0.7f)); // Bottom
            DrawRectangle(bRec.x + bRec.width - 3, bRec.y, 3, bRec.height,
                          Fade(BLACK, 0.7f)); // Right
          }
        }
      }
    }

    // UI
    DrawText(TextFormat("SCORE: %04i", score), 20, 10, 20, WHITE);
    DrawText(TextFormat("LIVES: %i", player.life), screenWidth - 100, 10, 20,
             BLACK);

    if (!ball.active) {
      DrawText("PRESS SPACE to LAUNCH",
               screenWidth / 2 - MeasureText("PRESS SPACE to LAUNCH", 20) / 2,
               screenHeight / 2, 20, DARKGRAY);
    }

  } else if (currentScreen == ENDING) {
    if (victory) {
      DrawText("YOU WIN!", screenWidth / 2 - MeasureText("YOU WIN!", 40) / 2,
               screenHeight / 2 - 40, 40, GREEN);
    } else {
      DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 40) / 2,
               screenHeight / 2 - 40, 40, RED);
    }
    DrawText(TextFormat("FINAL SCORE: %i", score),
             screenWidth / 2 -
                 MeasureText(TextFormat("FINAL SCORE: %i", score), 20) / 2,
             screenHeight / 2 + 10, 20, BLACK);
    DrawText("PRESS ENTER to RESTART",
             screenWidth / 2 - MeasureText("PRESS ENTER to RESTART", 20) / 2,
             screenHeight / 2 + 40, 20, DARKGRAY);
  }

  EndDrawing();
}

void UpdateDrawFrame() {
  UpdateGame();
  DrawGame();
}
