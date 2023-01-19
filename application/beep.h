#ifndef __BEEP_H
#define __BEEP_H

typedef enum
{
    BEEP_SHORT_100MS = 0,
    BEEP_SHORT_200MS,
    BEEP_LONG,
} BEEP_TYPE_E;

void beep_init(void);
void send_beep_event(BEEP_TYPE_E type);
void send_beep_event_from_isr(BEEP_TYPE_E type);

#endif