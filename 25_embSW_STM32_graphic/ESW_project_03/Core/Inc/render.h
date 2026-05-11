#ifndef __RENDER_H__
#define __RENDER_H__

#include <stdint.h>
#include <stdbool.h>

#define MAX_ENTITIES 10 // Increased entity limit

// Represents a drawable object on the screen (sprite/tile)
typedef struct {
    int id;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    const uint16_t* data; // Pointer to 16-bit RGB565 data
    bool is_visible;
} Entity;

extern Entity g_entities[MAX_ENTITIES];

// Initializes the UI elements (e.g., buttons) by creating them as entities.
void RENDER_InitUI(void);

// Creates a new entity for rendering. Returns the entity's ID or -1 on failure.
int RENDER_CreateEntity(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* data);

// Main drawing function called per horizontal line by the display driver.
void draw_line(int line_num, uint16_t * line_buf);

// Draws the background for a given line.
void draw_bg(int line_num, uint16_t * line_buf);

// Draws all visible entities (tiles/sprites) for a given line.
void draw_tile(int line_num, uint16_t * line_buf) ;

void end_of_frame(void);

void change_bg_color();

#endif
