#pragma once
// should be able to look at examples/network/simple_sniffer for a multi file cmake example
void set_px(uint8_t r, uint8_t c);
void render(Point *points, size_t len, uint8_t start_r, uint8_t start_c);
void render_char(int c, uint8_t start_x, uint8_t start_y);
void fin_render();
void init_strip_or_something();

typedef struct {
    uint8_t x;
    uint8_t y;
} Point;
