#include "gen_port.h"

static void gen_port_description(object_t *obj, FILE *f) {
	dml_port_t *port = (dml_port_t *)obj;
	int  i = 0;
	object_t  *impl;

	fprintf(f, "\nstatic struct InterfaceDescription %s_port_ifaces[] = {\n", obj->name);
	for (i = 0; i < port->num; i++) {
		impl = port->impls[i];
		fprintf(f, "\t[%d] = (struct InterfaceDescription ) {\n", i);
		fprintf(f, "\t\t.name = \"%s\",\n", impl->name);
		fprintf(f, "\t\t.iface = &%s_iface,\n", impl->qname);
		fprintf(f, "\t},\n");
	}
	fprintf(f, "\t[%d] =  {}\n", i);
	fprintf(f, "};\n");
}

void gen_device_port(device_t  *dev, FILE *f) {
	struct list_head *p;
	object_t *tmp;
	int i = 0;

	fprintf(f, "\nstatic const struct PortDescription %s_ports[] = {\n", dev->obj.name);	
	list_for_each(p, &dev->ports){
		tmp = (object_t *)list_entry(p, object_t, entry);
		fprintf(f, "\t[%d] = (struct PortDescription) {\n", i);
		fprintf(f, "\t\t.name = \"%s\",\n", tmp->name);
		fprintf(f, "\t\t.id = %s_port_ifaces,\n", tmp->name);
		fprintf(f, "\t},\n");
		i++;
	}
	fprintf(f, "\t[%d] = {}\n", i);
	fprintf(f, "};\n");
}

static void gen_port_implement_code(object_t *obj, FILE *f) {
	dml_port_t *port = (dml_port_t *)obj;
	int i = 0;
	
	for(i = 0; i < port->num; i++) {
		gen_implement_code(port->impls[i], f);
	}
}

void gen_port_implement_iface(object_t *obj, FILE *f) {
	dml_port_t *port = (dml_port_t *)obj;
	int i = 0;
	
	for(i = 0; i < port->num; i++) {
		gen_iface(port->impls[i], f);
	}
}

static void gen_port_implement_header(object_t *obj, FILE *f) {
	dml_port_t *port = (dml_port_t *)obj;
	int i = 0;
	
	for(i = 0; i < port->num; i++) {
		gen_implement_header((object_t *)port->impls[i], f);
	}
}

void gen_device_port_code(device_t *dev, FILE *f) {
	struct list_head *p;
	object_t *tmp;
	dml_port_t *port;
	list_for_each(p, &dev->ports) {
		tmp = (object_t *)list_entry(p, object_t, entry);
		gen_port_implement_code(tmp, f);
#if backend != 4
		gen_port_implement_iface(tmp, f);
#endif
	}
}

void add_port_method(object_t *obj) {
	object_t *tmp;
	dml_port_t *port;
	int i;
	port = (dml_port_t *)obj;

	for(i = 0; i < port->num; i++) {
		add_implement_method(port->impls[i]);
	}
}

void gen_device_port_description(device_t *dev, FILE *f) {
	struct list_head *p;
	object_t *tmp;
	
	list_for_each(p, &dev->ports) {
		tmp = (object_t *)list_entry(p, object_t, entry);
		gen_port_description(tmp, f);
	}
	gen_device_port(dev, f);
}

void gen_device_port_header(device_t *dev, FILE *f) {
	struct list_head *p;
	object_t *tmp;
	
	list_for_each(p, &dev->ports) {
		tmp = (object_t *)list_entry(p, object_t, entry);
		gen_port_implement_header(tmp, f);
	}
}


