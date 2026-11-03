#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>

#define SCALE 50
#define TILE_COUNT 60
#define TILE_HEIGHT 32
#define TILE_WIDTH 32
#define WIDTH_IN_TILES SCALE * 16 / TILE_WIDTH
#define HEIGHT_IN_TILES SCALE * 9 / TILE_HEIGHT
#define MAX_ENTITIES 1000
#define PLAYER_ID 1

typedef struct Entity Entity;
struct Entity {
  Vector2 pos;
  Vector2 dimensions;
  Texture2D texture;
  bool focused;
};

static bool DEBUG_MODE = true;
static int STAGE = 0;
static int LAST_ENTITY_ID = 1;
static Entity *FOCUSED_ENTITY;
static Entity *COPIED_ENTITY;

void addEntity(Entity *entities, Entity entity) {
  entities[LAST_ENTITY_ID] = entity;
  LAST_ENTITY_ID++;
}

void snapEntityToGrid(Entity *tile) {
  Vector2 center = {0};
  center.x = tile->pos.x + tile->dimensions.x / 2.0f;
  center.y = tile->pos.y + tile->dimensions.y / 2.0f;
  float column = floorf(center.x / TILE_WIDTH);
  float row = floorf(center.y / TILE_HEIGHT);
  tile->pos.x = column * TILE_WIDTH;
  tile->pos.y = row * TILE_HEIGHT;
}

void input(Entity *entities) {
  Vector2 mouse_pos = GetMousePosition();
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    for (int i = 0; i < LAST_ENTITY_ID; ++i) {
      Entity *tile = &entities[i];
      if (mouse_pos.x >= tile->pos.x &&
          mouse_pos.x <= tile->pos.x + tile->dimensions.x &&
          mouse_pos.y >= tile->pos.y &&
          mouse_pos.y <= tile->pos.y + tile->dimensions.y) {
        tile->focused = true;
        FOCUSED_ENTITY = tile;
      }
    }
  }

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && FOCUSED_ENTITY->focused) {
    Vector2 d = GetMouseDelta();
    FOCUSED_ENTITY->pos.x += d.x;
    FOCUSED_ENTITY->pos.y += d.y;
  }

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    snapEntityToGrid(FOCUSED_ENTITY);
  }

  if (IsKeyDown(KEY_LEFT_CONTROL)) {

    if (IsKeyPressed(KEY_C)) {
      COPIED_ENTITY = FOCUSED_ENTITY;
    }
    if (IsKeyPressed(KEY_V)) {
      Vector2 mouse_pos = GetMousePosition();
      Entity tile = {0};
      tile.pos = mouse_pos;
      tile.dimensions = COPIED_ENTITY->dimensions;
      tile.focused = true;
      tile.texture = COPIED_ENTITY->texture;
      addEntity(entities, tile);
    }
  }

  if (IsKeyPressed(KEY_KP_0)) {
    STAGE = 0;
  }
  if (IsKeyPressed(KEY_KP_0)) {
    STAGE = 1;
  }

  if (IsKeyPressed(KEY_F1)) {
    DEBUG_MODE = !DEBUG_MODE;
  }
}

void loadTilesetTextures(Texture2D tileset[TILE_COUNT]) {
  char buffer[500];
  for (size_t i = 0; i < TILE_COUNT; i++) {
    snprintf(buffer, sizeof(buffer),
             "/home/omar/Work/game_level_editor/assets/Tiles/Tile_%ld.png",
             i + 1);
    tileset[i] = LoadTexture(buffer);
  }
}

void buildGridTexture(RenderTexture2D *grid_texture) {
  *grid_texture = LoadRenderTexture(WIDTH_IN_TILES * 32, HEIGHT_IN_TILES * 32);
  BeginTextureMode(*grid_texture);
  ClearBackground(BLANK);
  char buffer[32];
  for (int i = 0; i < WIDTH_IN_TILES; ++i) {
    for (int j = 0; j < HEIGHT_IN_TILES; ++j) {
      snprintf(buffer, sizeof(buffer), "%d:%d", i, j);
      int text_width = MeasureText(buffer, 8);
      DrawText(buffer, i * 32 + (32 - text_width) / 2, j * 32 + 12, 8, RED);
      DrawRectangleLinesEx((Rectangle){i * 32, j * 32, 32, 32}, 1.0f, RED);
    }
  }
  EndTextureMode();
}
void drawGrid(RenderTexture2D grid_texture) {
  if (!DEBUG_MODE)
    return;
  DrawTextureRec(grid_texture.texture,
                 (Rectangle){0, 0, grid_texture.texture.width,
                             -grid_texture.texture.height},
                 (Vector2){0, 0}, WHITE);
}

void drawMap(int map[WIDTH_IN_TILES][HEIGHT_IN_TILES],
             Texture2D tileset[TILE_COUNT]) {
  for (size_t i = 0; i < WIDTH_IN_TILES; ++i) {
    for (size_t j = 0; j < HEIGHT_IN_TILES; ++j) {
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

void loadTileset(Entity *entities, Texture2D tileset[TILE_COUNT]) {
  int row = 0;
  int col = 0;
  for (int i = 0; i < TILE_COUNT; ++i) {
    Entity tile = {0};
    tile.pos.x = col * TILE_WIDTH;
    tile.pos.y = row * TILE_HEIGHT;
    tile.dimensions.x = TILE_WIDTH;
    tile.dimensions.y = TILE_HEIGHT;
    tile.focused = false;
    tile.texture = tileset[i];
    addEntity(entities, tile);
    row++;
    if (row >= HEIGHT_IN_TILES) {
      row = 0;
      col++;
    }
  }
}

void drawEntities(Entity *entities) {
  for (int i = 1; i < LAST_ENTITY_ID; ++i) {
    DrawTextureV(entities[i].texture, entities[i].pos, WHITE);
  }
}

int main() {
  InitWindow(16 * SCALE, 9 * SCALE, "level editor");
  SetTargetFPS(60);
  int map[WIDTH_IN_TILES][HEIGHT_IN_TILES] = {0};
  Texture2D tileset[TILE_COUNT];
  loadTilesetTextures(tileset);
  RenderTexture2D grid_texture = {0};
  buildGridTexture(&grid_texture);

  Entity entities[MAX_ENTITIES];
  loadTileset(entities, tileset);

  int fps;
  float dt;
  char buffer[1000];
  while (!WindowShouldClose()) {
    dt = GetFrameTime();
    fps = GetFPS();

    snprintf(buffer, sizeof(buffer), "fps : %d", fps);
    input(entities);
    BeginDrawing();
    ClearBackground(BLACK);
    switch (STAGE) {
    case 0: {
    } break;
    default: {
    };
    }

    drawGrid(grid_texture);
    drawEntities(entities);
    DrawText(buffer, 2, 10, 8, WHITE);
    EndDrawing();
  }
  return 0;
}
