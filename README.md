# Aronkanoid

Aronkanoid is a simple Arkanoid clone built in C++ using the [Raylib](https://www.raylib.com/) library.

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

* **Move Left:** `Left Arrow` or `A`
* **Move Right:** `Right Arrow` or `D`
* **Launch Ball:** `Spacebar`

## Gameplay

* Destroy all the bricks to win.
* You start with 3 lives.
* If the ball falls below the paddle, you lose a life.
* The final score is based on the number of bricks broken (10 points each).

## Customization

You can tweak game settings (like ball speed, paddle speed, layout, colors) in `src/main.cpp`.
