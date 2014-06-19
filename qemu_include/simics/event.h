/*
 * event.h: 
 *
 * Copyright (C) 2013 alloc <alloc.young@gmail.com>
 * Skyeye Develop Group, for help please send mail to
 * <skyeye-developer@lists.gro.clinux.org>
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __EVENT_H__ 
#define __EVENT_H__ 
#include "simics/base_types.h"
typedef void (*event_cb_t)(conf_object_t *, void *);
typedef struct event_class {
        /* function called when event expires */
		event_cb_t cb;
} event_class_t;

typedef struct { cycles_t (*get_cycle_count)(conf_object_t *queue); double (*get_time)(conf_object_t *queue); cycles_t (*cycles_delta)(conf_object_t *clock, double when); uint64 (*get_frequency)(conf_object_t *queue); void (*post_cycle)(conf_object_t *queue, event_class_t *evclass, conf_object_t *obj, cycles_t cycles, void *user_data); void (*post_time)(conf_object_t *queue, event_class_t *evclass, conf_object_t *obj, double seconds, void *user_data); void (*cancel)(conf_object_t *queue, event_class_t *evclass, conf_object_t *obj, int (*pred)(void *data, void *match_data), void *match_data); cycles_t (*find_next_cycle)(conf_object_t *queue, event_class_t *evclass, conf_object_t *obj, int (*pred)(void *data, void *match_data), void *match_data); attr_value_t (*events)(conf_object_t *obj); cycles_t (*advance)(conf_object_t *queue, cycles_t cycles); } cycle_interface_t;


#define VT_old_time_post SIM_event_post
#define VT_old_time_clean SIM_event_cancel

#define VT_old_step_post SIM_event_step_post
#define VT_old_step_clean SIM_event_step_cancel


void SIM_event_post(conf_object_t *obj, int when, int flags, event_cb_t cb, void *user_data);
void SIM_event_cancel(conf_object_t *obj, int flags, event_cb_t cb, void *user_data);
pc_step_t SIM_step_count(conf_object_t *obj);
void SIM_event_step_post(conf_object_t *obj, int when, event_cb_t cb, void *user_data);
void SIM_event_step_cancel(conf_object_t *obj,  event_cb_t cb, void *user_data);

#endif /* __EVENT_H__ */
