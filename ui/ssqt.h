#ifndef __SSQT_H
#define __SSQT_H

typedef struct
{
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t hight;
    char *txt;  //只用做文字距离边缘x=0,y=center
    char *img;  //图片在文字下方刷新
    //sq_widget_t *parent; 指向控件或者页面
    //链表孩子保存
    //信号与槽函数
}sq_widget_t;

/*
按钮：背景，文字
*/
typedef struct
{
    //控件第一个孩子指针
    //缓冲区
}sq_page_t;

//切换页面需要全屏刷
//页面内部局部刷
//检测左右滑动与单击双击事件
#endif
