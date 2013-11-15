#ifndef __CHK_COMMON__
#define __CHK_COMMON__
#include "gen_object.h"
#include "gen_common.h"
void chk_dml_method(object_t *obj, struct method_name *m);
void change_current_table(symtab_t table);
void restore_current_table();
symtab_t get_current_table();
#endif
