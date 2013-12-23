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
#include <assert.h>
#include <string.h>
#include  "ast.h"
#include "ref.h"
#include "gen_debug.h"
#include "info_output.h"
#include "gen_common.h"

extern obj_ref_t *OBJ;
extern symtab_t current_table;
void ref_info_init(ref_info_t *fi) {
	struct list_head *ptr = &fi->list;
	INIT_LIST_HEAD(ptr);
}

node_info_t  *new_node_info(tree_t *node, tree_t *index) {
	node_info_t *n = (node_info_t *)gdml_zmalloc(sizeof(*n));
	INIT_LIST_HEAD(&n->entry);
	n->node = node;
	n->index = index;
	return n;
}

ref_info_t *new_ref_info(void) {
        ref_info_t *ref = (ref_info_t *)gdml_zmalloc(sizeof(*ref));
        ref_info_init(ref);
        return ref;
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
		ni = new_node_info(expr, NULL);
                add_node_info(fi, ni);
                return;
        }else if(type == QUOTE_TYPE){
                my_DBG("quote type\n");
		ni = new_node_info(expr, NULL);
                add_node_info(fi, ni);
                return;
        }else if(type == COMPONENT_TYPE){
                node = expr->component.expr;
                my_DBG("expr %p \n", node);
                collect_ref_info(node,fi);
		node = dml_keyword_node(expr->component.comp, &expr->common.location);
		ni = new_node_info(node, NULL);
                add_node_info(fi,ni);
                node = expr->component.ident;
                my_DBG("ident %s, node %p\n", node->ident.str, node);
		ni = new_node_info(node, NULL);
                add_node_info(fi,ni);
        } else if (type == DML_KEYWORD_TYPE) {
                /*bank field */
		ni = new_node_info(expr, NULL);
                add_node_info(fi, ni);
        }  else if (type == BIT_SLIC_EXPR_TYPE) {
			collect_ref_info(expr->bit_slic.expr, fi);
			ni = new_node_info(NULL, expr->bit_slic.bit);
			add_node_info(fi, ni);
	} else {
		my_DBG("TODO: other type %d\n", type);
		printf("TODO: other type %d\n", type);
	}
}

void printf_ref(ref_ret_t *ref_ret){
	tree_t *node;
	struct list_head *p = NULL;
	node_info_t *ni;
	const char *name = NULL;
	ref_info_t *fi;
	symbol_t sym;
	int i = 0;
	tree_t *tmp = NULL;
	object_t *obj = NULL;
	struct list_head *head;
	const char *alias = NULL;
	char *tmp_str = NULL;
	object_t *con = NULL;

	if(ref_ret->con) {
			con = ref_ret->con;
	}

	fi = ref_ret->ref;
	head = &fi->list;
	p = head->next;
	while(p != head) {
		ni = list_entry(p, node_info_t, entry);
		node = ni->node;
		normal_case:
		if(node) {
			if(node->common.type == IDENT_TYPE){
					name = node->ident.str;
			} else if (node->common.type == DML_KEYWORD_TYPE){
					//name = node->common.name;
					name = node->ident.str;
			}else if(node->common.type == QUOTE_TYPE){
					tmp = node->quote.ident;
					name = tmp->ident.str;
			} else {
					my_DBG("node name %s type %d\n",node->common.name,node->common.type);

			}
			if(i == 0){
					i++;
					sym = symbol_find_notype(current_table, name);
					if(!sym){
							my_DBG("no sym %s found in current symtab \n",name);
					}
					if(ref_ret->is_obj) {
							obj = (object_t *)sym->attr;
							alias = get_obj_ref(obj);
							if(obj->is_array) {
								int len = 0;
								tmp_str = strrchr(alias, '[');	
								if(tmp_str) {
									len = tmp_str - alias;
								}
								tmp_str = gdml_zmalloc(len + 1);
								memcpy(tmp_str, alias, len);
								tmp_str[len] = '\0';	
								alias = tmp_str;
							}
					} else {
							alias = get_symbol_alias(sym);
					}
					D("%s", alias);
			} else {
					D("%s", name);
			}
			p = p->next;
	   } else if(ni->index) {
			while(!ni->node && ni->index) {
					D("[");
					translate(ni->index);
					D("]");
					p = p->next;
					if(p == head) {
							break;
					} else {
							ni = list_entry(p, node_info_t, entry);
					}
			}
			node = ni->node;
			if(p != head) {
					goto normal_case;
			}
		}
	}
	if(tmp_str){
		free(tmp_str);
	}
}

