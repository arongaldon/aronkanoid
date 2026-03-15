# Aronkanoid

Aronkanoid, a classic game reimagined by Aron Galdon. Built from scratch in C++ using the [Raylib](https://www.raylib.com/) library.

## Dependencies

This game uses CMake and FetchContent to automatically download Raylib during the build process.

If you are on Linux, you will need some development libraries installed first. A `build.sh` script is provided to handle this automatically on Debian/Ubuntu-based systems.

## Building and Running

1. **Install dependencies and build the project:**
   Run the provided build script. It will ask for `sudo` to install the required Linux packages (like X11 and OpenGL development headers), configure CMake, and build the project.

   ```bash
   ./build.sh
   ```

2. **Run the game:**
   Once built, you can run the executable:

   ```bash
   ./build/Aronkanoid
   ```

## Controls

* **Move Paddle:** `Left/Right Arrows`, `A / D` keys, or simply move your **Mouse**.
* **Launch Ball:** `Spacebar` or **Left Mouse Click**.

*(Note: During gameplay, the mouse cursor is automatically hidden for a better experience).*

## Gameplay Features

* **Advanced Physics:** When bricks are destroyed, they lose power (dimmening in color) and physically fall down the screen.
* **Interactive Debris:** Falling bricks can bounce off your paddle (gaining horizontal spin based on where they hit) and even collide and bounce elastically against other falling bricks. They also pile up on top of unbroken active bricks!
* **Deadly Trap Explosions:** A randomly chosen brick acts as a trap. When broken, it falls down the screen. If it reaches the player's Y-coordinate, it triggers a massive explosion covering half the screen's X-space! You must move away quickly to avoid the blast radius and losing a life.
* **Parallax Starfield:** A multi-layered, dynamic parallax starfield runs in the background, providing a deep, immersive space aesthetic during menus and gameplay.
* **Dynamic Speed:** Every time a brick is broken, your score goes up (10 points each) and the ball's overall speed increases dynamically.
* **3D Visuals:** The blocks and the player paddle feature a dynamic 3D beveled shading effect with drop-shadows.

## Brick Improvements (Power-ups)

When broken blocks fall down the screen, catching them with your paddle can grant you powerful upgrades (and sometimes tricky downgrades!). Here are all the improvements you can collect:

* **BIGGER_BAR:** Increases the width of your paddle, making it easier to catch the ball.
* **STICKY_BAR:** Makes the paddle sticky, catching the ball and allowing you to reposition and launch it again.
* **Y_FREE:** Unlock vertical movement! Move your paddle up and down freely to intercept the ball.
* **BULLETS:** Mounts cannons on your paddle, allowing you to shoot and break bricks directly.
* **EXTRA_LIFE:** Grants you an extra life (+1 HP).
* **EXTRA_BALL:** Spawns an additional ball into the game.
* **MULTI_BALL:** Spawns two extra balls simultaneously!
* **AI_BAR:** Summons an AI-controlled companion paddle above yours that helps bounce the balls back up.
* **FIREBALL:** Ignites your ball, turning it into a blazing fireball.
* **SLOW_DOWN:** Instantly slows down the overall ball speed multiplier, making fast-paced action manageable again.
* **LASER:** Displays a trajectory laser sight for the active ball(s) to help you aim your return perfectly.
* **SHIELD:** Generates a temporary energy barrier at the bottom of the screen that saves a ball from falling out once.
* **SHRINK_BAR:** (Downgrade) Decreases the width of your paddle, making the game harder!

## Customization

You can tweak game settings (like dimensions, physics params, layout and colors) in the various `src/` headers such as `Entities.hpp` and `Game.hpp`.
