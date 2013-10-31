#include <assert.h>
#include "chk_common.h"
extern object_t *OBJ;
static symtab_t saved_table;
extern symtab_t current_table;

static void pre_chk_method(object_t *obj, tree_t *method) {
        method_attr_t *attr;
        symtab_t table;

        attr = method->common.attr;
        table = attr->table;
        saved_table = table->parent;
        table->parent = obj->symtab;
        current_table = table;
        OBJ = obj;
}

static void post_chk_method(object_t *obj, tree_t *method) {
        method_attr_t *attr;
        symtab_t table;

        attr = method->common.attr;
        table = attr->table;
        table->parent = saved_table;
        OBJ = NULL;
}

void chk_dml_method(object_t *obj, struct method_name *m) {
        tree_t *method = m->method;

        /* we should pase the elements and calcualate the
         * expressions that in the method block, as we did
         * not do them before */
        pre_chk_method(obj, method);
        parse_method_block(method);
        post_chk_method(obj, method);
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