extern object_t* get_device_obj();
extern symtab_t get_symbol_table(symtab_t sym_tab, symbol_t symbol);
extern symbol_t get_symbol_from_root_table(const char* name, type_t type);
static object_t* get_bank_obj(symtab_t table, const char* name) {
	object_t* obj = NULL;
	symbol_t sym = NULL;
	object_t* dev = get_device_obj();
	struct list_head* head = dev->childs.next;
	long long head_ptr = (long long)(head);
	long long childs_ptr = (long long)(&(dev->childs));
	if (head_ptr != childs_ptr) {
		obj = (object_t*)(list_entry(head, object_t, entry));
	} else {
	/* when a device not have bank, the default_bank is template bank*/
		obj = gdml_zmalloc(sizeof(object_t));
		sym = get_symbol_from_root_table("bank", TEMPLATE_TYPE);
		obj->name = strdup(name);
		obj->obj_type = strdup("bank");
		obj->symtab = get_symbol_table(table, sym);
		check_template_parsed("bank");
	}

	return obj;
}

static object_t* get_parameter_obj(symtab_t table, const char* name) {
	assert(table != NULL); assert(name != NULL);
	symbol_t symbol = symbol_find(table, name, PARAMETER_TYPE);
        parameter_attr_t* attr = symbol->attr;
        if (attr->is_original == 0) return;
        tree_t* node = attr->common.node;
        tree_t* param_spec = node->param.paramspec;
        tree_t* expr_node = NULL;
        if (param_spec && param_spec->paramspec.expr) {
                expr_node = param_spec->paramspec.expr;
        } else {
                return NULL;
        }
        tree_t* ident = NULL;
        symbol_t obj_sym = NULL;
        if (expr_node->common.type == QUOTE_TYPE) {
                ident = expr_node->quote.ident;
                obj_sym = symbol_find(table, ident->ident.str, OBJECT_TYPE);
                if (obj_sym) {
                        return (object_t*)(obj_sym->attr);
                }
        }
        else {
		fprintf(stderr, "other expr node type '%s'\n", expr_node->common.name);
		exit(-1);
        }
        return (object_t*)(obj_sym->attr);
}

symtab_t get_obj_param_table(symtab_t table, symbol_t symbol) {
	assert(table != NULL);  assert(symbol != NULL);
	object_t* obj = get_parameter_obj(table, symbol->name);
	symtab_t ret_table = NULL;
	if (obj) {
		ret_table = obj->symtab;
	}
	return ret_table;
}

extern symbol_t get_symbol_from_template(symtab_t table, const char* parent_name, const char* name);
static object_t* get_other_type_obj(symtab_t table, const char* parent_name, const char* name) {
	assert(table != NULL); assert(name != NULL);
	object_t* obj = NULL;
	symbol_t sym = NULL;
	if (!strcmp(name, "this")) {
		obj = OBJ->obj;
	}
	else {
		sym = symbol_find_notype(table, name);
		if (sym->type == SELECT_TYPE) {
			select_attr_t* attr = sym->attr; if (strcmp(name, "bank") || attr->type != PARAMETER_TYPE) {
				error("no object %s symbol found line: %d", name, __LINE__);
			}
		} else if (sym->type == DATA_TYPE) {
			obj = gdml_zmalloc(sizeof(object_t));
			obj->name = sym->name;
			obj->obj_type = strdup("data");
			obj->symtab = get_symbol_table(table, sym);
			return obj;
		} else if (sym->type == REGISTER_TYPE) {
			object_attr_t* attr = sym->attr;
			obj = gdml_zmalloc(sizeof(object_t));
			obj->name = sym->name;
			obj->node = attr->common.node;
			obj->obj_type = strdup("register");
			symtab_t tmp = get_symbol_table(table, sym);
			if (tmp == NULL && parent_name) {
				sym = get_symbol_from_template(table, parent_name, name);
				tmp = get_symbol_table(table, sym);
			}
			obj->symtab = tmp;
			return obj;
			/* not implement*/
		} else if (sym->type != PARAMETER_TYPE && sym->type != DATA_TYPE &&
				sym->type != REGISTER_TYPE) {
			error("no object %s symbol found line: %d", name, __LINE__);
		}
		if (!strcmp(name, "default_bank") || !strcmp(name, "bank")) {
			obj = get_bank_obj(table, name);
		} else if (sym->type == PARAMETER_TYPE) {
			return get_parameter_obj(table, sym->name);
		}
		else {
			error("no object %s symbol found lien: %d", name, __LINE__);
		}
	}

	return obj;
}

