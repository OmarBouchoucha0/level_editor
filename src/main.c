#include <raylib.h>
#include <stdio.h>
#include <string.h>

#define SCALE 50
#define TILE_COUNT 61
#define WIDTH_IN_TILES SCALE * 16 / 32
#define HEIGHT_IN_TILES SCALE * 9 / 32

static bool DEBUG_MODE = true;
static int STAGE = 0;

void input() {
  if (IsKeyPressed(KEY_LEFT)) {
    STAGE--;
  }
  if (IsKeyPressed(KEY_RIGHT)) {
    STAGE++;
  }

  if (IsKeyPressed(KEY_F1)) {
    DEBUG_MODE = !DEBUG_MODE;
  }
}

void loadTilesetTextures(Texture2D tileset[TILE_COUNT]) {
  char buffer[500];
  for (size_t i = 1; i < 61; i++) {
    snprintf(buffer, sizeof(buffer),
             "/home/omar/Work/game_level_editor/assets/Tiles/Tile_%ld.png", i);
    tileset[i] = LoadTexture(buffer);
  }
}

void drawMap(int map[WIDTH_IN_TILES][HEIGHT_IN_TILES],
             Texture2D tileset[TILE_COUNT]) {
  char buffer[500];
  int text_width;
  for (size_t i = 0; i < WIDTH_IN_TILES; ++i) {
    for (size_t j = 0; j < HEIGHT_IN_TILES; ++j) {
      if (DEBUG_MODE) {
        snprintf(buffer, sizeof(buffer), "%ld:%ld", i, j);
        text_width = MeasureText(buffer, 8);
        DrawText(buffer, i * 32 + text_width / 2, j * 32, 8, RED);
        DrawRectangleLinesEx((Rectangle){i * 32, j * 32, 32, 32}, 1.0f, RED);
      }
      if (map[i][j] > 0 && map[i][j] < TILE_COUNT) {
        DrawTextureV(tileset[map[i][j]], (Vector2){i * 32, j * 32}, WHITE);
      }
    }
  }
}

void debugStage(int map[WIDTH_IN_TILES][HEIGHT_IN_TILES]) {
  int counter = 1;
  for (size_t i = 0; i < WIDTH_IN_TILES; ++i) {
    for (size_t j = 0; j < HEIGHT_IN_TILES; ++j) {
      if (counter >= TILE_COUNT) {
        return;
      }
      map[i][j] = counter;
      counter++;
    }
  }
}

void emptyStage(int map[WIDTH_IN_TILES][HEIGHT_IN_TILES]) {
  memset(map, 0, (WIDTH_IN_TILES - 1) * HEIGHT_IN_TILES * sizeof(map[0][0]));
}

int main() {
  InitWindow(16 * SCALE, 9 * SCALE, "level editor");
  int map[WIDTH_IN_TILES][HEIGHT_IN_TILES] = {0};
  Texture2D tileset[TILE_COUNT];
  loadTilesetTextures(tileset);

  while (!WindowShouldClose()) {
    input();
    BeginDrawing();
    ClearBackground(BLACK);
    switch (STAGE) {
    case 0: {
      debugStage(map);
    } break;
    default: {
      emptyStage(map);
    };
    }
    drawMap(map, tileset);
    EndDrawing();
  }
  return 0;
}
