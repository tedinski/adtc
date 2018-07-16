#include <stdlib.h>
#include <assert.h>

#include "tree.h"

struct tree_root {
	struct tree_ref_any *root;
};
struct tree_ref {
	struct tree_ref_any *ref;
};

struct tree_ref_any {
	enum { TREE_NODE, TREE_LEAF } type;
};
struct tree_ref_node {
	struct tree_ref_any header;
	struct tree_ref_any *left;
	struct tree_ref_any *right;
};
struct tree_ref_leaf {
	struct tree_ref_any header;
	int value;
};

// internal apis
struct tree_ref_any *tree_ref_copy(struct tree_root *target, struct tree_ref_any *ref);
void tree_ref_delete(struct tree_ref_any *ref);
void tree_node_delete(struct tree_ref_node *ref);
struct tree_ref_any *tree_node_copy(struct tree_root *target, struct tree_ref_node *orig);
void tree_leaf_delete(struct tree_ref_leaf *ref);
struct tree_ref_any *tree_leaf_copy(struct tree_root *target, struct tree_ref_leaf *orig);

struct tree_root *tree_new()
{
	struct tree_root *tree = malloc(sizeof(*tree));
	
	tree->root = 0;
	return tree;
}
void tree_root_set(struct tree_root *root, struct tree_ref ref)
{
	root->root = ref.ref;
}
struct tree_root *tree_copy(struct tree_root *orig)
{
	struct tree_root *tree = tree_new();
	if(orig->root != 0) {
		tree_root_set(tree, (struct tree_ref){ tree_ref_copy(tree, orig->root) });
	}
	return tree;
}
void tree_delete(struct tree_root *root)
{
	if(root->root != 0) {
		tree_ref_delete(root->root);
	}
	free(root);
}
void tree_ref_delete(struct tree_ref_any *ref)
{
	if(ref->type == TREE_NODE) {
		tree_node_delete((struct tree_ref_node *)ref);
	} else if(ref->type == TREE_LEAF) {
		tree_leaf_delete((struct tree_ref_leaf *)ref);
	} else {
		abort();
	}
}
struct tree_ref_any *tree_ref_copy(struct tree_root *target, struct tree_ref_any *ref)
{
	if(ref->type == TREE_NODE) {
		return tree_node_copy(target, (struct tree_ref_node *)ref);
	} else if(ref->type == TREE_LEAF) {
		return tree_leaf_copy(target, (struct tree_ref_leaf *)ref);
	} else {
		abort();
	}
}



struct tree_ref tree_node_new(struct tree_root *root)
{
	struct tree_ref_node *ref = malloc(sizeof(*ref));
	ref->header.type = TREE_NODE;
	ref->left = 0;
	ref->right = 0;
	return (struct tree_ref){ (struct tree_ref_any *)ref };
}
void tree_node_setl(struct tree_root *root, struct tree_ref ref_orig, struct tree_ref l)
{
	assert(ref_orig.ref->type == TREE_NODE);
	struct tree_ref_node *ref = (struct tree_ref_node *)ref_orig.ref;
	ref->left = l.ref;
}
void tree_node_setr(struct tree_root *root, struct tree_ref ref_orig, struct tree_ref r)
{
	assert(ref_orig.ref->type == TREE_NODE);
	struct tree_ref_node *ref = (struct tree_ref_node *)ref_orig.ref;
	ref->right = r.ref;
}
void tree_node_delete(struct tree_ref_node *ref)
{
	tree_ref_delete(ref->left);
	tree_ref_delete(ref->right);
	free(ref);
}
struct tree_ref_any *tree_node_copy(struct tree_root *target, struct tree_ref_node *orig)
{
	struct tree_ref ref = tree_node_new(target);
	
	struct tree_ref_any *l = tree_ref_copy(target, orig->left);
	struct tree_ref_any *r = tree_ref_copy(target, orig->right);
	
	tree_node_setl(target, ref, (struct tree_ref){l});
	tree_node_setr(target, ref, (struct tree_ref){r});
	
	return ref.ref;
}

struct tree_ref tree_leaf_new(struct tree_root *root_unused, int v)
{
	struct tree_ref_leaf *ref = malloc(sizeof(*ref));
	ref->header.type = TREE_LEAF;
	ref->value = v;
	return (struct tree_ref){ (struct tree_ref_any *)ref };
}
void tree_leaf_delete(struct tree_ref_leaf *ref)
{
	free(ref);
}
struct tree_ref_any *tree_leaf_copy(struct tree_root *target, struct tree_ref_leaf *orig)
{
	return tree_leaf_new(target, orig->value).ref;
}



///// This code needs to know the size of tree_ref, but it should otherwise be unchanged for other benchmarks
struct tree_ref generate(struct tree_root *root, int depth)
{
	if(depth == 1) {
		return tree_leaf_new(root, 0);
	}
	
	struct tree_ref node = tree_node_new(root);
	
	tree_node_setl(root, node, generate(root, depth - 1));
	tree_node_setr(root, node, generate(root, depth - 1));
	
	return node;
}
void generate_into(struct tree_root *root, int depth)
{
	struct tree_ref ref = generate(root, depth);
	tree_root_set(root, ref);
}