symbol_t get_ref_sym(tree_t *t, ref_ret_t *ret, symtab_t table){
		ref_info_t *fi;
        tree_t *node;
        tree_t *tmp;
        tree_t *node_next;
        tree_t *node3;
        node_info_t *ni_next;
        node_info_t *ni;
        struct list_head *p;
        symbol_t sym = NULL;
		const char* fore_name = NULL;
        const char *name = NULL;
        const char *name2 = NULL;
        struct symtab *symtab;
        int ref_obj = 0;
        object_t *obj = NULL;
        object_t *obj2 = NULL;
		int has_interface = 0;
		tree_t *index = NULL;
	
		if(!table) {
			symtab = current_table;
		} else {
			symtab = table;
		}
        /*collect ref info*/
		fi = new_ref_info();
		collect_ref_info(t, fi);
		ret->ref = fi;
		//ref_info_print(fi);
		p = fi->list.next;
        if(p){
                ni = list_entry(p, node_info_t, entry);
                node = ni->node;
		//printf("node name: %s\n", node->common.name);
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
        p = fi->list.next;
        struct list_head *head = &fi->list;
        my_DBG("outof\n");
		if(p->next == p) {
                /*only one node*/
                goto end;
        }

        while(p->next != head)
        {
				ni = list_entry(p, node_info_t, entry);
				ni_next = list_entry(p->next, node_info_t, entry);
				node = ni->node;
                node_next = ni_next->node;
normal_case:
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
                                } else if (node3->common.type == DML_KEYWORD_TYPE){
                                        name2 = node3->ident.str;
				} else {
                                        my_DBG("only ident valid\n");
                                }
                                if(!name2 || !name){
                                        my_DBG("error name null\n");
                                }
				sym = symbol_find(symtab, name, OBJECT_TYPE);
				if (sym) {
					obj = (object_t*)(sym->attr);
				} else {
					obj = get_other_type_obj(symtab, fore_name, name);
				}
                                symtab = obj->symtab;
				if (symtab->is_parsed == 0 && obj->node) {
					parse_unparsed_obj(obj->node, symtab);
				}
                                my_DBG("object name %s, name %s\n", obj->name, name2);
				if(!strcmp(obj->obj_type, "interface")) {
					obj2 = obj;
					ret->con = NULL;
					ret->iface = obj2;
					ret->index = index;
					node = t->component.ident;
					ret->method = node->ident.str;
					has_interface = 1;
					break;
				}
				symtab->obj = obj;
                                sym = symbol_find_notype(symtab, name2);
                                if(sym){
					my_DBG("sym type %d, interface type %d\n", sym->type, INTERFACE_TYPE);
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
							ret->index = index;
							has_interface = 1;
                                                        break;
                                                }
                                        }
                                }
				else {
					printf("symbol '%s' not find\n", name2);
				}
                        }
                        p = ni_next->entry.next;
			fore_name = name;
                } else if (ni_next->index) {
					/*skip index*/
					while(ni_next->index) {
							index = ni_next->index;
							p = ni_next->entry.next;
							ni_next = list_entry(p, node_info_t, entry);
					}
					node_next = ni_next->node;;
					if(p != head) {
							goto normal_case;
					}
				}
        }
	ret->index = index;
	if(has_interface && ret->con) {
		 /*just ref object part*/
		 p = &ni_next->entry;
		 p->prev->next = head;
	}
end:
        my_DBG("out of 2\n");
        return sym;
}
