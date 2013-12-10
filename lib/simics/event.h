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
#include <simics/base_types.h>
typedef void (*event_cb_t)(conf_object_t *, void *);
typedef struct event_class {
        /* function called when event expires */
		event_cb_t cb;
} event_class_t;

void SIM_event_post(conf_object_t *clock, conf_object_t *obj, int when, int flags, event_cb_t cb, void *user_data );
void SIM_event_cancel(conf_object_t *clock, conf_object_t *obj, event_cb_t cb);

#endif /* __EVENT_H__ */
