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
  bool isSticky;
  bool isYFree;
  bool canShoot;
  bool hasLaser;
  bool hasShield;
};

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

struct Ball {
  Vector2 position;
  Vector2 speed;
  int radius;
  bool active;
  bool readyToLaunch;
  bool isFireball;
};

struct Brick {
  Vector2 position;
  Vector2 size;
  bool active;
  bool falling;
  bool isTrap;
  Vector2 speed;
  Color color;
  ImprovementType improvement;
  int hp;
  int maxHp;
};

enum GameScreen { MENU, GAMEPLAY, ENDING };

const int MAX_BULLETS = 10;
struct Bullet {
  Vector2 position;
  bool active;
  Vector2 speed;
};
Bullet bullets[MAX_BULLETS] = {0};

void ResetPlayerPowerups();

// Global variables
const int MAX_BALLS = 5;
Player player = {0};

struct AIPlayer {
  Vector2 position;
  Vector2 size;
  bool active;
  float speedX;
};
AIPlayer aiBar = {0};

Ball balls[MAX_BALLS] = {0};
Brick bricks[B_ROWS][B_COLUMNS] = {0};
GameScreen currentScreen = MENU;
int score = 0;
bool victory = false;
float speedMultiplier = 1.0f;
int explosionTimer = 0;
Vector2 explosionPos = {0, 0};
float explosionRadius = 0;

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

void ResetPlayerPowerups() {
  player.size.x = 100;
  player.isSticky = false;
  player.isYFree = false;
  player.canShoot = false;
  player.hasLaser = false;
  player.hasShield = false;
  aiBar.active = false;
}

void InitGame() {
  // Init Player
  player.size = (Vector2){100, 20};
  ResetPlayerPowerups();
  for (int i = 0; i < MAX_BULLETS; i++)
    bullets[i].active = false;
  player.position = (Vector2){screenWidth / 2 - player.size.x / 2,
                              screenHeight - player.size.y * 2};
  player.life = 3;

  // Init Ball
  for (int i = 0; i < MAX_BALLS; i++) {
    balls[i].radius = 10;
    balls[i].position = (Vector2){player.position.x + player.size.x / 2,
                                  player.position.y - balls[i].radius * 2};
    balls[i].speed = (Vector2){0, 0};
    balls[i].active = false;
    balls[i].readyToLaunch = (i == 0);
    balls[i].isFireball = false;
  }

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
      bricks[i][j].isTrap = false;
      bricks[i][j].speed = (Vector2){0, 0};
      bricks[i][j].improvement = NONE;

      if (i == 0) {
        bricks[i][j].color = GRAY;
        bricks[i][j].hp = 2;
        bricks[i][j].maxHp = 2;
      } else {
        bricks[i][j].hp = 1;
        bricks[i][j].maxHp = 1;
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
  }

  // Set one random block as the trap. No need to seed, Raylib pre-seeds random.
  int trapR = GetRandomValue(0, B_ROWS - 1);
  int trapC = GetRandomValue(0, B_COLUMNS - 1);
  bricks[trapR][trapC].isTrap = true;

  aiBar.size = (Vector2){80, 20};
  aiBar.speedX = 6.0f;

  for (int i = 0; i < B_ROWS; i++) {
    for (int j = 0; j < B_COLUMNS; j++) {
      if (!bricks[i][j].isTrap && GetRandomValue(1, 100) <= 25) {
        bricks[i][j].improvement = (ImprovementType)GetRandomValue(1, 13);
      }
    }
  }

  score = 0;
  victory = false;
  speedMultiplier = 1.0f;
  explosionTimer = 0;
  currentScreen = MENU;
}

