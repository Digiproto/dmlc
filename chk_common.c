#include <assert.h>
#include "chk_common.h"
extern obj_ref_t *OBJ;
static symtab_t saved_table;
extern symtab_t current_table;

/**
 * @brief pre_chk_method : make some work before check method
 *
 * @param obj : the object of containing method
 * @param method : the method to be checked
 */
static void pre_chk_method(object_t *obj, tree_t *method) {
        method_attr_t *attr;
        symtab_t table;
	
        attr = method->common.attr;
        table = attr->table;
        saved_table = table->parent;
        table->parent = obj->symtab;
        current_table = table;
        OBJ->obj = obj;
}

/**
 * @brief post_chk_method : restore the context after check method
 *
 * @param obj : the object of containing method
 * @param method : the method to be checked
 */
static void post_chk_method(object_t *obj, tree_t *method) {
        method_attr_t *attr;
        symtab_t table;

        attr = method->common.attr;
        table = attr->table;
        table->parent = saved_table;
        OBJ = NULL;
}

/**
 * @brief chk_dml_method : check the method of device
 *
 * @param obj : the object of device
 * @param m : the method of device
 */
void chk_dml_method(object_t *obj, struct method_name *m) {
        tree_t *method = m->method;
	if (method == NULL)
		return;

        /* we should pase the elements and calcualate the
         * expressions that in the method block, as we did
         * not do them before */
        //pre_chk_method(obj, method);
	context_t context;
	obj_ref_t obj_ref;

	obj_ref.obj = obj;
	obj_ref.ref = NULL;
	current_table = obj->symtab;
	pre_gen_method(&obj_ref, method, &context);
        parse_method_block(method);
        //post_chk_method(obj, method);
	post_gen_method(&obj_ref, method, &context);
}

symtab_t get_current_table() {
	return current_table;
}

void change_current_table(symtab_t table) {
	assert(table != NULL);
	saved_table = current_table;
	current_table = table;
	return;
}

void restore_current_table() {
	current_table = saved_table;
	return;
}


