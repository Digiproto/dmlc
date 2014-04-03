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
extern FILE *out;
static void gen_header(node_entry_t *nodex, FILE *f) {
	symbol_t sym = nodex->sym;
	tree_t *node = sym->attr;
	out = f;
	gen_src_loc(&node->common.location);
	fprintf(f, "\n%s", node->head.str);
}

void gen_device_header(device_t *dev, FILE *f) {
	struct list_head *p;
	tree_t *node;
	node_entry_t *head;

	list_for_each(p, &dev->headers) {
		head = list_entry(p, node_entry_t, entry);
		gen_header(head, f);
	}
}

void gen_device_footer(device_t *dev, FILE *f) {
	struct list_head *p;
	tree_t *node;
	node_entry_t *head;

	list_for_each(p, &dev->footers) {
		head = list_entry(p, node_entry_t, entry);
		gen_header(head, f);
	}
}
