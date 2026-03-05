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

* **Move Paddle:** `Left/Right Arrows`, `A / D` keys, or simply move your **Mouse**.
* **Launch Ball:** `Spacebar` or **Left Mouse Click**.

*(Note: During gameplay, the mouse cursor is automatically hidden for a better experience).*

## Gameplay Features

* **Advanced Physics:** When bricks are destroyed, they lose power (dimmening in color) and physically fall down the screen.
* **Interactive Debris:** Falling bricks can bounce off your paddle (gaining horizontal spin based on where they hit) and even collide and bounce elastically against other falling bricks. They also pile up on top of unbroken active bricks!
* **Dynamic Speed:** Every time a brick is broken, your score goes up (10 points each) and the ball's overall speed increases dynamically.
* **3D Visuals:** The blocks and the player paddle feature a dynamic 3D beveled shading effect with drop-shadows.

## Customization

You can tweak game settings (like ball speed, paddle speed, layout, colors, and gravity) in `src/main.cpp`.
