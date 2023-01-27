#ifndef __PAGE_H
#define __PAGE_H


typedef enum
{
    PAGE_1 = 0,
    PAGE_2,
    PAGE_3,
    PAGE_4,
    PAGE_5,

}page_index_e;



void page_init();
void send_page_event(page_index_e type);
void send_page_event_from_isr(page_index_e type);

#endif
