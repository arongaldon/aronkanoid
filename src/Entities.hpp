#pragma once
#include "Constants.hpp"

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

struct Bullet {
  Vector2 position;
  bool active;
  Vector2 speed;
};

struct AIPlayer {
  Vector2 position;
  Vector2 size;
  bool active;
  float speedX;
};

struct Star {
  Vector2 position;
  float speedY;
  float radius;
  Color color;
};
