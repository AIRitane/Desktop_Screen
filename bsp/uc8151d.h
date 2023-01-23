#ifndef LVGL_DEMO_UC8151D_H
#define LVGL_DEMO_UC8151D_H

/*************************************管脚定义**************************************/
#define PIN_DC 14
#define PIN_DC_BIT ((1ULL << 14))
#define PIN_RST 12
#define PIN_RST_BIT ((1ULL << 12))
#define PIN_BUSY 13
#define PIN_BUSY_BIT ((1ULL << 13))
#define PIN_CS 27
#define PIN_CS_BIT ((1ULL << 27))
#define PIN_MOSI 26
#define PIN_MISO 23
#define PIN_CLK 25

/*************************************外部接口**************************************/
/*************************************基础驱动**************************************/
void screen_init();
void screen_full_display(char *buf);
void screen_partial_display(unsigned int x_start, unsigned int y_start, char *buf, unsigned int PART_COLUMN, unsigned int PART_LINE);
void screen_set_ram(char *buf);
void refresh_part();
void deep_sleep();

/*************************************基本UI驱动**************************************/
void screen_draw_point(char *buf, uint16_t x, uint16_t y, uint8_t color);
void screen_draw_fill(char *buf, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color);
void screen_draw_line(char *buf, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color);
void screen_draw_rectangle(char *buf, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color);
void screen_draw_wide_rectangle(char *buf, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t width, uint8_t color);
void screen_pic_overlay(char *buf, char *pic, uint16_t x, uint16_t y, uint16_t width, uint16_t high);
/*************************************汉字UI驱动**************************************/
void screen_show_chinese(char *buf, uint16_t x, uint16_t y, const char *str, uint16_t color);
#endif // LVGL_DEMO_UC8151D_H
