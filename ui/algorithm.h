/*
* 信号不带参数，槽函数也不能带参数版本
*/
#ifndef __ALGORITHM_H
#define __ALGORITHM_H

// 槽函数类型
typedef void (*slot_t)();

//链表
typedef struct __node node_t;
 struct __node{
    slot_t   data;
    node_t*  next;
};

// 信号类型
typedef struct
{
    node_t *slot;
} signal_t;

signal_t *Qsignal();
void Qconnect(signal_t *_signal, slot_t _slot);
void emit(signal_t *_signal);
void del(signal_t *_signal);

#endif
