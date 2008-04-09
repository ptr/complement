// -*- C++ -*- Time-stamp: <04/06/11 10:59:55 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: ConnectionProcessor.cc,v 1.3 2004/06/16 14:26:39 ptr Exp $"
#  else
#ident "@(#)$Id: ConnectionProcessor.cc,v 1.3 2004/06/16 14:26:39 ptr Exp $"
#  endif
#endif

#include "ConnectionProcessor.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

__impl::Mutex ConnectionProcessor::m_lock;
__impl::Mutex ConnectionProcessor::r_lock;
__impl::Mutex ConnectionProcessor::orphan_lock;
__impl::Mutex ConnectionProcessor::finished_lock;
ConnectionProcessor::container_type ConnectionProcessor::m;
ConnectionProcessor::reverse_container_type ConnectionProcessor::r;
ConnectionProcessor::container_type ConnectionProcessor::orphan;
ConnectionProcessor::container_type ConnectionProcessor::finished;

bool trace_flag;

ConnectionProcessor::ConnectionProcessor( std::sockstream& s ) :
    sockstr( s )
{
  if ( trace_flag ) {
    cout << "connect" << endl;
  }
  // cerr << "Server see connection\n"; // Be silent, avoid interference
  // with Input line prompt
}

/*
 * Server listen some tcp port, and accept connections.
 * For every connction server expect following:
 *
 *    id=<identification string> action=<action>
 *
 * Here <identification string> any (uniquie for clients)
 * printable string without any whitespaces and <action>
 * is 'out' or 'recv' (without quotations).
 *
 * Action 'out' designate, that client wait notification
 * (from some other side) about message with <identification string>
 * pass; this client stay on wire and wait response from this server.
 *
 * When some other side detect message with <identification string>
 * it send notification to this server in the form
 *
 *    id=<identification string> action=recv
 *
 * Server send to waiting client 'ok\n'. After that client may close
 * connection.
 *
 */

void ConnectionProcessor::connect( std::sockstream& s )
{
  string msg_tmp;
  getline( s, msg_tmp );
  // cerr << msg_tmp << endl;

  stringstream ss( msg_tmp );

  string item;
  const string id_key( "id=" );
  const string action_key( "action=" );
  const string action_out( "out" );
  const string action_recv( "recv" );
  const string action_lstf( "lstf" );
  const string action_lsto( "lsto" );
  const string action_clrf( "clrf" );
  const string action_clro( "clro" );

  string id;
  string action;

  while ( !(ss >> item).fail() ) {
    if ( item.find( id_key ) == 0 ) {
      id = item.substr( id_key.size() );
      // cerr << "id is " << id << endl;
    } else if ( item.find( action_key ) == 0 ) {
      action = item.substr( action_key.size() );
      // cerr << "action is " << action << endl;
    }
  }

  if ( action == action_out ) {
    MT_REENTRANT( m_lock, _1 );
    MT_REENTRANT( r_lock, _2 );
    m[id] = conn( &s );
    r[&s] = id;
    // cerr << hex << (void *)&s << dec << endl;
    // cerr << "More: " << hex << (void *)m[id].s << dec << endl;
    if ( trace_flag ) {
      cout << action_key << action_out << ' ' << id << endl;
    }
  } else if ( action == action_recv ) {
    MT_REENTRANT( m_lock, _1 );
    if ( m[id].s != 0 && m[id].s->good() ) {
      // cerr << "try send ok" << endl;
      // cerr << hex << (void *)m[id].s << dec << endl;
      __impl::Thread::gettime( &m[id].tm[1] ); // fix time
      MT_LOCK( finished_lock );
      finished[id] = m[id];
      finished[id].s = 0;
      MT_UNLOCK( finished_lock );
      timespec elapsed = m[id].tm[1] - m[id].tm[0];

      *(m[id].s) << "ok " // << endl; // notification of waiting client
                 << action_key << action_recv << ' '
                 << id << ' '
                 << elapsed.tv_sec << "." << setiosflags(ios_base::right) << setfill('0') << setw(9) << elapsed.tv_nsec << endl;
      // store in finished contatiner
      if ( trace_flag ) {
        cout << action_key << action_recv << ' '
             << id << ' '
             << elapsed.tv_sec << "." << setiosflags(ios_base::right) << setfill('0') << setw(9) << elapsed.tv_nsec << endl;
      }
      // cerr << "ok sended" << endl;
    }
  } else if ( action == action_lstf ) {
    // list finished
    MT_REENTRANT( finished_lock, _1 );
    timespec elapsed;
    s << "== begin" << endl;
    for ( container_type::iterator i = finished.begin(); i != finished.end(); ++i ) {
      elapsed = (i->second).tm[1] - (i->second).tm[0];
      s << i->first << ' '
        << elapsed.tv_sec << '.'
        << setiosflags(ios_base::right) << setfill('0') << setw(9)
        << elapsed.tv_nsec << '\n';
    }
    s << "== end" << endl;
  } else if ( action == action_lsto ) {
    // list orphaned
    MT_REENTRANT( orphan_lock, _1 );
    timespec elapsed;
    s << "== begin" << endl;
    for ( container_type::iterator i = orphan.begin(); i != orphan.end(); ++i ) {
      __impl::Thread::gettime( &elapsed );
      elapsed -= (i->second).tm[0];
      s << i->first << ' '
        << elapsed.tv_sec << '.'
        << setiosflags(ios_base::right) << setfill('0') << setw(9)
        << elapsed.tv_nsec << '\n';
    }
    s << "== end" << endl;
  } else if ( action == action_clro ) {
    // clear list orphaned
    MT_REENTRANT( orphan_lock, _1 );
    orphan.clear();
  } else if ( action == action_clrf ) {
    // clear list finished
    MT_REENTRANT( finished_lock, _1 );
    finished.clear();
  }

  return;
}

void ConnectionProcessor::close()
{
  MT_REENTRANT( m_lock, _1 );
  MT_REENTRANT( r_lock, _2 );
  if ( r.find( &sockstr ) != r.end() ) {
    string id = r[&sockstr];
    timespec t = m[id].tm[1];
    if ( t.tv_sec == 0 && t.tv_nsec == 0 ) {
      MT_REENTRANT( orphan_lock, _1 );
      orphan[id] = m[id];
      orphan[id].s = 0;
    }
    m.erase( id );
    r.erase( &sockstr );
  }
}
