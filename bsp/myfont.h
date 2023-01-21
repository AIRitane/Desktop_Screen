#ifndef __MYFRONT_H
#define __MYFRONT_H

#define FONT_LINE_HEIGHT 26
#define FONT_BASE_LINE 24
#define FONT_GAP 0
#define FONT_BPP    1

int lvgl_get_bitmap(uint32_t letter, uint8_t *bitmap_buf, uint8_t *box_w, uint8_t *box_h, int8_t *offset_x, int8_t *offset_y);

#endif