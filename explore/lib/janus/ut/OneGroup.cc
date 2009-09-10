// -*- C++ -*- Time-stamp: <09/09/10 15:29:34 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "vt_operations.h"

#include <iostream>
#include <janus/vtime.h>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>

#include <set>
#include <list>

namespace janus {

using namespace std;

typedef set<stem::addr_type> super_spirit_type;

struct VT_base :
    public stem::__pack_base
{
    void pack( std::ostream& s ) const;
    void unpack( std::istream& s );

    VT_base()
      { }
    VT_base( const VT_base& vt_ ) :
        vt( vt_.vt )
      { }

    void swap( VT_base& );

    vtime vt;
};

void VT_base::pack( std::ostream& s ) const
{
  vt.pack( s );
}

void VT_base::unpack( std::istream& s )
{
  vt.unpack( s );
}

void VT_base::swap( VT_base& r )
{
  std::swap( vt, r.vt );
}

struct VT_event :
    public VT_base
{
    VT_event()
      { }
    VT_event( const VT_event& e ) :
        VT_base( e ),
        ev( e.ev )
      { }

    virtual void pack( std::ostream& s ) const;
    virtual void unpack( std::istream& s );

    void swap( VT_event& );

    stem::Event ev;
};

void VT_event::pack( std::ostream& s ) const
{
  VT_base::pack( s );
  __pack( s, ev.code() );
  __pack( s, ev.flags() );
  __pack( s, ev.value() );
}

void VT_event::unpack( std::istream& s )
{
  VT_base::unpack( s );
  stem::code_type c;
  __unpack( s, c );
  ev.code( c );
  uint32_t f;
  __unpack( s, f );
  ev.resetf( f );
  // string d;
  __unpack( s, ev.value() );
  // std::swap( d, ev.value() );
}

void VT_event::swap( VT_event& r )
{
  std::swap( vt, r.vt );
  std::swap( ev, r.ev );
}

struct VT_sync :
    public VT_base
{
    VT_sync()
      { }
    VT_sync( const VT_sync& vt_ ) :
        VT_base( vt_ )
      { }
};

class VTM_one_group_handler :
    public stem::EventHandler
{
  public:
    VTM_one_group_handler();
    VTM_one_group_handler( stem::addr_type id );
    VTM_one_group_handler( stem::addr_type id, const char *info );
    ~VTM_one_group_handler();

    void join( super_spirit_type& );
    void VTSend( const stem::Event& );

    template <class Duration>
    bool wait( const Duration& rel_time )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );

        return cnd.timed_wait( lk, rel_time, status );
      }

    vtime_matrix_type vt;

    std::string mess;

    void reset()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx ); pass = false; }


  private:
    void message( const stem::Event& );

    void new_member_round1( const stem::Event_base<VT_sync>& );
    void new_member_round2( const stem::Event_base<VT_sync>& );
    void vt_process( const stem::Event_base<VT_event>& );

    typedef std::list<stem::Event_base<VT_event> > delay_container_type;

    delay_container_type dc;

    std::tr2::mutex mtx;
    std::tr2::condition_variable cnd;
    bool pass;

    struct _status
    {
        _status( VTM_one_group_handler& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_handler& me;
    } status;


    DECLARE_RESPONSE_TABLE( VTM_one_group_handler, stem::EventHandler );
};

#define VT_GROUP_R1  0x1205
#define VT_GROUP_R2  0x1206
#define VT_MESS      0x1207

#define EV_FREE      0x9000

VTM_one_group_handler::VTM_one_group_handler() :
    EventHandler(),
    pass( false ),
    status( *this )
{
  enable();
}

VTM_one_group_handler::VTM_one_group_handler( stem::addr_type id ) :
    EventHandler( id ),
    pass( false ),
    status( *this )
{
  enable();
}

VTM_one_group_handler::VTM_one_group_handler( stem::addr_type id, const char *info ) :
    EventHandler( id, info ),
    pass( false ),
    status( *this )
{
  enable();
}

VTM_one_group_handler::~VTM_one_group_handler()
{
  // cnd.wait();
}

void VTM_one_group_handler::VTSend( const stem::Event& inc_ev )
{
  stem::Event_base<VT_event> ev( VT_MESS );
  ev.value().ev = inc_ev;

  vtime& self = vt[self_id()];
  ++self[self_id()];

  for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    if ( i->first != self_id() ) {
      ev.dest( i->first );
      ev.value().vt = chg( self, vt[i->first] );
      // vt[i->first] = self;
      Send( ev );
    }
  }
}

