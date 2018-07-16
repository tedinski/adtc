#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "tree.h"
#include "buffers.h"

typedef unsigned int idx;
const int NO_IDX = UINT_MAX;

struct tree_root {
	struct buffer buf;
	idx rt;
};
struct tree_ref {
	idx index;
};


struct tree_ref_any {
	enum { TREE_NODE, TREE_LEAF } type;
};
struct tree_ref_node {
	struct tree_ref_any header;
	idx left;
	idx right;
};
struct tree_ref_leaf {
	struct tree_ref_any header;
	int value;
};

// internal apis
idx tree_ref_copy(struct tree_root *target, struct buffer *buf, idx index);
idx tree_node_copy(struct tree_root *target, struct buffer *buf, struct tree_ref_node *orig);
idx tree_leaf_copy(struct tree_root *target, struct buffer *buf, struct tree_ref_leaf *orig);


struct tree_root *tree_new()
{
	struct tree_root *tree = malloc(sizeof(*tree));
	
	buffer_init(&tree->buf, 3, 4*1024);
	tree->rt = NO_IDX;
	
	return tree;
}
void tree_root_set(struct tree_root *root, struct tree_ref ref)
{
	root->rt = ref.index;
}
struct tree_root *tree_copy(struct tree_root *orig)
{
	struct tree_root *tree = tree_new();
	if(orig->rt != NO_IDX) {
		tree_root_set(tree, (struct tree_ref){ tree_ref_copy(tree, &orig->buf, orig->rt) });
	}
	return tree;
}
void tree_delete(struct tree_root *root)
{
	buffer_destroy(&root->buf);
	free(root);
}

struct tree_ref tree_node_new(struct tree_root *root)
{
	struct tree_ref_node *ref;
	struct tree_ref ref_idx;
	buffer_alloc(&root->buf, sizeof(*ref), (void**)&ref, &ref_idx.index);
	
	ref->header.type = TREE_NODE;
	ref->left = NO_IDX;
	ref->right = NO_IDX;
	return ref_idx;
}
void tree_node_setl(struct tree_root *root, struct tree_ref ref_orig, struct tree_ref l)
{
	struct tree_ref_node *ref = buffer_index(&root->buf, ref_orig.index);
	assert(ref->header.type == TREE_NODE);
	ref->left = l.index;
}
void tree_node_setr(struct tree_root *root, struct tree_ref ref_orig, struct tree_ref r)
{
	struct tree_ref_node *ref = buffer_index(&root->buf, ref_orig.index);
	assert(ref->header.type == TREE_NODE);
	ref->right = r.index;
}

struct tree_ref tree_leaf_new(struct tree_root *root, int v)
{
	struct tree_ref_leaf *ref;
	struct tree_ref ref_idx;
	buffer_alloc(&root->buf, sizeof(*ref), (void**)&ref, &ref_idx.index);
	
	ref->header.type = TREE_LEAF;
	ref->value = v;
	return ref_idx;
}



idx tree_ref_copy(struct tree_root *target, struct buffer *buf, idx index)
{
	struct tree_ref_any *ref = buffer_index(buf, index);
	if(ref->type == TREE_NODE) {
		return tree_node_copy(target, buf, (struct tree_ref_node *)ref);
	} else if(ref->type == TREE_LEAF) {
		return tree_leaf_copy(target, buf, (struct tree_ref_leaf *)ref);
	} else {
		abort();
	}
}
idx tree_node_copy(struct tree_root *target, struct buffer *buf, struct tree_ref_node *orig)
{
	struct tree_ref ref = tree_node_new(target);
	
	idx l = tree_ref_copy(target, buf, orig->left);
	idx r = tree_ref_copy(target, buf, orig->right);
	
	tree_node_setl(target, ref, (struct tree_ref){l});
	tree_node_setr(target, ref, (struct tree_ref){r});
	
	return ref.index;
}
idx tree_leaf_copy(struct tree_root *target, struct buffer *buf, struct tree_ref_leaf *orig)
{
	return tree_leaf_new(target, orig->value).index;
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

