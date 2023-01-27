#ifndef __SSQT_H
#define __SSQT_H

#include "algorithm.h"
#include <stdint.h>

// 组件
typedef struct
{
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t hight;
    char *txt;
    uint8_t txt_offsetx;
    uint8_t txt_offsety;
    char *img;
    uint8_t img_offsetx;
    uint8_t img_offsety;

    signal_t *clicked;
    signal_t *long_clicked;
    signal_t *left_swipe;
    signal_t *right_swipe;
    signal_t *up_swipe;
    signal_t *down_swipe;
} sq_widget_t;

// 页面
typedef struct __sq_node sq_node_t;
struct __sq_node
{
    sq_widget_t *data;
    sq_node_t *next;
};

typedef struct
{
    sq_node_t *widget;
    char buf[5000];
} sq_page_t;

// APP管理
typedef struct __sq_pnode sq_pnode_t;
struct __sq_pnode
{
    sq_page_t *data;
    sq_pnode_t *next;
};

typedef struct
{
    sq_pnode_t *page;
    sq_pnode_t *sel_page;
} sq_app_t;

sq_app_t *sq_creat_app();
void sq_app_page_add(sq_app_t *app, sq_page_t *page);
void delete_app_page(sq_app_t *app, sq_page_t *page);

sq_page_t *creat_page();
sq_widget_t *creat_widget(sq_page_t *page);
void delete_page(sq_page_t *page);
void delete_widget(sq_page_t *page, sq_widget_t *widget);
void set_sel_page(sq_app_t *app,sq_page_t *page);

void ssqt_init(sq_app_t *app);
#endif
