// <!!-*-C++-*- file: directory.h --->
// <!!-------------------------------------------------------------------------->
// <!! Copyright (C) 1997 Dietmar Kuehl >
// <!!   Universitaet Konstanz, Germany, Lehrstuhl fuer praktische Informatik I >
// <!!>
// <!! Permission to use, copy, modify, distribute and sell this >
// <!! software for any purpose is hereby granted without fee, provided >
// <!! that the above copyright notice appears in all copies and that >
// <!! both that copyright notice and this permission notice appear in >
// <!! supporting documentation. The Universitaet Konstanz and the >
// <!! Lehrstuhl fuer praktische Informatik I make no representations >
// <!! about the suitability of this software for any purpose. It is >
// <!! provided "as is" without express or implied warranty. >
// <!!-------------------------------------------------------------------------->

// Author: Dietmar Kühl dietmar.kuehl@uni-konstanz.de www.informatik.uni-konstanz.de/~kuehl
// Title:  Declaration for an iterator traversing the entries of a directory

//------------------------------------------------------------------------------

#if !defined(DIRECTORY_H)
#define DIRECTORY_H

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <iterator>

#ifdef __STL_USE_NAMESPACES
using namespace __STD;
#endif

// -----------------------------------------------------------------------------
// The basic building block: An input iterator used to iterate over
// the entries of a directory. This iterator only provides access to
// the names of the entries which are returned from code(operator*)()
// as code(string)s.

class dir_it_rep;

class dir_it :
#ifndef __STL_USE_NAMESPACES
    public input_iterator<string, int>
#else
    public iterator<input_iterator_tag,string,int,string*,string&>
#endif
{
  public:
    // --------------------------------------------------------------------
    // default ctor: creates an iterator suitable as past the end indicator
    dir_it() :
	i_rep(0)
      { }
    explicit dir_it( const string &);
    dir_it(const dir_it& it);
    ~dir_it();

    dir_it &operator= (const dir_it& it);

    string operator* () const { return i_value; }

    dir_it& operator++ ();
    dir_it operator++ (int) { dir_it rc(*this); operator++(); return rc; }

    bool operator== (const dir_it& it) const;
    bool operator!= (const dir_it& it) const { return !operator== (it); }

  private:
    dir_it_rep *i_rep;    // representation for the next value
    string     i_value;   // the current value
};

//--------------------------------------------------------------------------

#endif /* DIRECTORY_H */
