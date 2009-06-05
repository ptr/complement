// -*- C++ -*- Time-stamp: <09/06/05 00:43:34 ptr>

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
    std::string s;
};

#endif // __vf_h
