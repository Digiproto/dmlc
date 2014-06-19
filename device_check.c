#include "device_check.h"
#include "gen_implement.h"
#include "gen_port.h"
#include "gen_event.h"
static void init_obj(object_t *obj);
static void add_group_register_method(object_t *obj) {
	group_t *gp;
	struct list_head *p;
	object_t *tmp;

	gp = (group_t *)obj;
	list_for_each(p, &gp->groups){
		tmp = list_entry(p, object_t, entry);
		add_group_register_method(tmp);
	}
	list_for_each(p, &gp->registers){
		tmp = list_entry(p, object_t, entry);
		add_object_method(tmp, "read_access");
		add_object_method(tmp, "write_access");

	}
}

static void add_attribute_method(object_t *obj) {
	attribute_t *attr;
	
	attr = (attribute_t *)obj;
	if(!attr->alloc_type) {
		//add_object_method(obj, "set");
		//add_object_method(obj, "get");
	}
}
/**
 * @brief add_register_check_method : add some default methods of registers
 * into mehod list of register
 *
 * @param obj : the object of register
 */
static void add_register_check_method(object_t *obj) {
	struct list_head *p;
	object_t *tmp;
	bank_t *bank;
	
	bank = (bank_t *)obj;	
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		add_object_method(tmp, "read_access");
		add_object_method(tmp, "write_access");
	}	
	//fprintf(stderr, "obj type %s\n", obj->obj_type);
	list_for_each(p, &bank->groups){
		tmp = list_entry(p, object_t, entry);
		add_group_register_method(tmp);
	}
}
/**
 * @brief add_pre_method : add some default methods of object to object method list
 *
 * @param dev : the object of device
 */
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
	list_for_each(p, &dev->attributes) {
		tmp = list_entry(p, object_t, entry);
		add_attribute_method(tmp);
	}
}

static void init_list_head(object_t *obj) {
	INIT_LIST_HEAD(&obj->methods);	
	INIT_LIST_HEAD(&obj->method_generated);	
}

static void init_none_object(object_t* obj) {
	/* we do not need to do anything */
	return;
}

static void init_bank_sepcial_obj(object_t* obj) {
	bank_t*  bank = (bank_t*)obj;
	struct list_head *p;
	object_t *t;

	list_for_each(p, &obj->events){
		t = list_entry(p, object_t, entry);
		init_obj(t);
	}
	list_for_each(p, &bank->attributes){
		t = list_entry(p, object_t, entry);
		init_obj(t);
	}
	list_for_each(p, &bank->implements){
		t = list_entry(p, object_t, entry);
		init_obj(t);
	}
	list_for_each(p, &bank->groups){
		t = list_entry(p, object_t, entry);
		init_obj(t);
	}
	return;
}

static void init_group_special_obj(object_t* obj) {
	struct list_head* p;
	object_t* tmp;
	group_t* gp = (group_t*)obj;
	list_for_each(p, &obj->events){
		tmp = list_entry(p, object_t, entry);
		init_obj(tmp);
	}
	list_for_each(p, &gp->registers){
		tmp = list_entry(p, object_t, entry);
		init_obj(tmp);
	}
	list_for_each(p, &gp->groups){
		tmp = list_entry(p, object_t, entry);
		init_obj(tmp);
	}
}

static void init_port_special_object(object_t* obj) {
	struct list_head *p;
	object_t *tmp;
	dml_port_t* port = (dml_port_t*)obj;
	list_for_each(p, &obj->events){
		tmp = list_entry(p, object_t, entry);
		init_obj(tmp);
	}
	list_for_each(p, &port->connects){
		tmp = list_entry(p, object_t, entry);
		init_obj(tmp);
	}
	list_for_each(p, &port->attributes){
		tmp = list_entry(p, object_t, entry);
		init_obj(tmp);
	}
	list_for_each(p, &port->implements){
		tmp = list_entry(p, object_t, entry);
		init_obj(tmp);
	}
	return;
}

static void (*init_obj_special[])(object_t* obj) = {
	[Obj_Type_None] = init_none_object,
	[Obj_Type_Device] = init_none_object,
	[Obj_Type_Bank] = init_bank_sepcial_obj,
	[Obj_Type_Register] = init_none_object,
	[Obj_Type_Field]  = init_none_object,
	[Obj_Type_Attribute] = init_none_object,
	[Obj_Type_Connect]  = init_none_object,
	[Obj_Type_Port]    = init_port_special_object,
	[Obj_Type_Implement] = init_none_object,
	[Obj_Type_Interface] = init_none_object,
	[Obj_Type_Data]     = init_none_object,
	[Obj_Type_Event]    = init_none_object,
	[Obj_Type_Group]    = init_group_special_obj
};
/**
 * @brief init_obj : init all object lists in object
 *
 * @param obj : the object of lists
 */
static void init_obj(object_t *obj) {
	struct list_head *p;
	object_t *tmp;
	
	init_list_head(obj);
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		init_obj(tmp);
	}	
	init_obj_special[obj->encoding](obj);
}

/**
 * @brief back_to_zero : set all lists in device to initializing state
 *
 * @param dev : the object of device
 */
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
	list_for_each(p, &dev->attributes) {
		tmp = list_entry(p, object_t, entry);
		init_obj(tmp);
	}
}

/**
 * @brief after_check : free the mehod list of object after check finish
 *
 * @param dev : the object of device
 */
static void after_check(device_t *dev) {
	struct list_head *p;
	object_t *tmp;

	back_to_zero(dev);
	
	list_for_each(p, &dev->obj.childs) {
		tmp = list_entry(p, object_t, entry);
		if ((!strcmp(tmp->obj_type, "bank")) && (!strcmp(tmp->name, "__fake_bank"))) {
			p = p->next;
			list_del(&tmp->entry);
		}
	}
}

/**
 * @brief device_check : check the declaration and expression of device block
 *
 * @param dev : the object of device
 */
void device_check(device_t *dev) {
	add_pre_method(dev);
	chk_dml_code(dev);
	after_check(dev);
}
