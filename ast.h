#ifndef __AST_H__
#define __AST_H__
typedef struct node
{
	int type;
	char *name;
	struct node *sibling;
	struct node *child;
} node_t;
#endif
