// -*- C++ -*- Time-stamp: <2011-02-15 14:07:47 ptr>

/*
 * Copyright (c) 2010-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "yard_test.h"

#include <yard/yard.h>
#include "../pack.h"

#include <fstream>
#include <unistd.h>

using namespace std;

int EXAM_IMPL(yard_btree_test::btree_basic)
{
    using namespace yard;

    int sizes[3];
    sizes[0] = 756;
    sizes[1] = 4096;
    sizes[2] = 4*4096;

    for (int k = 0; k < 3; ++k)
    {
        BTree tree;
        tree.open("/tmp/btree", ios_base::in | ios_base::out | ios_base::trunc, sizes[k]);

        const int count = 1050;
        BTree::block_coordinate coord;
        BTree::coordinate_type coordinate;

        for (int i = 0; i < count; ++i)
        {
            BTree::key_type key;
            key.u.l[0] = 3*i;
            key.u.l[1] = 0;

            coordinate = tree.lookup(coordinate, key);

            coord.address = i;
            coordinate = tree.insert(coordinate, key, coord);
            tree.clear_cache();
        }

        for (int i = 0; i < count; ++i)
        {
            BTree::key_type key;
            key.u.l[0] = 3*i;
            key.u.l[1] = 0;

            BTree::coordinate_type coordinate = tree.lookup(key);

            const BTree::block_type& block = tree.get(coordinate);
            BTree::block_type::const_iterator node = block.find(key);

            EXAM_REQUIRE(node != block.end());
            EXAM_CHECK(node->first == key);
            EXAM_CHECK(node->second.address == i);
        }
    }

    unlink( "/tmp/btree" );

    return EXAM_RESULT;
}

int EXAM_IMPL(yard_btree_test::btree_random)
{
    using namespace yard;

    int sizes[1];
    sizes[0] = 800;

    for (int k = 0; k < 1; ++k)
    {
        BTree tree;
        tree.open("/tmp/btree", ios_base::in | ios_base::out | ios_base::trunc, sizes[k]);

        const int count = 20000;

        vector<pair<BTree::key_type, BTree::block_coordinate> > added_entries;
        BTree::block_coordinate coord;
        BTree::key_type key;

        for (int i = 0; i < count; ++i)
        {
            key = xmt::uid();

            coord.address= rand();
            coord.size = rand();

            BTree::coordinate_type coordinate = tree.lookup(key);

            tree.insert(coordinate, key, coord);

            added_entries.push_back(make_pair(key, coord));
        }

        int i = 0;
        for (vector<pair<BTree::key_type, BTree::block_coordinate> >::const_iterator entry_iterator = added_entries.begin();
             entry_iterator != added_entries.end();
             ++entry_iterator)
        {
            BTree::coordinate_type coordinate = tree.lookup(entry_iterator->first);

            const BTree::block_type& block = tree.get(coordinate);
            BTree::block_type::const_iterator data= block.find(entry_iterator->first);
            EXAM_CHECK(data->second.address == entry_iterator->second.address);
            EXAM_CHECK(data->second.size == entry_iterator->second.size);

            ++i;
        }
    }

    unlink( "/tmp/btree" );

    return EXAM_RESULT;
}

int EXAM_IMPL(yard_btree_test::btree_init_existed)
{
    const int count = 200000;
    using namespace yard;

    int sizes[1];
    sizes[0] = 4*4096;

    for (int k = 0; k < 1; ++k)
    {
      vector<BTree::block_coordinate> added_entries;
        {
            BTree tree;
            tree.open("/tmp/btree", ios_base::in | ios_base::out | ios_base::binary | ios_base::trunc, sizes[k]);
            BTree::key_type key;
            BTree::block_coordinate coord;

            for (int i = 0; i < count; ++i)
            {
                key.u.l[0] = 3*i;
                key.u.l[1] = 0;

                coord.address= rand();
                coord.size = rand();

                BTree::coordinate_type coordinate = tree.lookup(key);

                tree.insert(coordinate, key, coord);

                added_entries.push_back(coord);
            }
        }

        {
            BTree tree;
            tree.open("/tmp/btree", ios_base::in | ios_base::out | ios_base::binary);

            int i = 0;
            BTree::key_type key;
            for (vector<BTree::block_coordinate>::const_iterator entry_iterator = added_entries.begin();
                 entry_iterator != added_entries.end();
                 ++entry_iterator)
            {
                key.u.l[0] = 3*i;
                key.u.l[1] = 0;
                BTree::coordinate_type coordinate = tree.lookup(key);

                const BTree::block_type& block = tree.get(coordinate);
                BTree::block_type::const_iterator data= block.find(key);
                EXAM_CHECK(data->second.address == entry_iterator->address);
                EXAM_CHECK(data->second.size == entry_iterator->size);

                ++i;
            }
        }
    }

    unlink( "/tmp/btree" );

    return EXAM_RESULT;
}
