// -*- C++ -*- Time-stamp: <10/01/15 21:02:14 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <janus/torder.h>

namespace janus {

using namespace std;
using namespace xmt;
using namespace stem;
using namespace std::tr2;

torder_vs::torder_vs() :
    basic_vs()
{
}

torder_vs::torder_vs( const char* info ) :
    basic_vs( info )
{
}

torder_vs::~torder_vs()
{
}

int torder_vs::vs( const stem::Event& inc_ev )
{
  // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;
  if ( /* vs_group_size() == 0 || */ /* lock_addr != stem::badaddr || */ isState(VS_ST_LOCKED) ) {
    de.push_back( inc_ev );
    // cerr << __FILE__ << ':' << __LINE__ << " View Update " << self_id() << ' ' << lock_addr << endl;
    return 1;
  }

  // if ( !de.empty() ) {
  //   cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << ' ' << de.size() << endl;
  // }

  stem::Event_base<vs_event> ev( VS_EVENT );
  ev.value().id = xmt::uid();
  ev.value().view = view;
  ev.value().ev = inc_ev;

  vtime& self = vt[self_id()];
  ++self[self_id()];

  const stem::code_type code = inc_ev.code();

  if ( (code != VS_UPDATE_VIEW) && (code != VS_LOCK_VIEW) && (code != VS_FLUSH_LOCK_VIEW ) ) {
    this->vs_event_origin( self, ev.value().ev );
  }

  ev.value().ev.setf( stem::__Event_Base::vs );
  // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << ' ' << hex << inc_ev.code() << dec << endl;
  for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    if ( i->first != self_id() ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << endl;
      ev.dest( i->first );
      ev.value().vt = self; // chg( self, vt[i->first] );
      // vt[i->first] = self; // <------------
      Send( ev );
    }
  }
  // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;

  return 0;
}

// void torder_vs::vs_send_flush()
// {
// }

void torder_vs::vs_process( const stem::Event_base<vs_event>& ev )
{
  // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << ' ' << hex << ev.value().ev.code() << dec << endl;
  // check the view version first:
  if ( ev.value().view != view ) {
    if ( ev.value().view > view ) {
      ove.push_back( ev ); // push event into delay queue
    }
    return;
  }

  stem::code_type code = ev.value().ev.code();

  if ( code == VS_UPDATE_VIEW ) {
    ove.push_back( ev ); // push event into delay queue
    return;
  }

  // check virtual time:
  vtime tmp = vt[ev.src()];

  for ( vtime::vtime_type::const_iterator i = ev.value().vt.vt.begin(); i != ev.value().vt.vt.end(); ++i ) {
    if ( i->second < tmp[i->first] ) {
      // ev.src() fail?
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;
      return;
    }
    tmp[i->first] = i->second;
  }

  // tmp.sup( ev.value().vt );

  vtime& self = vt[self_id()];

  for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    if ( i->first == ev.src() ) {
      if ( (i->second + 1) != tmp[ev.src()] ) {
        if ( (i->second + 1) < tmp[ev.src()] ) {
          // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << ' ' << ev.src()  << ' ' << (i->second + 1) << ' ' << tmp[ev.src()] << endl;
          ove.push_back( ev ); // push event into delay queue
        } else {
          // Ghost event from past: Drop? Error?
          // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << ' ' << ev.src() << endl;
        }
        return;
      }
    } else if ( i->second < tmp[i->first] ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << ' ' << ev.src() << ' ' << i->second << ' ' << tmp[i->first] << endl;
      ove.push_back( ev ); // push event into delay queue
      return;
    }
  }

  ++self[ev.src()];
  vt[ev.src()] = tmp; // vt[ev.src()] = self; ??   <--------
  
  ev.value().ev.src( ev.src() );
  ev.value().ev.dest( ev.dest() );

  if ( (code != VS_LOCK_VIEW) && (code != VS_FLUSH_LOCK_VIEW) ) {
    // Update view not passed into vs_event_derivative,
    // it specific for Virtual Synchrony
    this->vs_event_derivative( self, ev.value().ev );
  }

  this->Dispatch( ev.value().ev );

  if ( !ove.empty() ) {
    process_delayed();
  }
}

