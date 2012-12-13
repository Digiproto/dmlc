#ifndef __AST_H__
#define __AST_H__
typedef struct device_node{
	char* name;
}device_node;

typedef struct bitorder_node{
	char* name;
}bitorder_node;
#if 0
typedef struct import_node{
	char* filename;
}import_node_t
typedef struct template_node{
	char* name;
}
typedef struct loggroup_node{
}

typedef struct typedef_node{
}
typedef struct constant_node{
}
typedef struct struct_node{
}
typedef struct object_stmt_node{
}
typedef struct parameter_node{
}
typedef struct method_node{
}
typedef struct istemplate_stmt_node{
}

/* object node */
typedef struct bank_node{
	char* name;
	char* template_name;
}

int insert_bank_node(char* name, char* template_name){
}
typedef struct register_node{
}
typedef struct register_array_node{
}

typedef struct field_node{
}
typedef struct data_node{
}
typedef struct connect_node{
}

typedef struct connect_array_node{
}

typedef struct interface_node{
}

typedef struct attribute_node{
}
typedef struct attribute_array_node{
}

typedef struct event_node{
}
typedef struct group_node{
}
typedef struct group_array_node{
}

typedef struct port_node{
}
typedef struct implement_node{
}
#endif
typedef struct node{
	int type;
	char* name;
	struct node* sibling;
	struct node* child;
}node_t;
#endif
