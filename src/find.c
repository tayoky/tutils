#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <stdio.h>
#include <pwd.h>
#include <tutils.h>


CMD_NOPT(find, "find [PATH...] [EXPR...]\n"
"find files\n");

static char **ptr;
static int ret = 0;
static int has_action = 0;

typedef struct node {
	int type;
	struct node *left;
	struct node *right;
	union {
		long number;
		char *str;
		uid_t uid;
		gid_t gid;
		mode_t f_type;
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

typedef struct file {
	const char *path;
	const char *name;
	struct stat st;
} file_t;

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
#define ARG_UID     7

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
	PRIM("-user"   , NODE_USER, ARG_UID),
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

static int parse_arg(node_t *node, primary_t *primary) {
	if (primary->argtype == ARG_NOARG) {
		return 0;
	}
	char *str = get_str();
	if (!str) {
		error("expected argument to '%s'", primary->name);
		return -1;
	}
	switch (primary->argtype) {
	case ARG_NUMBER:;
		char *end;
		node->number = strtol(str, &end, 10);
		if (end == str || *end) {
			error("invalid number '%s' to '%s'", str, primary->name);
			return -1;
		}
		break;
	case ARG_STRING:
		node->str = str;
		break;
	case ARG_TYPE:
		if (!str[0] || str[1]) {
invalid_type:
			error("invalid type '%s' to '%s'", str, primary->name);
			return -1;
		}
		switch (str[0]) {
		case 'b':
			node->f_type = S_IFBLK;
			break;
		case 'c':
			node->f_type = S_IFCHR;
			break;
		case 'd':
			node->f_type = S_IFDIR;
			break;
		case 'l':
			node->f_type = S_IFLNK;
			break;
		case 'p':
			node->f_type = S_IFIFO;
			break;
		case 'f':
			node->f_type = S_IFREG;
			break;
		case 's':
			node->f_type = S_IFSOCK;
			break;
		default:
			goto invalid_type;
		}
		break;
	case ARG_UID:;
		uid_t uid = str2uid(str);
		if (uid < 0) {
			error("invalid username or uid to '%s'", primary->name);
			return -1;
		}
		node->uid = uid;
		break;
	}
	return 0;
}

static node_t *parse_primary(void) {
	char *name = get_str();
	if (!name) return NULL;
	for (size_t i=0; i<arraylen(primaries); i++) {
		if (strcmp(name, primaries[i].name)) continue;
		node_t *node = new_node(primaries[i].type);
		if (node->type == NODE_PRINT || node->type == NODE_EXEC || node->type == NODE_OK) {
			has_action = 0;
		}
		if (parse_arg(node, &primaries[i]) < 0) {
			free_node(node);
			return NULL;
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

static node_t *parse_exprs(void) {
	if (!peek_str()) {
		// if nothing just print
		return new_node(NODE_PRINT);
	}
	node_t *node = parse_or_list();
	if (!node) return NULL;
	if (!has_action) {
		// by default add a print
		node_t *print = new_node(NODE_PRINT);
		node_t *and = new_node(NODE_AND);
		and->left = node;
		and->right = print;
		return and;
	}
	return node;
}

// return 1 if true else 0
static int check_node(file_t *file, node_t *node) {
	switch (node->type) {
	case NODE_NOT:
		return !check_node(file, node->left);
	case NODE_AND:
		if (!check_node(file, node->left)) return 0;
		return check_node(file, node->right);
	case NODE_OR:
		if (check_node(file, node->left)) return 1;
		return check_node(file, node->right);
	case NODE_NAME:
		return glob_match(node->str, file->name);
	case NODE_PATH:
		return glob_match(node->str, file->path);
	case NODE_PRINT:
		puts(file->path);
		return 1;
	case NODE_NOUSER:
		return getpwuid(file->st.st_uid) == NULL;
	case NODE_LINKS:
		return file->st.st_nlink == node->number;
	case NODE_TYPE:
		return (file->st.st_mode & S_IFMT) == node->f_type;
	case NODE_USER:
		return file->st.st_uid == node->uid;
	default:
		error("TODO : unimplemented node");
		ret = 1;
		return 0;
	}
}

static char *get_basename(const char *path) {
	// this should work with all basename
	char *first_dup = strdup(path);
	char *result = strdup(basename(path));
	free(first_dup);
	return result;
}

static int do_find(const char *path, node_t *node) {
	file_t file = {
		.path = path,
		.name = get_basename(path),
	};
	if (lstat(path, &file.st) < 0) {
		perror(path);
		ret = 1;
		return -1;
	}
	check_node(&file, node);

	if (!S_ISDIR(file.st.st_mode)) {
		return 0;
	}

	// if is a dir need to be recursive
	DIR *dir = opendir(path);
	if (!dir) {
		return 0;
	}
	struct dirent *entry;
	while ((entry = readdir(dir))) {
		// ignore "." and ".."
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
			continue;
		}
		char full_path[strlen(path) + strlen(entry->d_name) + 2];
		if (path[0] && path[strlen(path)-1] == '/') {
			// aready has trailling "/"
			sprintf(full_path, "%s%s", path, entry->d_name);
		} else {
			sprintf(full_path, "%s/%s", path, entry->d_name);
		}
		do_find(full_path, node);
	}
	closedir(dir);
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
	node_t *root = parse_exprs();
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
	return ret;
}
