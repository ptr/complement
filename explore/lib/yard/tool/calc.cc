#include <yard/yard.h>
#include <iostream>
#include <cstdlib>

using namespace yard;

int main()
{
    BTree tree;
    tree.init_empty("/tmp/btree_sizes", 4096);

    int i = 0;
    int last_delta_height = 1;
    while (last_delta_height <= 2)
    {
        BTree::key_type key = xmt::uid();

        block_coordinate coord;
        coord.address= rand();
        coord.size = rand();

        BTree::coordinate_type coordinate = tree.lookup(key);
        int height = coordinate.size();

        tree.insert(coordinate, key, coord);

        if (last_delta_height != height)
        {
            std::cout << (i - 1) << std::endl;
            last_delta_height = height;
        }

        ++i;
    }

    return 0;
}
