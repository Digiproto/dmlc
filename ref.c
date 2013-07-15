/*
 * ref.c: 
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

#include "ref.h"

void ref_info_init(ref_info_t *fi) {
	struct list_head *ptr = &fi->list;
	INIT_LIST_HEAD(ptr);
}

node_info_t  *new_node_info(tree_t *node) {
	node_info_t *n = (node_info_t *)gdml_zmalloc(sizeof(*n));
	INIT_LIST_HEAD(&n->entry);
	n->node = node;
	return n;
}

void add_node_info(ref_info_t *fi, node_info_t *ni) {
	list_add_tail(&ni->entry, &fi->list);
}

void ref_info_destroy(ref_info_t *fi) {
	struct list_head *p;
	node_info_t *ni;
	tree_t *node;
	struct list_head *head;

	head = &fi->list;
	p = head->next;
	while(p != head) {
		ni = list_entry(p,node_info_t,entry);
		node = ni->node;
		if(node->common.type == DML_KEYWORD_TYPE){
			free(node);
		}
		p = p->next;
		free(ni);
	}
}

void ref_info_print(ref_info_t *fi) {
	struct list_head *p;
	node_info_t *ni;
	tree_t *node;

	list_for_each(p, &fi->list) {
		ni = list_entry(p, node_info_t, entry);
		node = ni->node;
		printf("name %s\n", node->common.name);
	}

}
