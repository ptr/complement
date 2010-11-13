// -*- C++ -*- Time-stamp: <10/07/07 18:48:57 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __vf_h
#define __vf_h

#include <mt/mutex>
#include <mt/condition_variable>
#include <stem/EventHandler.h>
#include <string>

class VF :
    public stem::EventHandler
{
  public:
    VF( stem::addr_type id );
    virtual ~VF();

    virtual bool Dispatch( const stem::Event& );

    // int v;
    static int s;

  private:
#if 0
    std::tr2::mutex m;
    std::tr2::condition_variable cnd;
    
    struct check_v 
    {
      check_v( VF& m ) :
          me( m )
        { }

      bool operator()() const
        { return me.v == 1; }

      VF& me;
    };
#endif
};

class VF1 :
    public stem::EventHandler
{
  public:
    VF1( stem::addr_type id );
    virtual ~VF1();

    virtual bool Dispatch( const stem::Event& );

  private:
    char* s;
};

#endif // __vf_h
