
/* Copyright (C) 
* 2012 - Michael.Kang blackfin.kang@gmail.com
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/

/* 
* Symbol table management for gdml.
*/

/**
* @file symbol.c
* @brief 
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2012-11-30
*/

#include <stdio.h>				/* for (f)printf(), std{out,int} */
#include <stdlib.h>				/* for exit */
#include <string.h>				/* for strcmp, strdup & friends */
#include <assert.h>
#include "symbol.h"
#ifdef SYMBOL_DEBUG
#define DBG(fmt, ...) do { fprintf(stderr, fmt, ## __VA_ARGS__); } while (0)
#else
#define DBG(fmt, ...) do { } while (0)
#endif

#define MAX_SYMBOLS 10000
static int str_hash (const char *str)
{
	if (!*str)
		return 0;
	int len = strlen (str);
	int i = 0, j = 0;
	int s[4] = { 0, 0, 0, 0 };
	for (; i < 4; i++) {
		j = i;
		while (j < len) {
			s[i] += (int) str[j];
			//printf("for s[%d]=%d, str[%d]=%d\n", i, s[i], j, str[j]);
			j += 4;
		}
		//printf("s[%d]=%d\n", i, s[i]);
	}
	int hash_value = (s[0] % 10) * 1000 + (s[1] % 10) * 100
		+ (s[2] % 10) * 10 + s[3] % 10;
	//printf("hash of %s is %d\n", str, hash_value);
	return hash_value;
}

static symbol_t *symbol_table[MAX_SYMBOLS];	/* not visible from outside */

symbol_t *symbol_find (char *name, int type)
{
	symbol_t *symbol = symbol_table[str_hash (name)];
	//printf("In %s, name=%s, hash value=%d\n", __FUNCTION__, name, str_hash(name));
	if (symbol == NULL) {
		/* can not find the symbol */
		printf ("can not find the symbol %s(type:%d)\n", name, type);
	}
	else if (symbol->type == type) {
		/* hash hit, find the right type */
	}
	else {
		/* hash conflict */
		while (symbol->type != type && symbol != NULL) {
			symbol = symbol->next;
		}
	}
	return symbol;
}

int symbol_insert (char *name, int type, void *attr)
{
	assert (name != NULL);
	symbol_t *s = symbol_table[str_hash (name)];
	symbol_t *new_symbol = (symbol_t *) malloc (sizeof (symbol_t));
	new_symbol->name = strdup (name);
	new_symbol->type = type;
	new_symbol->attr.default_type = attr;
	new_symbol->next = NULL;

	DBG ("In %s, name=%s, type=%d, hash value=%d\n", __FUNCTION__, name, type,
		 str_hash (name));
	if (s == NULL) {			/* blank slot */
		symbol_table[str_hash (name)] = new_symbol;
	}
	else {						/* conflict */
		while (s->next != NULL)
			s = s->next;
		s->next = new_symbol;
	}
	return 0;
}
