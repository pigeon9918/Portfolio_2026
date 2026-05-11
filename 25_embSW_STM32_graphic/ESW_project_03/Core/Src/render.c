
#include <string.h>
#include <stdbool.h>
#include "main.h"
#include "ili9341.h"
#include "render.h"
#include "button.h"

// --- Global Variables ---
Entity g_entities[MAX_ENTITIES];

static unsigned int start_intensity = (0xFF >> 1);
static unsigned int intensity_change = 1;
static int color = 0;

static void clear_entities() {
  // Initialize all entities to be not visible
  memset(g_entities, 0, sizeof(g_entities));
  for (int i = 0; i < MAX_ENTITIES; i++) {
    g_entities[i].id = -1; // Invalidate ID
  }
}

// Initializes the UI by creating button entities
void RENDER_InitUI(void) {
  // Clear any existing entities to have a clean slate
  clear_entities();

  RENDER_CreateEntity(
    100,
    100,
    CURSOR_24x24_WIDTH,
    CURSOR_24x24_WIDTH,
    cursor_icon_24x24
  );

  RENDER_CreateEntity(
    400,
    400,
    CURSOR_24x24_WIDTH,
    CURSOR_24x24_WIDTH,
    cursor_icon_24x24
  );

  RENDER_CreateEntity(
    BUTTON_POWER_X,
    BUTTON_POWER_Y,
    32,
    32,
    button_power_icon_32x32
  );

  RENDER_CreateEntity(
    BUTTON_COLOR_X,
    BUTTON_COLOR_Y,
    BUTTON_WIDTH,
    BUTTON_HEIGHT,
    button_power_icon_32x32
  );
}

// Creates a new entity and adds it to the list for rendering
int RENDER_CreateEntity(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* data) {
  for (int i = 0; i < MAX_ENTITIES; i++) {
    if (!g_entities[i].is_visible) {
      g_entities[i].id = i;
      g_entities[i].x = x;
      g_entities[i].y = y;
      g_entities[i].width = width;
      g_entities[i].height = height;
      g_entities[i].data = data;
      g_entities[i].is_visible = true;
      return i; // Return entity ID
    }
  }
  return -1; // No space for new entity
}

// Main drawing function for a single horizontal line
void draw_line(int line_num, uint16_t * line_buf) {
  draw_bg(line_num, line_buf);
  draw_tile(line_num, line_buf);
}

// Draws the scrolling gradient background
void draw_bg(int line_num, uint16_t * line_buf) {
  // line_num이 처음 또는 마지막 줄이면 White border로 채우고 return.
  if (line_num == 0 || line_num == ILI9341_HEIGHT - 1) {
    for(int i=0; i<ILI9341_WIDTH; i++) {
      line_buf[i] = 0xFFFF;
    }
    return;
  }
  
  int brightness = (start_intensity + line_num) < 248 ? (start_intensity + line_num) : 248*2 - (start_intensity + line_num);

  uint16_t rgb_val = 0;
  if(color == 2)
    rgb_val = ILI9341_COLOR565(0, 0, brightness);
  else if(color == 1)
    rgb_val = ILI9341_COLOR565(0, brightness, 0);
  else
    rgb_val = ILI9341_COLOR565(brightness, 0, 0);

  for(int i=1; i<ILI9341_WIDTH - 1; i++) {
    line_buf[i] = (rgb_val << 8) | (rgb_val >> 8);
  }
  
  // line_buf의 첫 pixel과 마지막 pixel은 White border로 채움.
  line_buf[0] = line_buf[ILI9341_WIDTH - 1] = 0xFFFF;
}

// change bg color
inline void change_bg_color() {
  color++;
  color %= 3;
}

// overwrite icon
void draw_tile(int line_num, uint16_t * line_buf) {
  for (int i = 0; i < MAX_ENTITIES; i++) {
    Entity* ent = &g_entities[i];

    if (!ent->is_visible) {
      continue;
    }

    if (line_num >= ent->y && line_num < (ent->y + ent->height)) {
      int local_y = line_num - ent->y;
      const uint16_t* pixel_data_row = ent->data + (local_y * ent->width);

      for (int local_x = 0; local_x < ent->width; local_x++) {
        int screen_x = ent->x + local_x;

        if (screen_x >= 1 && screen_x < ILI9341_WIDTH - 1) {
          uint16_t rgb_val = pixel_data_row[local_x];
          if (rgb_val != 0x0000) {
            line_buf[screen_x] = (rgb_val << 8) | (rgb_val >> 8);
          }
        }
      }
    }
  }
}

// Called at the end of each full frame render
void end_of_frame(void) {
  static int pos = 1;

  // start_intensity를 +1 증가 또는 -1 감소 시킴.
  start_intensity = (pos) ? start_intensity + intensity_change : start_intensity - intensity_change;
  // start_intensity가 MAX 이면 다음부터는 감소.
  if (start_intensity > 240) {
    pos = 0; 
  }
  // start_intensity가 0 이면 다음부터는 증가.
  else if (start_intensity == 0x0000) {
    pos = 1;
  }
}