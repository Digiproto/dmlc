#include "device_check.h"
#include "gen_implement.h"
#include "gen_port.h"
#include "gen_event.h"

static void add_register_check_method(object_t *obj) {
	struct list_head *p;
	object_t *tmp;
	
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		add_object_method(tmp, "read_access");
		add_object_method(tmp, "write_access");
	}	
	
}


static void add_pre_method(device_t *dev) {
	struct list_head *p;
	object_t *tmp;

	add_object_method(&dev->obj, "init");
	add_object_method(&dev->obj, "post_init");
	add_object_method(&dev->obj, "soft_reset");
	add_object_method(&dev->obj, "hard_reset");
	/*add banks, registers*/
	list_for_each(p, &dev->obj.childs) {
		tmp = list_entry(p, object_t, entry);
		add_object_method(tmp, "access");
		add_register_check_method(tmp);
	}	
	/*add more object to check*/
	list_for_each(p, &dev->implements) {
		tmp = list_entry(p, object_t, entry);
		add_implement_method(tmp);
	}
	list_for_each(p, &dev->ports) {
		tmp = list_entry(p, object_t, entry);
		add_port_method(tmp);
	}
	list_for_each(p, &dev->obj.events) {
		tmp = list_entry(p, object_t, entry);
		add_event_method(tmp);
	}
}

static void init_list_head(object_t *obj) {
	INIT_LIST_HEAD(&obj->methods);	
	INIT_LIST_HEAD(&obj->method_generated);	
}

static void init_obj(object_t *obj) {
	struct list_head *p;
	object_t *tmp;
	
	init_list_head(obj);
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		init_obj(tmp);
	}	
}

static void back_to_zero(device_t *dev) {
	struct list_head *p;
	object_t *tmp;
	int i;
	
	init_obj(&dev->obj);
	/*init more checked object, connect, attribute etc*/
#define INIT_ETC(x) \
	do{ \
		list_for_each(p, &dev->x) { \
			tmp = list_entry(p, object_t, entry); \
			init_obj(tmp); \
		} \
	}while(0)

	//INIT_ETC(events);
	INIT_ETC(connects);
	INIT_ETC(constants);
	INIT_ETC(attributes);
	INIT_ETC(implements);
	INIT_ETC(ports);
	list_for_each(p, &dev->ports) {
		tmp = list_entry(p, object_t, entry); 
		dml_port_t *port = (dml_port_t *)tmp;
		for(i = 0; i < port->num; i++) {
			init_obj(port->impls[i]);
		}
	}
#undef INIT_ETC
	list_for_each(p, &dev->obj.events) {
		tmp = list_entry(p, object_t, entry);
		init_obj(tmp);
	}
}

static void after_check(device_t *dev) {
	back_to_zero(dev);
}

void device_check(device_t *dev) {
	add_pre_method(dev);
	chk_dml_code(dev);
	after_check(dev);
}
