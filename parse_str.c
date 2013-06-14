/*
 * scanfstr.c:
 *
 * Copyright (C) 2013 eJim Lee <lyjforjob@gmail.com>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <assert.h>
#include "tree.h"
#include "ast.h"

/**
 * @brief strlist_free free a string list.
 *
 * @param strlist      string list.
 * @param n            the number of string.
 */
void strlist_free(char **strlist, int n)
{
	int i;
	for(i = 0; i < n; i++) {
		free(strlist[i]);
	}
	free(strlist);
}

/**
 * @brief substring pick a sub string from main string.
 *
 * @param str       the main string.
 * @param so        the start of sub string.
 * @param eo        the next char of sub string end.
 *
 * @return          the sub string.
 */
char* substring(const char *str, int so, int eo)
{
	int len = eo - so;
	if(len <= 0)
		return NULL;
	char *tmp = malloc(len + 1);
	tmp = strncpy(tmp, str + so, len);
	tmp[len] = '\0';
	return tmp;
}

/**
 * @brief scanfstr get format string number and type from string, like printf.
 *
 * @param str      the string.
 * @param typelist a list of format string type.
 *
 * @return         a number of format string.
 */
int scanfstr(const char *str, char ***typelist)
{
	if(str == NULL && typelist == NULL)
		return -1;

	const int    cflags    = REG_EXTENDED;
	const size_t nmatch    = 3;
	const int    buflen    = 1024;
	const char  *pattern   = "%[0-9]*[.]?[0-9]*((h{0,2}|l{0,2})[#]?[diufscpeExXgGocnm]{1})";
	const int    typeindex = 1;
	const int    endindex  = 2;

	int rt, n = 0;
	char **tmplist = NULL;
	char errbuf[buflen];
	regmatch_t pm[nmatch];
	regex_t reg;

	rt = regcomp(&reg, pattern, cflags);
	if(rt != 0) {
		regerror(rt, &reg, errbuf, sizeof(errbuf));
		fprintf(stderr, "%s:\npattern: '%s'\n", errbuf, pattern);
		return -1;
	}

	while(regexec(&reg, str, nmatch, pm, 0) != REG_NOMATCH) {
		char **t = realloc(tmplist, (++n) * sizeof(char*));
		if(t == NULL) {
			strlist_free(tmplist, n - 1);
			return -1;
		}
		tmplist = t;
		tmplist[n - 1] = substring(str, pm[typeindex].rm_so, pm[typeindex].rm_eo);
		str = str + pm[endindex].rm_eo;
	}
	*typelist = tmplist;

	regfree(&reg);
	return n;
}

struct log_args* parse_log(tree_t* node) {
	assert(node != NULL);
	printf("In %s, line = %d, log fromat: %s\n",
			__func__, __LINE__, node->log.format);

	struct log_args* log = (struct log_args*)gdml_zmalloc(sizeof(struct log_args));
	char** typelist = NULL;
	int arg_num = 0;

	log->argc = scanfstr(node->log.format, &typelist);
	arg_num = get_list_num(node->log.args);

	if ((log->argc) > arg_num) {
		printf("argc: %d, arg_num: %d\n", log->argc, arg_num);
		fprintf(stderr, "warning: too few arguments for format\n");
		/* TODO: handle the error */
		exit(-1);
	}
	/* FIXME: we should refer to printf? */

	return log;
}

#ifdef SCANFSTRING_DEBUG
int main(void)
{
	char *str = "a = %i, b = %d, c = %9.2f, d = %8.3hd, e = %7.4hhd";
	char **typelist;
	int n = scanfstr(str, &typelist);
	int i;
	if(n > 0) {
		for(i = 0; i < n; i++) {
			printf("%s\n", typelist[i]);
		}
	}
	return 0;
}
#endif

