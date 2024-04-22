#ifndef __FSM_H
#define __FSM_H

#include "stdint.h"

#define TRAN(___STATE)  (me->next_state = (uint16_t)(___STATE))

//状态机处理定义
#define STATE_ENTRY_ACTION                    if ( me->current_state != me->previous_state ) {
#define STATE_TRANSITION_ACTION                me->previous_state = me->current_state; } if ( me->next_state == me->current_state ) {
#define STATE_TRANSITION_ACTION_EXCLUSIVE    }else if ( me->next_state == me->current_state ) {
#define STATE_EXIT_ACTION                     }if( me->next_state != me->current_state ) {
#define STATE_EXIT_ACTION_EXCLUSIVE            }else if ( me->next_state != me->current_state ) {
#define STATE_END                            me->current_state = me->next_state; } break;

typedef struct
{
    uint8_t sig;
    
}event_t;

typedef struct
{
    uint8_t previous_state;
    uint8_t current_state;
    uint8_t next_state;
    
}state_t;


#endif

