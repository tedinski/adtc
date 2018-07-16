struct tree_root;
struct tree_ref;

struct tree_root *tree_new();
void tree_root_set(struct tree_root *, struct tree_ref);
struct tree_root *tree_copy(struct tree_root *);
void tree_delete(struct tree_root *);

struct tree_ref tree_node_new(struct tree_root *);
void tree_node_setl(struct tree_root *, struct tree_ref, struct tree_ref);
void tree_node_setr(struct tree_root *, struct tree_ref, struct tree_ref);

struct tree_ref tree_leaf_new(struct tree_root *, int);



//unchanged
void generate_into(struct tree_root *root, int depth);
