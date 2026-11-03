#include <raylib.h>

int main() {
  InitWindow(800, 450, "level editor");
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("hello", 100, 100, 32, RAYWHITE);
    EndDrawing();
  }
  return 0;
}
