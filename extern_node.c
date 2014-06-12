#include <stdio.h>
#include "tree.h"
#include "ast.h"
#include "list.h"
#include "string.h"

typedef struct {
        struct list_head entry;
        tree_t *node;
        const char *name;
} extern_entry_t;

static LIST_HEAD(externs);

void add_extern(tree_t *node, const char *name) {
	extern_entry_t *e;
	
	e = gdml_zmalloc(sizeof *e);
	e->node = node;
	e->name = strdup(name);
	INIT_LIST_HEAD(&e->entry);
	list_add_tail(&e->entry, &externs);
}

int in_extern(const char *name) {
	struct list_head *p;
	extern_entry_t *e;

	list_for_each(p, &externs) {
		e = list_entry(p, extern_entry_t, entry);
		if(!strcmp(e->name, name))
			return 1;
	}
	return 0;
}