void VTM_one_group_handler::new_member_round1( const stem::Event_base<VT_sync>& ev )
{
  vtime& newbie = vt[ev.src()];
  vtime& self = vt[self_id()];

  newbie = ev.value().vt;
  newbie[self_id()] = self[self_id()];
  self[ev.src()] = newbie[ev.src()];

  // super is ordered container
  super_spirit_type super;

  // all group members (except newbie) take from vtime
  for (vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    super.insert( i->first );
  }

  super_spirit_type::const_iterator i = super.find( self_id() );

  if ( ++i == super.end() ) {
    i = super.begin();
  }

  if ( *i == self_id() ) {
    return; // I'm single in the group
  }

  if ( ev.src() != self_id() ) { // continue round 1
    stem::Event_base<VT_sync> ev_next( VT_GROUP_R1 );

    ev_next.dest( *i );
    ev_next.src( ev.src() );
    ev_next.value().vt = newbie;

    Forward( ev_next );
  } else { // start round 2
    stem::Event_base<VT_sync> ev_next( VT_GROUP_R2 );

    ev_next.dest( *i );
    ev_next.src( ev.src() );
    ev_next.value().vt = newbie;

    Forward( ev_next );
  }
}

void VTM_one_group_handler::new_member_round2( const stem::Event_base<VT_sync>& ev )
{
  vtime& newbie = vt[ev.src()];
  vtime& self = vt[self_id()];

  newbie = ev.value().vt;

  // super is ordered container
  super_spirit_type super;

  // all group members (include newbie) take from vtime
  for (vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    super.insert( i->first );
  }

  super_spirit_type::const_iterator i = super.find( self_id() );

  if ( ++i == super.end() ) {
    i = super.begin();
  }

  if ( *i == self_id() ) {
    return; // I'm single in the group?
  }

  if ( ev.src() != self_id() ) {
    ev.dest( *i );
    Forward( ev );
  } else {
    for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
      if ( i->first != self_id() ) {
        vt[i->first][i->first] = i->second;
        vt[i->first][self_id()] = self[self_id()];
      }      
    }
  }

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  pass = true;
  cnd.notify_one();
}

void VTM_one_group_handler::join( super_spirit_type& super )
{
  super.insert( self_id() );

  super_spirit_type::const_iterator i = super.find( self_id() );

  if ( ++i == super.end() ) {
    i = super.begin();
  }

  if ( i != super.end() && *i != self_id() ) {
    stem::Event_base<VT_sync> ev( VT_GROUP_R1 );

    ev.dest( *i );
    ev.value().vt = vt[self_id()];

    Send( ev );
  }
}

void VTM_one_group_handler::vt_process( const stem::Event_base<VT_event>& ev )
{
  vtime tmp = vt[ev.src()];
  tmp.sup( ev.value().vt );

  vtime& self = vt[self_id()];

  for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    if ( i->first == ev.src() ) {
      if ( (i->second + 1) != tmp[ev.src()] ) {
        if ( (i->second + 1) < tmp[ev.src()] ) {
          dc.push_back( ev ); // Delay
        } else {
          // Ghost event from past: Drop? Error?
        }
        return;
      }
    } else if ( i->second < tmp[i->first] ) {
      dc.push_back( ev ); // Delay
      return;
    }
  }

  ++self[ev.src()];
  vt[ev.src()] = tmp; // vt[ev.src()] = self; ??
  
  ev.value().ev.src( ev.src() );
  ev.value().ev.dest( ev.dest() );

  this->Dispatch( ev.value().ev );

  /*
    for each event in delay_queue try to process it;
    repeat procedure if any event from delay_queue was processed.
   */
  bool delayed_process;
  do {
    delayed_process = false;
    for ( delay_container_type::iterator k = dc.begin(); k != dc.end(); ) {
      tmp = vt[k->src()];
      tmp.sup( k->value().vt );

      for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
        if ( i->first == k->src() ) {
          if ( (i->second + 1) != tmp[k->src()] ) {
            ++k;
            goto try_next;
          }
        } else if ( i->second < tmp[i->first] ) {
          ++k;
          goto try_next;
        }
      }

      ++self[k->src()];
      vt[k->src()] = tmp; // vt[k->src()] = self; ??

      k->value().ev.src( k->src() );
      k->value().ev.dest( k->dest() );

      this->Dispatch( k->value().ev );

      dc.erase( k++ );
      delayed_process = true;

      try_next:
        ;
    }
  } while ( delayed_process );
}

bool VTM_one_group_handler::_status::operator()() const
{
  return me.pass;
}

void VTM_one_group_handler::message( const stem::Event& ev )
{
  mess = ev.value();

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  pass = true;
  cnd.notify_one();
}

