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
#include "gen_debug.h"

extern symtab_t current_table;
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
			//free(node);
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
		my_DBG("name %s\n", node->common.name);
	}

}

void collect_ref_info(tree_t *expr, ref_info_t *fi){
        tree_t *node;
        node_info_t *ni;
        type_t type = expr->common.type;
        if(type == IDENT_TYPE){
                ni = new_node_info(expr);
                add_node_info(fi, ni);
                return;
        }else if(type == QUOTE_TYPE){
                my_DBG("quote type\n");
                ni = new_node_info(expr);
                add_node_info(fi, ni);
                return;
        }else if(type == COMPONENT_TYPE){
                node = expr->component.expr;
                my_DBG("expr %p \n", node);
                collect_ref_info(node,fi);
                node = dml_keyword_node(expr->common.name, &expr->common.location);
                ni = new_node_info(node);
                add_node_info(fi,ni);
                node = expr->component.ident;
                my_DBG("ident %s, node %p\n", node->ident.str, node);
                ni = new_node_info(node);
                add_node_info(fi,ni);
        } else if (type == DML_KEYWORD_TYPE) {
                /*bank field */
                ni = new_node_info(expr);
                add_node_info(fi, ni);
        }
}

symbol_t get_ref_sym(tree_t *t, ref_ret_t *ret, symtab_t table){
        ref_info_t fi;
        tree_t *node;
        tree_t *tmp;
        tree_t *node_next;
        tree_t *node3;
        node_info_t *ni_next;
        node_info_t *ni;
        struct list_head *p;
        symbol_t sym = NULL;
        const char *name = NULL;
        const char *name2 = NULL;
        struct symtab *symtab;
        int ref_obj = 0;
        object_t *obj = NULL;
        object_t *obj2 = NULL;
	
	if(!table) {
		symtab = current_table;
	} else {
		symtab = table;
	}
        /*collect ref info*/
        ref_info_init(&fi);
        collect_ref_info(t, &fi);
        ref_info_print(&fi);
        p = fi.list.next;
        if(p){
                ni = list_entry(p, node_info_t, entry);
                node = ni->node;
                if(node->common.type == IDENT_TYPE){
                        ref_obj = 0;
                        name = node->ident.str;
                        if(is_simics_api(name)) {
                                ret->is_obj = 0;
                                goto end;
                        }
                }else if(node->common.type == QUOTE_TYPE){
                        ref_obj = 1;
                        node = node->quote.ident;
                        name = node->ident.str;
                }else if(node->common.type == DML_KEYWORD_TYPE){
                        ref_obj = 1;
                        name = node->ident.str;
                } else {
                        my_DBG("error component foramt, type %d\n", node->common.type);
                }
        }
        sym = symbol_find_notype(symtab, name);
        if(!ref_obj && sym->type != OBJECT_TYPE){/*just printf it*/
                ret->is_obj = 0;
                goto end;
        }
        ret->is_obj = 1;
        /*maybe check expression*/
        p = &fi.list;
        struct list_head *head = p;
        my_DBG("outof\n");
        if(p->next->next == p) {
                /*only one node*/
                goto end;
        }

        while(p->next != head)
        {
                ni = list_entry(p->next, node_info_t, entry);
                ni_next = list_entry(p->next->next, node_info_t, entry);
                node = ni->node;
                node_next = ni_next->node;
                if(node_next) {
                        if(node_next->common.type == DML_KEYWORD_TYPE) {
                                if(node->common.type == QUOTE_TYPE){
                                        tmp = node->quote.ident;
                                        name = tmp->ident.str;
                                } else if(node->common.type == IDENT_TYPE || node->common.type == DML_KEYWORD_TYPE){
                                        name = node->ident.str;
                                } else {
                                        my_DBG("error format\n");
                                }
                                node3 = ((node_info_t *)ni_next->entry.next)->node;
                                if(node3->common.type == IDENT_TYPE){
                                        name2 = node3->ident.str;
                                } else {
                                        my_DBG("only ident valid\n");
                                }
                                if(!name2 || !name){
                                        my_DBG("error name null\n");
                                }
                                sym = symbol_find(symtab, name, OBJECT_TYPE);
                                my_DBG("object found %s, sym %p\n", name, sym);
                                if(!sym){
                                        my_DBG("no object %s symbol found\n",name);
                                }
                                obj = (object_t *)(sym->attr);
                                symtab = obj->symtab;
                                my_DBG("object name %s, name %s\n", obj->name, name2);
                                sym = symbol_find_notype(symtab, name2);
                                if(sym){
                                        my_DBG("sym type %d, interface type %d", sym->type, INTERFACE_TYPE);
                                        /*may be interface function need */
                                        if(sym->type == OBJECT_TYPE) {
                                                obj2 = (object_t *)sym->attr;
                                                if(!strcmp(obj2->obj_type, "interface")) {
                                                        my_DBG("interface object\n");
                                                        ret->con = obj;
                                                        ret->iface = obj2;
                                                        node = t->component.ident;
                                                        name = node->ident.str;
                                                        ret->method = name;
                                                        break;
                                                }
                                        }
                                }
                        }
                        p = ni_next->entry.next;
                }
        }
end:
        my_DBG("out of 2\n");
        ref_info_destroy(&fi);
        return sym;
}

