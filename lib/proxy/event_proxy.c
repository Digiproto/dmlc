#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>

#include <simics/dev_iface.h>
#include <simics/base_types.h>
#include <simics/bank_access.h>
#include <simics/core/object_model.h>
#include <simics/core/object_class.h>
#include <simics/core/object_iface.h>
#include <simics/core/object_resource.h>
#include <simics/event.h>

#include "schedule_tmp.h"

/*
typedef enum timing_unit {
	TIMING_UNIT_CYCLE,
	TIMING_UNIT_STEP,
	TIMING_UNIT_SECOND,
}timing_unit_t;
*/

/*
typedef struct event_class {
}event_class_t;
*/

typedef struct event_list {
	event_cb_t cb;
	conf_object_t *obj;
	void *user_data;
	schedule_t *sched;
	struct event_list *next;
}event_list_t;

typedef struct event_proxy {
	conf_object_t obj;
	sched_group_t *group;
	pc_step_t init_time;
	event_list_t list;
}event_proxy_t;

static inline uint64_t get_now_usec() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000000 + now.tv_usec;
}

static void* event_proxy_callback(void *obj)
{
	event_list_t *event = (event_list_t*) obj;
	event->cb(event->obj, event->user_data);
	return NULL;
}

static event_list_t* find_event(event_list_t *list, event_cb_t cb, void *user_data) {
	event_list_t *tmp = list;
	for(tmp = tmp->next; tmp; tmp = tmp->next) {
		if(tmp->cb == cb && tmp->user_data == user_data)
			return tmp;
	}
	return NULL;
}

static event_list_t* insert_event(event_list_t *list, conf_object_t *obj,
		event_cb_t cb, void *user_data) {
	event_list_t *event = MM_ZALLOC(sizeof(*event));
	event->cb = cb;
	event->obj = obj;
	event->user_data = user_data;

	event_list_t *tmp;
	for(tmp = list; tmp->next; tmp = tmp->next) {
	}
	tmp->next = event;
	return event;
}

static void remove_event(event_list_t *list, event_cb_t cb, void *user_data) {
	event_list_t *tmp, *f;
	for(tmp = list; tmp->next; tmp = tmp->next) {
		f = tmp->next;
		if(f->cb == cb && f->user_data == user_data) {
			tmp->next = f->next;
			free(f);
			break;
		}
	}
}

static void proxy_post(conf_object_t *clock, conf_object_t *obj, int when, event_cb_t cb, void *user_data) {
	event_proxy_t *proxy = (event_proxy_t*) clock;
	event_list_t *event = insert_event(&proxy->list, obj, cb, user_data);
	event->sched = schedule_tmp_insert(proxy->group,
			event_proxy_callback, event, SchedTmpOnce, when);
}

static void proxy_cancel(conf_object_t *clock, event_cb_t cb, void *user_data) {
	event_proxy_t *proxy = (event_proxy_t*) clock;
	event_list_t *tmp = find_event(&proxy->list, cb, user_data);
	schedule_tmp_delete(tmp->sched);
	schedule_tmp_destroy_sched(tmp->sched);
	remove_event(&proxy->list, cb, user_data);
}

static pc_step_t proxy_count(conf_object_t *clock) {
	event_proxy_t *proxy = (event_proxy_t*) clock;
	return proxy->init_time - get_now_usec();
}

void SIM_event_post(conf_object_t *obj, int when, int flags, event_cb_t cb, void *user_data) {
	conf_object_t *clock = obj->clock;
	proxy_post(clock, obj, when, cb, user_data + 1000);
}

void SIM_event_cancel(conf_object_t *obj, int flags, event_cb_t cb, void *user_data)
{
	conf_object_t *clock = obj->clock;
	proxy_cancel(clock, cb, user_data);
}

pc_step_t SIM_step_count(conf_object_t *obj) {
	conf_object_t *clock = obj->clock;
	return proxy_count(clock);
}

static const struct ConnectDescription event_proxy_connects[] = {
	[0] = {}
};

static const struct InterfaceDescription event_proxy_ifaces[] =  {
	[0] = {}
};

static const struct AttributeDescription event_proxy_attributes[] = {
	[0] = {}
};

static const class_resource_t event_proxy_resource = {
	.ifaces = event_proxy_ifaces,
	.connects = event_proxy_connects,
	.attributes = event_proxy_attributes,
};

static const struct bank_access_description event_proxy_bank_access[] = {
	[0] = {}
};

static conf_object_t* event_proxy_new_instance(const char *objname) {
	event_proxy_t *proxy = (event_proxy_t*) MM_ZALLOC(sizeof(*proxy));
	proxy->group = schedule_tmp_create();
	proxy->init_time = get_now_usec();
	SIM_object_register(&proxy->obj, objname);
	return &proxy->obj;
}

static const class_data_t event_proxy_data = {
	.new_instance = event_proxy_new_instance,
	.resources = &event_proxy_resource,
	.bank_access = event_proxy_bank_access,
};

void init_local(void) {
	static conf_class_t conf_class = {
		.cls_name = "event_proxy",
		.cls_data = &event_proxy_data,
	};
	SIM_register_conf_class(conf_class.cls_name, &conf_class);
}
