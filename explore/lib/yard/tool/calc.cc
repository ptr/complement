#include <yard/yard.h>
#include <iostream>
#include <cstdlib>

using namespace yard;

int main()
{
    BTree tree;
    tree.init_empty("/tmp/btree_sizes", 4096);

    int i = 0;
    int last_delta_height = tree.get_delta_height();
    while ((1 + tree.get_delta_height()) <= 2)
    {
        BTree::key_type key = xmt::uid();

        block_coordinate coord;
        coord.address= rand();
        coord.size = rand();

        BTree::coordinate_type coordinate = tree.lookup(key);

        tree.insert(coordinate, key, coord);

        if (last_delta_height != tree.get_delta_height())
        {
            std::cout << i << std::endl;
        }

        ++i;
        last_delta_height = tree.get_delta_height();
    }

    return 0;
}