void torder_vs::vs_process_lk( const stem::Event_base<vs_event>& ev )
{
  const stem::code_type code = ev.value().ev.code();

  // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;
  if ( ev.value().view != view ) {
    // cerr << __FILE__ << ':' << __LINE__ << endl;
    if ( code != VS_UPDATE_VIEW ) {
      ove.push_back( ev ); // push event into delay queue
      return; // ? view changed, but this object unknown yet
    }
//    cerr << __FILE__ << ':' << __LINE__ << ' ' << ev.src() << ' ' << self_id() << endl;
    if ( (view != 0) && (lock_addr != ev.src()) ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << lock_addr 
      //      << ' ' << ev.src() << endl;
      return; // ? update view: not owner of lock
    }
    // cerr << __FILE__ << ':' << __LINE__ << endl;
    if ( (view != 0) && ((view + 1) != ev.value().view) ) {
      ove.push_back( ev ); // push event into delay queue
      return; // ? view changed, but this object unknown yet
    }
    // cerr << __FILE__ << ':' << __LINE__ << ' ' << ev.value().vt.vt.size() << ' ' << self_id() << endl;
    if ( view == 0 ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;
      vt[self_id()] = ev.value().vt.vt; // align time with origin
      --vt[self_id()][ev.src()]; // will be incremented below
    }
#if 1
    else { // (view + 1) == ev.value().view
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;

      vtime& self = vt[self_id()];
      for ( vtime::vtime_type::iterator i = self.vt.begin(); i != self.vt.end(); ) {
        if ( ev.value().vt.vt.find( i->first ) == ev.value().vt.vt.end() ) {
          // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;
          vt.erase( i->first );
          self.vt.erase( i++ );
          // break;
        } else {
          ++i;
        }
      }
    }
#endif
    view = ev.value().view;
  }
  // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;

  if ( (code != VS_UPDATE_VIEW) && (code != VS_LOCK_VIEW) && (code != VS_FLUSH_VIEW) && (code != VS_FLUSH_LOCK_VIEW) ) {
    ove.push_back( ev ); // push event into delay queue
    return;
  }

  // check virtual time:
  vtime tmp = vt[ev.src()];

  for ( vtime::vtime_type::const_iterator i = ev.value().vt.vt.begin(); i != ev.value().vt.vt.end(); ++i ) {
    if ( i->second < tmp[i->first] ) {
      // ev.src() fail?
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;
      return;
    }
    tmp[i->first] = i->second;
  }

  vtime& self = vt[self_id()];

  for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    if ( i->first == ev.src() ) {
      if ( (i->second + 1) != tmp[ev.src()] ) {
        if ( (i->second + 1) < tmp[ev.src()] ) {
          // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << ' ' << ev.src()  << ' ' << (i->second + 1) << ' ' << tmp[ev.src()] << endl;
          ove.push_back( ev ); // push event into delay queue
        } else {
          // Ghost event from past: Drop? Error?
          // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << ' ' << ev.src() << endl;
        }
        return;
      }
    } else if ( i->second < tmp[i->first] ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << ' ' << ev.src() << ' ' << i->second << ' ' << tmp[i->first] << endl;
      ove.push_back( ev ); // push event into delay queue
      return;
    }
  }

  ++self[ev.src()];
  vt[ev.src()] = tmp; // vt[ev.src()] = self; ??   <--------
  
  ev.value().ev.src( ev.src() );
  ev.value().ev.dest( ev.dest() );

  if ( code == VS_UPDATE_VIEW ) {
    /* Specific for update view: vt[self_id()] should
       contain all group members, even if virtual time
       is zero (copy/assign vt don't copy entry with zero vtime!)
    */
    // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;
    for ( vtime::vtime_type::const_iterator i = ev.value().vt.vt.begin(); i != ev.value().vt.vt.end(); ++i ) {
      self[i->first];
    }
  } else if ( code == VS_FLUSH_VIEW ) {
    // flush passed into vs_event_derivative
    this->vs_event_derivative( self, ev.value().ev );
  }

  this->Dispatch( ev.value().ev );

  if ( !ove.empty() ) {
    process_delayed();
  }
}

DEFINE_RESPONSE_TABLE( torder_vs )
  EV_Event_base_T_( ST_NULL, VS_EVENT, vs_process, vs_event )
  EV_Event_base_T_( VS_ST_LOCKED, VS_EVENT, vs_process_lk, vs_event )
END_RESPONSE_TABLE

} // namespace janus
