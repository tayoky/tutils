#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <tutils.h>


CMD_NOPT(find, "find [PATH...] [EXPR...]\n"
"find files\n");

static char **ptr;

typedef struct node {
	int type;
	struct node *left;
	struct node *right;
	union {
		long number;
		char *str;
	};
} node_t;

#define NODE_NAME    1
#define NODE_PATH    2
#define NODE_NOUSER  3
#define NODE_NOGROUP 4
#define NODE_XDEV    5
#define NODE_PRUNE   6
#define NODE_PERM    7
#define NODE_TYPE    8
#define NODE_LINKS   9
#define NODE_USER    10
#define NODE_GROUP   11
#define NODE_SIZE    12
#define NODE_ATIME   13
#define NODE_CTIME   14
#define NODE_MTIME   15
#define NODE_EXEC    16
#define NODE_OK      17
#define NODE_PRINT   18
#define NODE_NEWER   19
#define NODE_DEPTH   20
#define NODE_NOT     21
#define NODE_AND     22
#define NODE_OR      23

static node_t *parse_or_list(void);

typedef struct primary {
	const char *name;
	int type;
	int argtype;
} primary_t;

#define PRIM(_name, _type, _argtype) {.name = _name, .type = _type, .argtype = _argtype}

#define ARG_NOARG   0
#define ARG_STRING  1
#define ARG_NUMBER  2
#define ARG_MODE    3
#define ARG_TYPE    4
#define ARG_SIZE    5
#define ARG_UTIL    6

static primary_t primaries[] = {
	PRIM("-name"   , NODE_NAME  , ARG_STRING),
	PRIM("-path"   , NODE_PATH  , ARG_STRING),
	PRIM("-nouser" , NODE_NOUSER, ARG_NOARG),
	PRIM("-nogroup", NODE_NOGROUP, ARG_NOARG),
	PRIM("-xdev"   , NODE_XDEV, ARG_NOARG),
	PRIM("-prune"  , NODE_PRUNE, ARG_NOARG),
	PRIM("-perm"   , NODE_PERM, ARG_MODE),
	PRIM("-type"   , NODE_TYPE, ARG_TYPE),
	PRIM("-links"  , NODE_LINKS, ARG_NUMBER),
	PRIM("-user"   , NODE_USER, ARG_STRING),
	PRIM("-group"  , NODE_GROUP, ARG_STRING),
	PRIM("-size"   , NODE_SIZE, ARG_SIZE),
	PRIM("-atime"  , NODE_ATIME, ARG_NUMBER),
	PRIM("-ctime"  , NODE_CTIME, ARG_NUMBER),
	PRIM("-mtime"  , NODE_MTIME, ARG_NUMBER),
	PRIM("-exec"   , NODE_EXEC, ARG_UTIL),
	PRIM("-ok"     , NODE_OK, ARG_UTIL),
	PRIM("-print"  , NODE_PRINT, ARG_NOARG),
	PRIM("-newer"  , NODE_NEWER, ARG_STRING),
	PRIM("-depth"  , NODE_DEPTH, ARG_NOARG),

};

static char *peek_str(void) {
	return *ptr;
}

static char *get_str(void) {
	char *str = *ptr;
	ptr++;
	return str;
}

static node_t *new_node(int type) {
	node_t *node = malloc(sizeof(node_t));
	memset(node, 0, sizeof(node_t));
	node->type = type;
	return node;
}

static void free_node(node_t *node) {
	if (!node) return;
	free_node(node->left);
	free_node(node->right);
	free(node);
}

static long parse_int(void) {
	char *str = get_str();
	char *end;
	long num = strtol(str, &end, 10);
	if (str == end || *end) {
		error("invalid number : '%s'", str);
		exit(1);
	}
	return num;
}

static node_t *parse_primary(void) {
	char *name = get_str();
	if (!name) return NULL;
	for (size_t i=0; i<arraylen(primaries); i++) {
		if (strcmp(name, primaries[i].name)) continue;
		node_t *node = new_node(primaries[i].type);
		switch (primaries[i].argtype) {
		case ARG_NUMBER:
			node->number = parse_int();
			break;
		case ARG_STRING:
			node->str = get_str();
			if (!node->str) {
				error("expected string argument to '%s'", name);
				free_node(node);
				return NULL;
			}
			break;
		}
		return node;
	}
	error("unknow operand '%s'", name);
	return NULL;
}

static node_t *parse_expr(void) {
	char *current = peek_str();
	if (!current) {
		error("expected expression");
		return NULL;
	}
	if (!strcmp(current, "(")) {
		get_str();
		node_t *node = parse_or_list();
		if (!node) return NULL;
		current = get_str();
		if (!current || strcmp(current, ")")) {
			free_node(node);
			error("non matching ')'");
			return NULL;
		}
		return node;
	} else if (!strcmp(current, "!")) {
		get_str();
		node_t *child = parse_expr();
		if (!child) return NULL;
		node_t *node = new_node(NODE_NOT);
		node->left = child;
		return node;
	}

	node_t *node = parse_primary();
	
	return node;
}

static node_t *parse_and_list(void) {
	node_t *left = parse_expr();
	if (!left) return NULL;

	char *current;

	while ((current = peek_str())) {
		if (!strcmp(current, "-or") || !strcmp(current, "-o") || !strcmp(current, ")")) {
			break;
		}
		if (!strcmp(current, "-and") || !strcmp(current, "-a")) {
			get_str();
		}
		node_t *right = parse_expr();
		if (!right) {
			free_node(left);
			return NULL;
		}
		node_t *node = new_node(NODE_AND);
		node->left = left;
		node->right = right;
		left = node;
	}
	return left;
}

static node_t *parse_or_list(void) {
	node_t *left = parse_and_list();
	if (!left) return NULL;

	char *current;

	while ((current = peek_str())) {
		if (!strcmp(current, ")")) {
			break;
		}
		if (!strcmp(current, "-or") || !strcmp(current, "-o")) {
			get_str();
		} else {
			// how did we get here
			error("unpossible error");
		}
		node_t *right = parse_and_list();
		if (!right) {
			free_node(left);
			return NULL;
		}
		node_t *node = new_node(NODE_OR);
		node->left = left;
		node->right = right;
		left = node;
	}
	return left;
}

static int do_find(const char *path, node_t *node) {
	// TODO
	error("TODO : implement find");
	return 0;
}

static int find_main(int argc, char **argv) {
	ptr = argv;
	while (peek_str()) {
		int c = peek_str()[0];
		if (c == '-' || c == '(' || c == '!') {
			break;
		}
		get_str();
	}
	size_t path_count = ptr - argv;
	node_t *root = parse_or_list();
	if (!root) {
		return 1;
	}
	if (path_count > 0) {
		for (size_t i=0; i<path_count; i++) {
			do_find(argv[i], root);
		}
	} else {
		do_find(".", root);
	}
	free_node(root);
	return 0;
}
