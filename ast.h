#ifndef __AST_H__
#define __AST_H__

#include "tree.h"

typedef struct node
{
	int type;
	char *name;
	struct node *sibling;
	struct node *child;
} node_t;

typedef struct level
{
	int depth;

} level_t;

#endif
