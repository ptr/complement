// <!!-*-C++-*- file: directory.cc --->
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
// Title:  Implementation of an iterator traversing the entries of a directory

//------------------------------------------------------------------------------

#include <directory.h>

#include <dirent.h>
#include <unistd.h>
#include <assert.h>

// -----------------------------------------------------------------------------
// Objects of class code(dir_it) use a code(dir_it_rep) as internal
// representation (or code(0) if they are invalid): A code(dir_it_rep)
// is basically a reference counted pair of a code(DIR*) and a
// code(dirent*). Note that code(dir_it) is strictly a single pass
// iterator: Copying code(dir_it)s does saves the current position
// only until one of the copies is advanced. Also, copies may get out
// of sync: Only the code(dir_it) used to advance and copies of this
// iterator after the advance operation return the current directory
// entry.

class dir_it_rep
{
  private:
    int        i_ref_count;
    DIR        *i_dir;
    dirent     *i_dirent;

    dir_it_rep( const dir_it_rep &); // no copying
    dir_it_rep& operator= (const dir_it_rep &); // no assigning

  public:
   // ------------------------------------------------------------------
   // ctor: Open the directory (if possible). The representation is not
   // put onto the initial entry of the directory: This is done by the
   // ctor of code(dir_it).
    dir_it_rep( const string& dir ) :  // normal allocation
	i_ref_count(1),
	i_dir(opendir(dir.c_str())),
	i_dirent(0)
      { }      
   // ------------------------------------------------------------------
   // dtor: If the directory was opened successfully, close it here and
   // release the memory set aside for the code(dirent).
    ~dir_it_rep()
      {
	if (i_dir)
	  closedir(i_dir);
      }

    char const *name() { return i_dirent? i_dirent->d_name: 0; }

    void reference() { ++i_ref_count; }
    bool release() { return --i_ref_count == 0; }
    bool get_next();
};

// -----------------------------------------------------------------------------
// The workhorse of the code(dir_it): The function to get the next
// entry from a directory. This function simply calls code(readdir)()
// and stores the result in code(dirent). If the call fails, it
// returns code(false), otherwise, i.e. on success, it returns
// code(true).

bool dir_it_rep::get_next()
{
  if (i_dir != 0){
    i_dirent = readdir(i_dir);
    return i_dirent != 0;
  } else
    return false;
}

// -----------------------------------------------------------------------------
// ctor from string: Creates an iterator for the named directory. If
// it opening the directory fails, the constructed iterator compares
// equal to the past the end iterator immediately.

dir_it::dir_it( const string& dir ):
  i_rep(new dir_it_rep(dir))
{
  if (!i_rep->get_next()) {
    delete i_rep;
    i_rep = 0;
  } else
    i_value = i_rep->name();
}

// -----------------------------------------------------------------------------
// copy ctor: copy the pointer to the representation and increase the
// reference count. Also copy the current name.

dir_it::dir_it( const dir_it& dir ):
  i_rep(dir.i_rep),
  i_value(dir.i_value)
{
  if (i_rep != 0)
    i_rep->reference();
}


// -----------------------------------------------------------------------------
// dtor: release the representation if the reference count drop to
// zero.

dir_it::~dir_it()
{
  if (i_rep != 0 && i_rep->release())
    delete i_rep;
}

// -----------------------------------------------------------------------------
// Assignment operator: Release the own representation and reference
// the representation of the RHS. However, make sure that
// self-assignment does not fail.

dir_it &dir_it::operator= ( const dir_it& it)
{
  if (it.i_rep != 0)
    it.i_rep->reference();
  if (i_rep != 0 && i_rep->release())
    delete i_rep;
  i_rep   = it.i_rep;
  i_value = it.i_value;
  return *this;
}

// -----------------------------------------------------------------------------
// Pre-increment operator: fetch the next directory entry.

dir_it &dir_it::operator++ ()
{
  assert(i_rep != 0);

  if (i_rep->get_next())
    i_value = i_rep->name();
  else {
    if (i_rep->release())
      delete i_rep;
    i_rep = 0;
  }

  return *this;
}

// -----------------------------------------------------------------------------
// Two iterators are considered equal if they are either both invalid
// (i.e. iterator where the representation is code(0)) or if they are
// both valid and point to the same name.

bool dir_it::operator== (const dir_it& it) const
{
  if (i_rep == 0)
    return it.i_rep == 0? true: false;
  else
    return it.i_rep == 0? false: i_value == it.i_value;
}
