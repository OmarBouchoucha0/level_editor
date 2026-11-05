#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>

#define SCALE 50.0f
#define TILE_COUNT 60
#define TILE_HEIGHT 32
#define TILE_WIDTH 32
#define WIDTH_IN_TILES SCALE * 16 / TILE_WIDTH
#define HEIGHT_IN_TILES SCALE * 9 / TILE_HEIGHT
#define MAX_ENTITIES 1000
#define PLAYER_ID 1
#define VELOCITY 300
#define SCREEN_CENTER_X 16.0f * SCALE / 2.0f
#define SCREEN_CENTER_Y 9.0f * SCALE / 2.0f

typedef struct Entity Entity;
struct Entity {
  Vector2 pos;
  Vector2 dimensions;
  Texture2D texture;
  bool focused;
  int stage;
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

Vector2 fromScreenPosToWorldPos(Vector2 screen_pos, Camera2D camera) {
  Vector2 world_pos = {0};
  world_pos.x = screen_pos.x + camera.target.x - SCREEN_CENTER_X;
  world_pos.y = screen_pos.y + camera.target.y - SCREEN_CENTER_Y;
  return world_pos;
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

void input(float dt, Entity *entities, Camera2D camera) {
  Vector2 mouse_pos = fromScreenPosToWorldPos(GetMousePosition(), camera);

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    for (int i = 0; i < LAST_ENTITY_ID; ++i) {
      Entity *tile = &entities[i];
      if (mouse_pos.x >= tile->pos.x &&
          mouse_pos.x <= tile->pos.x + tile->dimensions.x &&
          mouse_pos.y >= tile->pos.y &&
          mouse_pos.y <= tile->pos.y + tile->dimensions.y) {
        tile->focused = true;
        FOCUSED_ENTITY = tile;
      } else {
        FOCUSED_ENTITY = NULL;
      }
    }
  }

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && FOCUSED_ENTITY != NULL) {
    Vector2 d = GetMouseDelta();
    FOCUSED_ENTITY->pos.x += d.x;
    FOCUSED_ENTITY->pos.y += d.y;
  }

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && FOCUSED_ENTITY != NULL) {
    snapEntityToGrid(FOCUSED_ENTITY);
  }

  if (IsKeyDown(KEY_LEFT_CONTROL)) {

    if (IsKeyPressed(KEY_C)) {
      COPIED_ENTITY = FOCUSED_ENTITY;
    }
    if (IsKeyPressed(KEY_V)) {
      Entity tile = {0};
      tile.pos = mouse_pos;
      tile.dimensions = COPIED_ENTITY->dimensions;
      tile.focused = true;
      tile.stage = STAGE;
      tile.texture = COPIED_ENTITY->texture;
      addEntity(entities, tile);
    }
  }

  if (IsKeyDown(KEY_A)) {
    entities[PLAYER_ID].pos.x -= VELOCITY * dt;
  }
  if (IsKeyDown(KEY_D)) {
    entities[PLAYER_ID].pos.x += VELOCITY * dt;
  }
  if (IsKeyDown(KEY_W)) {
    entities[PLAYER_ID].pos.y -= VELOCITY * dt;
  }
  if (IsKeyDown(KEY_S)) {
    entities[PLAYER_ID].pos.y += VELOCITY * dt;
  }

  if (IsKeyPressed(KEY_KP_0)) {
    STAGE = 0;
  }
  if (IsKeyPressed(KEY_KP_1)) {
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

void buildDebugStageGridTexture(RenderTexture2D *grid_texture) {
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

void loadDebugTileset(Entity *entities, Texture2D tileset[TILE_COUNT]) {
  int row = 0;
  int col = 0;
  for (int i = 0; i < TILE_COUNT; ++i) {
    Entity tile = {0};
    tile.pos.x = col * TILE_WIDTH;
    tile.pos.y = row * TILE_HEIGHT;
    tile.dimensions.x = TILE_WIDTH;
    tile.dimensions.y = TILE_HEIGHT;
    tile.focused = false;
    tile.stage = STAGE;
    tile.texture = tileset[i];
    addEntity(entities, tile);
    row++;
    if (row >= HEIGHT_IN_TILES - 1) {
      row = 0;
      col++;
    }
  }
}

void drawEntities(Entity *entities) {
  for (int i = 1; i < LAST_ENTITY_ID; ++i) {
    if (entities[i].stage == STAGE) {
      DrawTextureV(entities[i].texture, entities[i].pos, WHITE);
    }
  }
}

int main() {
  InitWindow(16 * SCALE, 9 * SCALE, "level editor");
  SetTargetFPS(60);
  SetMouseCursor(MOUSE_CURSOR_ARROW);
  Texture2D tileset[TILE_COUNT];
  loadTilesetTextures(tileset);
  RenderTexture2D grid_texture = {0};
  buildDebugStageGridTexture(&grid_texture);

  Entity entities[MAX_ENTITIES];
  Entity player = {0};
  player.pos.x = SCREEN_CENTER_X;
  player.pos.y = SCREEN_CENTER_Y;
  player.dimensions.x = TILE_WIDTH;
  player.dimensions.y = TILE_HEIGHT;
  player.focused = false;
  player.stage = STAGE;
  addEntity(entities, player);

  loadDebugTileset(entities, tileset);

  Camera2D camera = {0};
  camera.target =
      (Vector2){entities[PLAYER_ID].pos.x, entities[PLAYER_ID].pos.y};
  camera.offset = (Vector2){GetRenderWidth() / 2.0f, GetRenderHeight() / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  int fps;
  float dt;
  char buffer[1000];
  while (!WindowShouldClose()) {
    dt = GetFrameTime();
    fps = GetFPS();

    camera.target =
        (Vector2){entities[PLAYER_ID].pos.x, entities[PLAYER_ID].pos.y};
    input(dt, entities, camera);
    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(camera);
    drawGrid(grid_texture);
    drawEntities(entities);
    EndMode2D();

    snprintf(buffer, sizeof(buffer), "fps : %d", fps);
    DrawText(buffer, 2, 10, 8, WHITE);

    switch (STAGE) {
    case 0: {
      snprintf(buffer, sizeof(buffer), "STAGE 0");
    } break;
    case 1: {
      snprintf(buffer, sizeof(buffer), "STAGE 1");
    } break;
    }
    DrawText(buffer, 2, 20, 8, WHITE);

    snprintf(buffer, sizeof(buffer), "camera : {x:%.2f|y:%.2f}",
             camera.target.x, camera.target.y);
    DrawText(buffer, 2, 30, 8, WHITE);

    Vector2 mouse_pos = fromScreenPosToWorldPos(GetMousePosition(), camera);
    snprintf(buffer, sizeof(buffer), "mouse : {x:%.2f|y:%.2f}", mouse_pos.x,
             mouse_pos.y);
    DrawText(buffer, 2, 40, 8, WHITE);
    EndDrawing();
  }
  return 0;
}
