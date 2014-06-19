/*
 * attribute.h: 
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

#ifndef __ATTRIBUTE_H__
#define __ATTRIBUTE_H__  
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
typedef enum {
        Sim_Val_Invalid,
        Sim_Val_String,
        Sim_Val_Integer,
        Sim_Val_Floating,
        Sim_Val_List,
        Sim_Val_Data,
        Sim_Val_Nil,
        Sim_Val_Object,
        Sim_Val_Dict,
        Sim_Val_Boolean,

        /* Internal use only */
        Sim_Val_Unresolved_Data,
        Sim_Val_Unresolved_Object
} attr_kind_t;

typedef struct attr_value {
        attr_kind_t          kind;
        union {
                const char *string;          /* Sim_Val_String */
                int64_t integer;           /* Sim_Val_Integer */
                bool boolean;                /* Sim_Val_Boolean */
                double floating;             /* Sim_Val_Floating */

                /* Sim_Val_List */
                struct {
                        size_t size;
                        struct attr_value *vector;  /* [size] */
                } list;

                /* Sim_Val_Dict */
                struct {
                        size_t size;
                        struct attr_dict_pair *vector;  /* [size] */
                } dict;

                /* Sim_Val_Data */
                struct {
                        size_t size;
                        uint8_t *data;         /* [size] */
                } data;

                DeviceState *object;       /* Sim_Val_Object */

                /* Sim_Val_Unresolved_Data (internal use only) */
                struct {
                        size_t size;
                        char *filename;
                        uint64_t offset;
                } udata;

                /* Sim_Val_Unresolved_Object (internal use only) */
                char *uobject;
        } u;
}attr_value_t;

struct attr_dict_pair {
        struct attr_value   key;
        struct attr_value   value;
};

static inline bool SIM_attr_is_list(attr_value_t attr) {
	return attr.kind == Sim_Val_List;
}

static inline bool SIM_attr_is_object(attr_value_t attr) {
	return attr.kind == Sim_Val_Object;
}

static inline attr_value_t SIM_make_attr_invalid(void) {
	attr_value_t tmp;
	tmp.kind = Sim_Val_Invalid;
	return tmp;
}

/*exact data from attr*/
static inline int64_t SIM_attr_integer(attr_value_t val) {
	return val.u.integer;
}

static inline const char* SIM_attr_string(attr_value_t val) {
	return val.u.string;
}

static inline bool SIM_attr_boolean(attr_value_t val) {
	return val.u.boolean;
}

static inline double SIM_attr_floating(attr_value_t val) {
	return val.u.floating;
}

static inline unsigned SIM_attr_list_size(attr_value_t val) {
	return val.u.list.size;
}

static inline DeviceState *SIM_attr_object(attr_value_t val) {
	return val.u.object;
}
static inline attr_value_t SIM_attr_list_item(attr_value_t val, unsigned index) {
	return val.u.list.vector[index];
}

static inline attr_value_t SIM_alloc_attr_list(unsigned len) {
	attr_value_t tmp;
	tmp.kind = Sim_Val_List;
	tmp.u.list.size = len;
	tmp.u.list.vector = (attr_value_t *)malloc(sizeof(attr_value_t) * len);
	memset((void *)tmp.u.list.vector, 0, len * sizeof(attr_value_t));
	return tmp;
}

static inline void SIM_attr_list_set_item(attr_value_t *attr, unsigned index, attr_value_t item) {
	/*FIXME need release resouces held by ex-attr, right now it's done by brutal force*/
	attr->u.list.vector[index] = item;
}

static inline attr_value_t SIM_make_attr_boolean(bool b) {
	attr_value_t tmp;
	tmp.kind = Sim_Val_Boolean;
	tmp.u.boolean = b;
	return tmp;
}

static inline attr_value_t SIM_make_attr_floating(double d) {
	attr_value_t tmp;
	tmp.kind = Sim_Val_Floating;
	tmp.u.floating = d;
	return tmp;
}

static inline attr_value_t SIM_make_attr_integer(int64_t i) {
	attr_value_t tmp;
	tmp.kind = Sim_Val_Integer;
	tmp.u.integer = i;
	return tmp;
}

static inline attr_value_t SIM_make_attr_obj(DeviceState *obj) {
	attr_value_t tmp;
	tmp.kind = Sim_Val_Object;
	tmp.u.object = obj;
	return tmp;
}

static inline attr_value_t SIM_make_attr_string(const char *name) {
	attr_value_t tmp;
	tmp.kind = Sim_Val_String;
	tmp.u.string = strdup(name);
	return tmp;
}

#endif /* __ATTRIBUTE_H__ */
