#include <assert.h>
#include <stdio.h>

#include "huff.h"

static void
print_indent(unsigned int indent)
{
        unsigned int i;
        for (i = 0; i < indent; i++) {
                printf("   ");
        }
}

static void
print_node(const struct hufftree *tree, const struct hnode *n,
           unsigned int indent)
{
        if (n->count == 0) {
                return;
        }
        bool is_leaf = n - tree->nodes < HUFF_NSYMS;
        print_indent(indent);
        if (is_leaf) {
                printf("leaf %02x (count %zu, %u bits)\n",
                       (huff_sym_t)(n - tree->nodes), n->count,
                       n->u.leaf.encoded_nbits);
        } else {
                printf("inner (count %zu)\n", n->count);
                unsigned int i;
                for (i = 0; i < 2; i++) {
                        print_node(tree, n->u.inner.children[i], indent + 1);
                }
        }
}

void
huff_dump_tree(const struct hufftree *t)
{
        unsigned int indent = 1;
        const struct hnode *n = &t->nodes[HUFF_NSYMS * 2 - 2];
        print_node(t, n, indent);
}

static void
print_table_entry(const uint8_t *table, size_t size, const uint8_t *entry,
                  unsigned int indent)
{
        assert(table <= entry);
        assert(entry + 2 <= table + size);
        uint8_t f = entry[0];
        unsigned int i;
        bool omitted = entry + 2 == table + size;
        print_indent(indent);
        unsigned int entryidx = (entry - table) / 3;
        assert(&table[entryidx * 3] == entry);
        if (omitted) {
                printf("# entry [%03x] raw %02x %02x\n", entryidx, entry[0],
                       entry[1]);
        } else {
                printf("# entry [%03x] raw %02x %02x %02x\n", entryidx,
                       entry[0], entry[1], entry[2]);
        }
        for (i = 0; i < 2 - omitted; i++) {
                uint8_t cf = (f >> (i * 4)) & 0x0f;
                bool leaf = (cf & 1) != 0;
                uint16_t v = ((uint16_t)(cf >> 1) << 8) | entry[i + 1];
                print_indent(indent);
                printf("[%u] %s %03x\n", i, leaf ? "leaf " : "inner", v);
                if (!leaf) {
                        assert(entryidx < v);
                        const uint8_t *next = &table[3 * v];
                        print_table_entry(table, size, next, indent + 1);
                }
        }
        if (omitted) {
                print_indent(indent);
                printf("[%u] %s\n", 1, "omitted");
        }
}

void
huff_dump_table(const uint8_t *table, size_t size)
{
        if (size == 0) {
                return;
        }
        print_table_entry(table, size, table, 1);
}
