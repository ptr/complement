// -*- C++ -*- Time-stamp: <2011-02-15 14:07:58 ptr>

/*
 * Copyright (c) 2010-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "yard_test.h"

#include <yard/yard.h>

using namespace std;

int EXAM_IMPL(yard_block_test::block_type_lookup)
{
    using namespace yard;

    BTree::block_type block;
    block.set_flags(BTree::block_type::leaf_node);

    BTree::key_type key;
    for (int i = 0; i < 50; ++i)
    {
        key.u.l[0] = 3*i;
        key.u.l[1] = 0;

        BTree::block_coordinate coord;
        coord.address = i;
        coord.size = 2*i;

        block.insert(key, coord);
    }

    for (int i = 0; i < 50; ++i)
    {
        key.u.l[0] = 3*i;
        key.u.l[1] = 0;

        BTree::block_type::const_iterator it = block.find(key);

        EXAM_REQUIRE(it != block.end());
        EXAM_CHECK(it->first == key);
        EXAM_CHECK(it->second.address == i);
        EXAM_CHECK(it->second.size == 2*i);
    }

    for (int i = 0; i < 50; ++i)
    {
        key.u.l[0] = 3*i + 1;
        key.u.l[1] = 0;

        BTree::block_type::const_iterator it = block.find(key);

        EXAM_REQUIRE(it == block.end());
    }

    return EXAM_RESULT;
}

int EXAM_IMPL(yard_block_test::block_type_route)
{
    using namespace yard;

    BTree::block_type block;

    BTree::key_type key;
    for (int i = 0; i < 50; ++i)
    {
        key.u.l[0] = 3*i;
        key.u.l[1] = 0;

        BTree::block_coordinate coord;
        coord.address = i;
        coord.size = 2*i;

        block.insert(key, coord);
    }

    for (int i = 0; i < 50; ++i)
    {
        key.u.l[0] = 3*i + 1;
        key.u.l[1] = 0;

        BTree::block_type::const_iterator it = block.route(key);

        EXAM_REQUIRE(it != block.end());
        EXAM_CHECK(it->first <= key);
        if ( ++it != block.end() ) {
          EXAM_CHECK(it->first > key);
        }
    }

    return EXAM_RESULT;
}

int EXAM_IMPL(yard_block_test::block_type_divide)
{
    using namespace yard;

    int sizes[3];
    sizes[0] = 1000;
    sizes[1] = 759;
    sizes[2] = 4096;

    for (int k = 0; k < 3; ++k)
    {
        BTree::block_type block;
        block.set_block_size(sizes[k]);
        block.set_flags(BTree::block_type::leaf_node);

        typedef vector<pair<BTree::key_type, BTree::block_coordinate> > data_type;
        data_type data;

        int i = 0;
        while (!block.is_overfilled())
        {
            BTree::key_type key;
            key.u.l[0] = 3*i;
            key.u.l[1] = 0;

            BTree::block_coordinate coord;
            coord.address = i;
            coord.size = 2*i;

            data.push_back(make_pair(key, coord));
            block.insert(key, coord);

            ++i;
        }

        BTree::block_type new_block;
        block.divide(new_block);
        pair<BTree::key_type, BTree::key_type> delimiters;

        EXAM_REQUIRE(block.begin() != block.end());
        EXAM_REQUIRE(new_block.begin() != new_block.end());

        delimiters.first = (--block.end())->first;
        delimiters.second = new_block.begin()->first;

        EXAM_REQUIRE(delimiters.first < delimiters.second);

        for (data_type::const_iterator it = data.begin();
             it != data.end(); ++it)
        {
            EXAM_REQUIRE((it->first <= delimiters.first) || (it->first >= delimiters.second));

            BTree::block_type::const_iterator result;
            BTree::block_type::const_iterator none;

            if (it->first <= delimiters.first)
            {
                result = block.find(it->first);
                none = block.end();
            }

            if (it->first >= delimiters.second)
            {
                result = new_block.find(it->first);
                none = new_block.end();
            }

            EXAM_REQUIRE(result != none);
            EXAM_CHECK(result->first == it->first);
            EXAM_CHECK(result->second.address == it->second.address);
            EXAM_CHECK(result->second.size == it->second.size);
        }
    }

    return EXAM_RESULT;
}

bool block_equal(const yard::BTree::block_type& left, const yard::BTree::block_type& right)
{
    using namespace yard;

    if (left.is_root() != right.is_root())
        return false;
    if (left.is_leaf() != right.is_leaf())
        return false;

    BTree::block_type::const_iterator l = left.begin();
    BTree::block_type::const_iterator r = right.begin();

    while ((l != left.end()) && (r != right.end()))
    {
        if (l->first != r->first)
            return false;

        if (l->second.address != r->second.address)
            return false;

        if ((left.is_leaf()) && (l->second.size != r->second.size))
            return false;

        ++l;
        ++r;
    }

    if ((l == left.end()) && (r == right.end()))
        return true;
    else
        return false;
}

int EXAM_IMPL(yard_block_test::pack_unpack)
{
    using namespace yard;

    BTree::block_type block[4];
    for (int i = 0; i < 4; ++i)
        block[i].set_block_size(4096);

    block[0].set_flags(BTree::block_type::leaf_node);
    block[1].set_flags(BTree::block_type::leaf_node | BTree::block_type::root_node);
    block[2].set_flags(BTree::block_type::root_node);

    for (int i = 0; i < 50; ++i)
    {
        BTree::key_type key;
        key.u.l[0] = 3*i;
        key.u.l[1] = 0;

        BTree::block_coordinate coord;
        coord.address = i * 4096;
        coord.size = 2*i;

        for (int k = 0; k < 4; ++k)
            block[k].insert(key, coord);
    }

    for (int k = 0; k < 4; ++k)
    {
        stringstream ss(ios_base::out|ios_base::in);
        block[k].pack(ss);

        BTree::block_type new_block;
        new_block.set_block_size(4096);
        ss.seekg(0, ios_base::beg);
        new_block.unpack(ss);

        EXAM_REQUIRE(block_equal(block[k], new_block));
    }

    return EXAM_RESULT;
}
