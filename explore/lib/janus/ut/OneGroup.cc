// -*- C++ -*- Time-stamp: <09/09/08 15:49:46 ptr>

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

namespace janus {

using namespace std;

typedef set<stem::addr_type> super_spirit_type;

struct VT_sync :
    public stem::__pack_base
{
    void pack( std::ostream& s ) const;
    void unpack( std::istream& s );

    VT_sync()
      { }
    VT_sync( const VT_sync& vt_ ) :
        vt( vt_.vt )
      { }

    vtime vt;
};

void VT_sync::pack( std::ostream& s ) const
{
  vt.pack( s );
}

void VT_sync::unpack( std::istream& s )
{
  vt.unpack( s );
}

class VTM_one_group_handler :
    public stem::EventHandler
{
  public:
    VTM_one_group_handler();
    VTM_one_group_handler( stem::addr_type id );
    VTM_one_group_handler( stem::addr_type id, const char *info );
    ~VTM_one_group_handler();

    void join( super_spirit_type& );

    void new_member_round1( const stem::Event_base<VT_sync>& );
    void new_member_round2( const stem::Event_base<VT_sync>& );

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

bool VTM_one_group_handler::_status::operator()() const
{
  return me.pass;
}

DEFINE_RESPONSE_TABLE( VTM_one_group_handler )
  EV_Event_base_T_( ST_NULL, VT_GROUP_R1, new_member_round1, VT_sync )
  EV_Event_base_T_( ST_NULL, VT_GROUP_R2, new_member_round2, VT_sync )
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

} // namespace janus