DEFINE_RESPONSE_TABLE( VTM_one_group_handler )
  EV_Event_base_T_( ST_NULL, VT_GROUP_R1, new_member_round1, VT_sync )
  EV_Event_base_T_( ST_NULL, VT_GROUP_R2, new_member_round2, VT_sync )
  EV_Event_base_T_( ST_NULL, VT_MESS, vt_process, VT_event )
  EV_EDS( ST_NULL, EV_FREE, message )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VT_one_group_core)
{
  super_spirit_type super_spirit;

  VTM_one_group_handler a1;

  a1.vt[a1.self_id()][a1.self_id()] = 1;

  a1.join( super_spirit );

  VTM_one_group_handler a2;
  
  a2.join( super_spirit );

  EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a1.vt[a1.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a1.vt[a1.self_id()][a2.self_id()] == 0 );
  EXAM_CHECK( a1.vt[a2.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a1.vt[a2.self_id()][a2.self_id()] == 0 );

  EXAM_CHECK( a2.vt[a2.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.vt[a2.self_id()][a2.self_id()] == 0 );
  EXAM_CHECK( a2.vt[a1.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.vt[a1.self_id()][a2.self_id()] == 0 );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_core3)
{
  super_spirit_type super_spirit;

  VTM_one_group_handler a1;
  VTM_one_group_handler a2;
  VTM_one_group_handler a3;

  a1.vt[a1.self_id()][a1.self_id()] = 1;

  a1.join( super_spirit );
  
  // a2.vt[a1.self_id()] = 1;
  a2.vt[a2.self_id()][a2.self_id()] = 2;

  a2.join( super_spirit );

  EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a1.vt[a1.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a1.vt[a1.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a1.vt[a2.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a1.vt[a2.self_id()][a2.self_id()] == 2 );

  EXAM_CHECK( a2.vt[a2.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.vt[a2.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a2.vt[a1.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.vt[a1.self_id()][a2.self_id()] == 2 );
  
  a3.join( super_spirit );

  EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a1.vt[a1.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a1.vt[a1.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a1.vt[a1.self_id()][a3.self_id()] == 0 );
  EXAM_CHECK( a1.vt[a2.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a1.vt[a2.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a1.vt[a2.self_id()][a3.self_id()] == 0 );
  EXAM_CHECK( a1.vt[a3.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a1.vt[a3.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a1.vt[a3.self_id()][a3.self_id()] == 0 );

  EXAM_CHECK( a2.vt[a2.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.vt[a2.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a2.vt[a2.self_id()][a3.self_id()] == 0 );
  EXAM_CHECK( a2.vt[a1.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.vt[a1.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a2.vt[a1.self_id()][a3.self_id()] == 0 );
  EXAM_CHECK( a2.vt[a3.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.vt[a3.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a2.vt[a3.self_id()][a3.self_id()] == 0 );

  EXAM_CHECK( a3.vt[a3.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a3.vt[a3.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a3.vt[a3.self_id()][a3.self_id()] == 0 );
  EXAM_CHECK( a3.vt[a1.self_id()][a1.self_id()] == 1 );
  // EXAM_CHECK( a3.vt[a1.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a3.vt[a1.self_id()][a3.self_id()] == 0 );
  // EXAM_CHECK( a3.vt[a2.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a3.vt[a2.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a3.vt[a2.self_id()][a3.self_id()] == 0 );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_send)
{
  super_spirit_type super_spirit;

  VTM_one_group_handler a1;

  a1.join( super_spirit );

  VTM_one_group_handler a2;

  a2.join( super_spirit );

  EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );

  a1.reset();
  a2.reset();

  stem::Event ev( EV_FREE );
  ev.value() = "message";

  a1.VTSend( ev );
  EXAM_CHECK( a1.vt[a1.self_id()][a1.self_id()] == 1 );

  EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a2.vt[a2.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.mess == "message" );

  a1.reset();
  a2.reset();

  VTM_one_group_handler a3;

  a3.join( super_spirit );

  EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a3.vt[a3.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a3.vt[a1.self_id()][a1.self_id()] == 1 );
  // EXAM_CHECK( a3.vt[a2.self_id()][a1.self_id()] == 1 );

  a1.reset();
  a2.reset();
  a3.reset();

  ev.value() = "another message";

  a3.VTSend( ev );
  EXAM_CHECK( a3.vt[a3.self_id()][a3.self_id()] == 1 );

  EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a2.vt[a2.self_id()][a3.self_id()] == 1 );
  EXAM_CHECK( a2.mess == "another message" );

  EXAM_CHECK( a1.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a1.vt[a1.self_id()][a3.self_id()] == 1 );
  EXAM_CHECK( a1.mess == "another message" );

  return EXAM_RESULT;
}

} // namespace janus