void UpdateGame() {
  if (currentScreen == MENU) {
    if (IsKeyPressed(KEY_ENTER)) {
      currentScreen = GAMEPLAY;
      HideCursor();
    }
  } else if (currentScreen == GAMEPLAY) {
    if (explosionTimer > 0)
      explosionTimer--;

    // Player movement
    Vector2 mouseDelta = GetMouseDelta();
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
      player.position.x = GetMouseX() - player.size.x / 2.0f;
      if (player.isYFree) {
        player.position.y = GetMouseY() - player.size.y / 2.0f;
      }
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
      player.position.x -= 12.0f;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
      player.position.x += 12.0f;

    if (player.isYFree) {
      if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        player.position.y -= 12.0f;
      if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        player.position.y += 12.0f;
    }

    if (player.position.x <= 0)
      player.position.x = 0;
    if (player.position.x + player.size.x >= screenWidth)
      player.position.x = screenWidth - player.size.x;

    if (player.isYFree) {
      if (player.position.y <= 0)
        player.position.y = 0;
      if (player.position.y + player.size.y >= screenHeight)
        player.position.y = screenHeight - player.size.y;
    } else {
      player.position.y = screenHeight - player.size.y * 2;
    }

    if (aiBar.active) {
      aiBar.position.y = player.position.y - 40; // A bit upper in Y
      // Find lowest ball heading downwards
      float lowestY = -1;
      float targetX = aiBar.position.x + aiBar.size.x / 2.0f;
      bool found = false;
      for (int k = 0; k < MAX_BALLS; k++) {
        if (balls[k].active && balls[k].speed.y > 0) {
          if (balls[k].position.y > lowestY) {
            lowestY = balls[k].position.y;
            targetX = balls[k].position.x;
            found = true;
          }
        }
      }

      if (found) {
        float aiCenterX = aiBar.position.x + aiBar.size.x / 2.0f;
        if (aiCenterX < targetX - 5.0f) {
          aiBar.position.x += aiBar.speedX;
        } else if (aiCenterX > targetX + 5.0f) {
          aiBar.position.x -= aiBar.speedX;
        }
      }

      if (aiBar.position.x <= 0)
        aiBar.position.x = 0;
      if (aiBar.position.x + aiBar.size.x >= screenWidth)
        aiBar.position.x = screenWidth - aiBar.size.x;
    }

    // Ball launch
    bool shootActionPressed =
        IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    for (int k = 0; k < MAX_BALLS; k++) {
      if (balls[k].readyToLaunch) {
        balls[k].position = (Vector2){player.position.x + player.size.x / 2,
                                      player.position.y - balls[k].radius * 2};
        if (shootActionPressed) {
          balls[k].active = true;
          balls[k].readyToLaunch = false;
          balls[k].speed =
              (Vector2){4.0f * speedMultiplier,
                        -5.0f * speedMultiplier}; // Initial velocity
        }
      }
    }

    if (player.canShoot && shootActionPressed) {
      for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
          bullets[i].active = true;
          bullets[i].position = (Vector2){
              player.position.x + player.size.x / 2.0f, player.position.y};
          bullets[i].speed = (Vector2){0, -8.0f};
          break;
        }
      }
    }

    // Bullets update
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (bullets[i].active) {
        bullets[i].position.y += bullets[i].speed.y;
        if (bullets[i].position.y < 0)
          bullets[i].active = false;

        for (int r = 0; r < B_ROWS; r++) {
          for (int c = 0; c < B_COLUMNS; c++) {
            if (bricks[r][c].active &&
                CheckCollisionCircleRec(bullets[i].position, 3,
                                        (Rectangle){bricks[r][c].position.x,
                                                    bricks[r][c].position.y,
                                                    bricks[r][c].size.x,
                                                    bricks[r][c].size.y})) {
              bullets[i].active = false;
              bricks[r][c].hp--;
              if (bricks[r][c].hp <= 0) {
                bricks[r][c].active = false;
                bricks[r][c].falling = true;
                bricks[r][c].speed = (Vector2){0, 2.0f};
                score += 10;
              } else {
                score += 5;
              }
            }
          }
        }
      }
    }

    // Ball movement
    for (int k = 0; k < MAX_BALLS; k++) {
      if (!balls[k].active)
        continue;

      balls[k].position.x += balls[k].speed.x;
      balls[k].position.y += balls[k].speed.y;

      // Wall collision
      if (balls[k].position.x + balls[k].radius >= screenWidth) {
        balls[k].position.x = screenWidth - balls[k].radius;
        balls[k].speed.x = -std::abs(balls[k].speed.x);
      } else if (balls[k].position.x - balls[k].radius <= 0) {
        balls[k].position.x = balls[k].radius;
        balls[k].speed.x = std::abs(balls[k].speed.x);
      }

      if (balls[k].position.y - balls[k].radius <= 0) {
        balls[k].position.y = balls[k].radius;
        balls[k].speed.y = std::abs(balls[k].speed.y);
      }

      // Floor collision (Death)
      if (balls[k].position.y + balls[k].radius >= screenHeight) {
        if (player.hasShield && (balls[k].active || balls[k].readyToLaunch)) {
          balls[k].speed.y = -std::abs(balls[k].speed.y);
          balls[k].position.y = screenHeight - balls[k].radius - 5.0f;
          player.hasShield = false;
        } else {
          balls[k].active = false;

          bool anyAlive = false;
          for (int a = 0; a < MAX_BALLS; a++) {
            if (balls[a].active || balls[a].readyToLaunch)
              anyAlive = true;
          }

          if (!anyAlive) {
            player.life--;
            ResetPlayerPowerups();
            for (int a = 0; a < MAX_BALLS; a++) {
              balls[a].active = false;
              balls[a].readyToLaunch = false;
              balls[a].isFireball = false;
            }
            if (player.life <= 0) {
              currentScreen = ENDING;
              victory = false;
              ShowCursor();
            } else {
              balls[0].readyToLaunch = true;
            }
          }
        }
      }

      // Player collision
      if (balls[k].active &&
          CheckCollisionCircleRec(balls[k].position, balls[k].radius,
                                  (Rectangle){player.position.x,
                                              player.position.y, player.size.x,
                                              player.size.y})) {
        if (balls[k].speed.y > 0) {
          if (player.isSticky) {
            balls[k].active = false;
            balls[k].readyToLaunch = true;
          } else {
            balls[k].speed.y *= -1;
            // Add some spin/english based on where it hit the paddle
            float hitFactor = (balls[k].position.x -
                               (player.position.x + player.size.x / 2)) /
                              (player.size.x / 2);
            balls[k].speed.x = hitFactor * 6.0f * speedMultiplier;
          }
        }
      }

      // AI Bar collision
      if (aiBar.active && balls[k].active &&
          CheckCollisionCircleRec(balls[k].position, balls[k].radius,
                                  (Rectangle){aiBar.position.x,
                                              aiBar.position.y, aiBar.size.x,
                                              aiBar.size.y})) {
        if (balls[k].speed.y > 0) {
          balls[k].speed.y *= -1;
          float hitFactor =
              (balls[k].position.x - (aiBar.position.x + aiBar.size.x / 2.0f)) /
              (aiBar.size.x / 2.0f);
          balls[k].speed.x = hitFactor * 6.0f * speedMultiplier;
          balls[k].position.y = aiBar.position.y - balls[k].radius;
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

          // Trap Explosion logic
          if (bricks[i][j].isTrap &&
              bricks[i][j].position.y + bricks[i][j].size.y >=
                  player.position.y) {
            bricks[i][j].falling = false;

            explosionPos =
                (Vector2){bricks[i][j].position.x + bricks[i][j].size.x / 2.0f,
                          player.position.y};
            explosionRadius = screenWidth / 4.0f; // half screen total area
            explosionTimer = 30;                  // 0.5 seconds at 60 FPS

            float playerCenterX = player.position.x + player.size.x / 2.0f;
            if (std::abs(playerCenterX - explosionPos.x) <=
                explosionRadius + player.size.x / 2.0f) {
              player.life--;
              ResetPlayerPowerups();
              for (int a = 0; a < MAX_BALLS; a++) {
                balls[a].active = false;
                balls[a].readyToLaunch = false;
              }
              if (player.life <= 0) {
                currentScreen = ENDING;
                victory = false;
                ShowCursor();
              } else {
                balls[0].readyToLaunch = true;
              }
            }
            continue;
          }

          // Player paddle collision
          if (!bricks[i][j].isTrap && bricks[i][j].speed.y > 0 &&
              CheckCollisionRecs(bRec, pRec)) {
            if (bricks[i][j].improvement != NONE) {
              bricks[i][j].falling = false;
              if (bricks[i][j].improvement == BIGGER_BAR) {
                player.size.x += 50;
                if (player.size.x > screenWidth - 100)
                  player.size.x = screenWidth - 100;
              } else if (bricks[i][j].improvement == STICKY_BAR)
                player.isSticky = true;
              else if (bricks[i][j].improvement == Y_FREE)
                player.isYFree = true;
              else if (bricks[i][j].improvement == BULLETS)
                player.canShoot = true;
              else if (bricks[i][j].improvement == EXTRA_LIFE)
                player.life++;
              else if (bricks[i][j].improvement == EXTRA_BALL) {
                for (int b = 0; b < MAX_BALLS; b++) {
                  if (!balls[b].active && !balls[b].readyToLaunch) {
                    balls[b].active = true;
                    Vector2 basePos =
                        (Vector2){player.position.x + player.size.x / 2.0f,
                                  player.position.y - 20};
                    for (int a = 0; a < MAX_BALLS; a++) {
                      if (balls[a].active && a != b) {
                        basePos = balls[a].position;
                        break;
                      }
                    }
                    balls[b].position = basePos;
                    balls[b].speed = (Vector2){((b % 2) == 0 ? 1.0f : -1.0f) *
                                                   4.0f * speedMultiplier,
                                               -5.0f * speedMultiplier};
                    break;
                  }
                }
              } else if (bricks[i][j].improvement == AI_BAR) {
                aiBar.active = true;
                aiBar.position.x = player.position.x + player.size.x / 2.0f -
                                   aiBar.size.x / 2.0f;
                aiBar.position.y = player.position.y - 40;
              } else if (bricks[i][j].improvement == FIREBALL) {
                for (int b = 0; b < MAX_BALLS; b++)
                  if (balls[b].active || balls[b].readyToLaunch)
                    balls[b].isFireball = true;
              } else if (bricks[i][j].improvement == SLOW_DOWN) {
                speedMultiplier = std::fmax(1.0f, speedMultiplier * 0.7f);
                for (int b = 0; b < MAX_BALLS; b++) {
                  if (balls[b].active) {
                    balls[b].speed.x *= 0.7f;
                    balls[b].speed.y *= 0.7f;
                  }
                }
              } else if (bricks[i][j].improvement == MULTI_BALL) {
                int spawned = 0;
                for (int b = 0; b < MAX_BALLS && spawned < 2; b++) {
                  if (!balls[b].active && !balls[b].readyToLaunch) {
                    balls[b].active = true;

                    Vector2 basePos =
                        (Vector2){player.position.x + player.size.x / 2.0f,
                                  player.position.y - 20};
                    for (int a = 0; a < MAX_BALLS; a++) {
                      if (balls[a].active && a != b) {
                        basePos = balls[a].position;
                        break;
                      }
                    }

                    balls[b].position = basePos;
                    balls[b].speed = (Vector2){((b % 2) == 0 ? 1.5f : -1.5f) *
                                                   4.0f * speedMultiplier,
                                               -4.5f * speedMultiplier};
                    balls[b].isFireball = false;
                    spawned++;
                  }
                }
              } else if (bricks[i][j].improvement == LASER)
                player.hasLaser = true;
              else if (bricks[i][j].improvement == SHIELD)
                player.hasShield = true;
              else if (bricks[i][j].improvement == SHRINK_BAR) {
                player.size.x -= 30;
                if (player.size.x < 40)
                  player.size.x = 40;
              }
            } else {
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
    for (int k = 0; k < MAX_BALLS; k++) {
      if (!balls[k].active)
        continue;
      for (int i = 0; i < B_ROWS; i++) {
        for (int j = 0; j < B_COLUMNS; j++) {
          if (bricks[i][j].active || bricks[i][j].falling) {
            Rectangle brickRec = {bricks[i][j].position.x,
                                  bricks[i][j].position.y, bricks[i][j].size.x,
                                  bricks[i][j].size.y};

            if (CheckCollisionCircleRec(balls[k].position, balls[k].radius,
                                        brickRec)) {
              if (bricks[i][j].active) {
                bricks[i][j].hp--;
                if (bricks[i][j].hp <= 0) {
                  bricks[i][j].active = false;
                  bricks[i][j].falling = true;
                  bricks[i][j].speed =
                      (Vector2){0, 2.0f}; // initial break speed
                  score += 10;
                  speedMultiplier *= 1.02f;
                  balls[k].speed.x *= 1.02f;
                  balls[k].speed.y *= 1.02f;
                } else {
                  score += 5;
                }
              }

              // Basic reflection logic -> very simplistic
              if (!balls[k].isFireball) {
                float dTop = std::abs(brickRec.y - balls[k].position.y);
                float dBot = std::abs((brickRec.y + brickRec.height) -
                                      balls[k].position.y);
                float dLeft = std::abs(brickRec.x - balls[k].position.x);
                float dRight = std::abs((brickRec.x + brickRec.width) -
                                        balls[k].position.x);

                float minDist =
                    std::fmin(std::fmin(dTop, dBot), std::fmin(dLeft, dRight));

                if (minDist == dTop) {
                  balls[k].speed.y = -std::abs(balls[k].speed.y);
                  balls[k].position.y = brickRec.y - balls[k].radius - 0.1f;
                } else if (minDist == dBot) {
                  balls[k].speed.y = std::abs(balls[k].speed.y);
                  balls[k].position.y =
                      brickRec.y + brickRec.height + balls[k].radius + 0.1f;
                } else if (minDist == dLeft) {
                  balls[k].speed.x = -std::abs(balls[k].speed.x);
                  balls[k].position.x = brickRec.x - balls[k].radius - 0.1f;
                } else {
                  balls[k].speed.x = std::abs(balls[k].speed.x);
                  balls[k].position.x =
                      brickRec.x + brickRec.width + balls[k].radius + 0.1f;
                }

                // Anti-stuck: If perfectly horizontal bounce and trapped near
                // top, add slight vertical
                if (std::abs(balls[k].speed.y) < 0.5f) {
                  balls[k].speed.y = (balls[k].speed.y >= 0 ? 1.0f : -1.0f);
                }
              }

              goto BallCollisionHandled;
            }
          }
        }
      }
    BallCollisionHandled:;
    }

    bool winCheck = true;
    for (int i = 0; i < B_ROWS; i++) {
      for (int j = 0; j < B_COLUMNS; j++) {
        if (bricks[i][j].active || bricks[i][j].falling) {
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
      ShowCursor();
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

    // Draw Cannons
    if (player.canShoot) {
      // Left Cannon
      DrawRectangle(player.position.x + 10, player.position.y - 12, 12, 12,
                    GRAY);
      DrawRectangle(player.position.x + 10, player.position.y - 12, 4, 12,
                    LIGHTGRAY);
      DrawRectangle(player.position.x + 20, player.position.y - 12, 2, 12,
                    DARKGRAY);

      // Right Cannon
      DrawRectangle(player.position.x + player.size.x - 22,
                    player.position.y - 12, 12, 12, GRAY);
      DrawRectangle(player.position.x + player.size.x - 22,
                    player.position.y - 12, 4, 12, LIGHTGRAY);
      DrawRectangle(player.position.x + player.size.x - 12,
                    player.position.y - 12, 2, 12, DARKGRAY);
    }

    // Draw AI Bar
    if (aiBar.active) {
      DrawRectangle(aiBar.position.x + 4, aiBar.position.y + 4, aiBar.size.x,
                    aiBar.size.y, Fade(BLACK, 0.4f));
      DrawRectangleV(aiBar.position, aiBar.size, ORANGE);
      DrawRectangle(aiBar.position.x, aiBar.position.y, aiBar.size.x, 4,
                    Fade(WHITE, 0.5f));
      DrawRectangle(aiBar.position.x, aiBar.position.y, 4, aiBar.size.y,
                    Fade(WHITE, 0.5f));
      DrawRectangle(aiBar.position.x, aiBar.position.y + aiBar.size.y - 4,
                    aiBar.size.x, 4, Fade(BLACK, 0.3f));
      DrawRectangle(aiBar.position.x + aiBar.size.x - 4, aiBar.position.y, 4,
                    aiBar.size.y, Fade(BLACK, 0.3f));
      DrawText("AI",
               aiBar.position.x + aiBar.size.x / 2 - MeasureText("AI", 10) / 2,
               aiBar.position.y + 5, 10, WHITE);
    }

    // Draw Bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (bullets[i].active) {
        DrawCircleV(bullets[i].position, 3, YELLOW);
      }
    }

    // Draw Shield
    if (player.hasShield) {
      for (int i = 0; i < screenWidth; i += 40) {
        DrawRectangle(i, screenHeight - 15, 38, 15, SKYBLUE);
        DrawRectangle(i, screenHeight - 15, 38, 3, Fade(WHITE, 0.7f));
        DrawRectangle(i, screenHeight - 15, 3, 15, Fade(WHITE, 0.7f));
        DrawRectangle(i, screenHeight - 3, 38, 3, Fade(BLACK, 0.3f));
        DrawRectangle(i + 35, screenHeight - 15, 3, 15, Fade(BLACK, 0.3f));
      }
    }

    // Draw Laser Trajectory
    if (player.hasLaser) {
      for (int k = 0; k < MAX_BALLS; k++) {
        if (balls[k].active || balls[k].readyToLaunch) {
          Vector2 simulatePos = balls[k].position;
          Vector2 simulateSpeed =
              (balls[k].speed.x == 0 && balls[k].speed.y == 0)
                  ? (Vector2){4.0f * speedMultiplier, -5.0f * speedMultiplier}
                  : balls[k].speed;

          for (int t = 0; t < 20; t++) {
            simulatePos.x += simulateSpeed.x * 2.0f;
            simulatePos.y += simulateSpeed.y * 2.0f;
            if (t % 2 == 0)
              DrawCircleV(simulatePos, 2, ColorAlpha(RED, 0.5f));
          }
        }
      }
    }

    // Draw Balls
    for (int k = 0; k < MAX_BALLS; k++) {
      if (balls[k].active || balls[k].readyToLaunch) {
        DrawCircleV(balls[k].position, balls[k].radius,
                    balls[k].isFireball ? RED : WHITE);
        if (balls[k].isFireball)
          DrawCircleV(balls[k].position, balls[k].radius * 0.6f, YELLOW);
      }
    }

    // Draw Explosion
    if (explosionTimer > 0) {
      float alpha = (float)explosionTimer / 30.0f;
      DrawCircleV(explosionPos, explosionRadius, Fade(ORANGE, alpha * 0.5f));
      DrawCircleV(explosionPos, explosionRadius * 0.75f,
                  Fade(RED, alpha * 0.8f));
      DrawCircleV(explosionPos, explosionRadius * 0.4f, Fade(YELLOW, alpha));
    }

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
            if (bricks[i][j].isTrap) {
              baseColor = BLACK;
            } else {
              // Darken color to look "turned off"
              baseColor.r = (unsigned char)(baseColor.r * 0.4f);
              baseColor.g = (unsigned char)(baseColor.g * 0.4f);
              baseColor.b = (unsigned char)(baseColor.b * 0.4f);
            }
          }

          // Body
          DrawRectangleRec(bRec, baseColor);

          if (bricks[i][j].maxHp > 1 && bricks[i][j].hp < bricks[i][j].maxHp) {
            DrawLineEx(
                (Vector2){bRec.x + 5, bRec.y + 5},
                (Vector2){bRec.x + bRec.width / 2.0f, bRec.y + bRec.height - 5},
                2.0f, BLACK);
            DrawLineEx(
                (Vector2){bRec.x + bRec.width / 2.0f, bRec.y + bRec.height - 5},
                (Vector2){bRec.x + bRec.width - 10, bRec.y + 10}, 2.0f, BLACK);
          }

          // Render Skull if it's the trap
          if (bricks[i][j].falling && bricks[i][j].isTrap) {
            float cx = bRec.x + bRec.width / 2.0f;
            float cy = bRec.y + bRec.height / 2.0f;
            DrawCircle(cx, cy - 2, 6, WHITE);           // top head
            DrawRectangle(cx - 4, cy + 1, 8, 4, WHITE); // jaw
            DrawCircle(cx - 2, cy - 1, 1.5f, BLACK);    // left eye
            DrawCircle(cx + 2, cy - 1, 1.5f, BLACK);    // right eye
            DrawRectangle(cx - 1, cy + 3, 2, 2, BLACK); // nose
          } else if (bricks[i][j].falling && bricks[i][j].improvement != NONE) {
            float cx = bRec.x + bRec.width / 2.0f;
            float cy = bRec.y + bRec.height / 2.0f;
            if (bricks[i][j].improvement == BIGGER_BAR)
              DrawText("< >", cx - MeasureText("< >", 10) / 2, cy - 5, 10,
                       WHITE);
            else if (bricks[i][j].improvement == STICKY_BAR)
              DrawText("S", cx - MeasureText("S", 10) / 2, cy - 5, 10, GREEN);
            else if (bricks[i][j].improvement == Y_FREE)
              DrawText("Y", cx - MeasureText("Y", 10) / 2, cy - 5, 10, BLUE);
            else if (bricks[i][j].improvement == BULLETS)
              DrawText("B", cx - MeasureText("B", 10) / 2, cy - 5, 10, RED);
            else if (bricks[i][j].improvement == EXTRA_LIFE)
              DrawText("1UP", cx - MeasureText("1UP", 10) / 2, cy - 5, 10,
                       YELLOW);
            else if (bricks[i][j].improvement == EXTRA_BALL)
              DrawText("+O", cx - MeasureText("+O", 10) / 2, cy - 5, 10,
                       SKYBLUE);
            else if (bricks[i][j].improvement == AI_BAR)
              DrawText("AI", cx - MeasureText("AI", 10) / 2, cy - 5, 10,
                       PURPLE);
            else if (bricks[i][j].improvement == FIREBALL)
              DrawText("FIRE", cx - MeasureText("FIRE", 10) / 2, cy - 5, 10,
                       RED);
            else if (bricks[i][j].improvement == SLOW_DOWN)
              DrawText("SLOW", cx - MeasureText("SLOW", 10) / 2, cy - 5, 10,
                       DARKBLUE);
            else if (bricks[i][j].improvement == MULTI_BALL)
              DrawText("X3", cx - MeasureText("X3", 10) / 2, cy - 5, 10, LIME);
            else if (bricks[i][j].improvement == LASER)
              DrawText("AIM", cx - MeasureText("AIM", 10) / 2, cy - 5, 10,
                       MAGENTA);
            else if (bricks[i][j].improvement == SHIELD)
              DrawText("SHLD", cx - MeasureText("SHLD", 10) / 2, cy - 5, 10,
                       SKYBLUE);
            else if (bricks[i][j].improvement == SHRINK_BAR)
              DrawText("><", cx - MeasureText("><", 10) / 2, cy - 5, 10,
                       MAROON);
          }

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
             WHITE);

    bool anyReady = false;
    for (int k = 0; k < MAX_BALLS; k++)
      if (balls[k].readyToLaunch)
        anyReady = true;
    if (anyReady) {
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
