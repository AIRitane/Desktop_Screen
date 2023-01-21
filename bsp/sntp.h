#ifndef __SNTP_H
#define __SNTP_H

typedef struct
{
    char year[6];
    char mon[6];
    char week[6];
    char hour[6];
    char min[6];
    char sec[6];
}sntp_time_t;

void obtain_time(void);
void sntp_time_fresh(sntp_time_t *sntp_time);

#endif