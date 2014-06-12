/*
 * gen_header.c: 
 *
 * Copyright (C) 2013 alloc <alloc.young@gmail.com>
 * Skyeye Develop Group, for help please send mail to
 * <skyeye-developer@lists.gro.clinux.org>
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "gen_header.h"
static LIST_HEAD(headers); 
static LIST_HEAD(footers);
typedef struct {
	struct list_head entry;
	tree_t *node;
} nodex_t;


static void add_to_list(struct list_head *list, tree_t *node) {
	nodex_t *e;
	
	e = (nodex_t *)gdml_zmalloc(sizeof *e);
	INIT_LIST_HEAD(&e->entry);	
	e->node = node;	
	list_add_tail(&e->entry, list);
}

void add_header(tree_t *node) {
	add_to_list(&headers, node);
}

void add_footer(tree_t *node) {
	add_to_list(&footers, node);
}

int simics_include_dir(const char *dir) {
	const char *simics_include_str = "#include <simics";	
	char *ret;

	ret = strstr(dir, simics_include_str);
	if(ret) {
		return 1;
	}
	return 0;
}

extern FILE *out;
static void gen_header(node_entry_t *nodex, FILE *f) {
	symbol_t sym = nodex->sym;
	tree_t *node = sym->attr;
	int ret;

	if((ret = simics_include_dir(node->head.str)));
		return;
	out = f;
	gen_src_loc(&node->common.location);
	fprintf(f, "%s", node->head.str);
}

static void gen_header2(nodex_t *nodex, FILE *f) {
	tree_t *node = nodex->node;
	int ret = 0;

	ret = simics_include_dir(node->head.str);
	if(ret)
		return;
	out = f;
	gen_src_loc(&node->common.location);
	fprintf(f, "%s", node->head.str);
}
void gen_device_header(device_t *dev, FILE *f) {
	struct list_head *p;
	nodex_t *head;

	list_for_each(p, &headers) {
		head = list_entry(p, nodex_t, entry);
		gen_header2(head, f);
	}
}

void gen_device_footer(device_t *dev, FILE *f) {
	struct list_head *p;
	nodex_t *head;

	list_for_each(p, &footers) {
		head = list_entry(p, nodex_t, entry);
		gen_header2(head, f);
	}
}
