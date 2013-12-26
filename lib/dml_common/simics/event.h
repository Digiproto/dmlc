#ifndef __EVENT_H__ 
#define __EVENT_H__ 

#include "simics/base_types.h"

typedef void (*event_cb_t)(conf_object_t *, void *);
typedef int64_t pc_step_t; 
typedef struct event_class {
        /* function called when event expires */
		event_cb_t cb;
} event_class_t;

#define VT_old_time_post SIM_event_post
#define VT_old_time_clean SIM_event_cancel

void SIM_event_post(conf_object_t *obj, int when, int flags, event_cb_t cb, void *user_data);
void SIM_event_cancel(conf_object_t *obj, int flags, event_cb_t cb, void *user_data);
pc_step_t SIM_step_count(conf_object_t *obj);

#endif /* __EVENT_H__ */
