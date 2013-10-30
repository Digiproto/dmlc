#include "device_check.h"

static add_register_check_method(object_t *obj) {
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

}

static void init_list_head(object_t *obj) {
	INIT_LIST_HEAD(&obj->methods);	
	INIT_LIST_HEAD(&obj->method_generated);	
}


static init_register_obj(object_t *obj) {
	struct list_head *p;
	object_t *tmp;
	
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		init_list_head(tmp);
	}	
	
}

static void back_to_zero(device_t *dev) {
	struct list_head *p;
	object_t *tmp;
	
	init_list_head(&dev->obj);
	/*add banks, registers*/
	list_for_each(p, &dev->obj.childs) {
		tmp = list_entry(p, object_t, entry);
		init_list_head(tmp);
		init_register_obj(tmp);
	}	
	/*add more object to check*/

}

static void after_check(device_t *dev) {
	back_to_zero(dev);
}

void device_check(device_t *dev) {
	add_pre_method(dev);
	check_dml_code(dev);
	after_check(dev);
}
