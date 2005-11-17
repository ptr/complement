// -*- C++ -*- Time-stamp: <05/11/15 17:26:34 ptr>

#include <resolver/res_ip.h>
#include <errno.h>


#if 0
// ADL not fully supported (by gcc 3.4.4), so do it in std namespace:

namespace std {

inline bool operator <( const resolver::details::resolve::id_type& l, const resolver::details::resolve::id_type& r )
{
  // return *l < *r;
  return &l < &r;
}

}

#endif // 0

namespace resolver {

namespace details {

using namespace std;
using namespace boost;

#define RETERR(err) return -1 // do { __set_errno (err); return (-1); } while (0)

static void setsection(ns_msg *msg, ns_sect sect);

int x_ns_skiprr(const u_char *ptr, const u_char *eom, ns_sect section, int count)
{
  const u_char *optr = ptr;

  for ( ; count > 0; count--) {
    int b, rdlength;

    b = dn_skipname(ptr, eom);
    if (b < 0)
      RETERR(EMSGSIZE);
    ptr += b /*Name*/ + NS_INT16SZ /*Type*/ + NS_INT16SZ /*Class*/;
    if (section != ns_s_qd) {
      if (ptr + NS_INT32SZ + NS_INT16SZ > eom)
        RETERR(EMSGSIZE);
      ptr += NS_INT32SZ/*TTL*/;
      NS_GET16(rdlength, ptr);
      ptr += rdlength/*RData*/;
    }
  }
  if (ptr > eom)
    RETERR(EMSGSIZE);
  return (ptr - optr);
}

int x_ns_initparse(const u_char *msg, int msglen, ns_msg *handle)
{
  const u_char *eom = msg + msglen;
  int i;

  memset(handle, 0x5e, sizeof *handle);
  handle->_msg = msg;
  handle->_eom = eom;
  if (msg + NS_INT16SZ > eom)
    RETERR(EMSGSIZE);
  NS_GET16(handle->_id, msg);
  if (msg + NS_INT16SZ > eom)
    RETERR(EMSGSIZE);
  NS_GET16(handle->_flags, msg);
  for (i = 0; i < ns_s_max; i++) {
    if (msg + NS_INT16SZ > eom)
      RETERR(EMSGSIZE);
    NS_GET16(handle->_counts[i], msg);
  }
  for (i = 0; i < ns_s_max; i++)
    if (handle->_counts[i] == 0)
      handle->_sections[i] = NULL;
    else {
      int b = details::x_ns_skiprr(msg, eom, (ns_sect)i, handle->_counts[i]);

      if (b < 0)
        return (-1);
      handle->_sections[i] = msg;
      msg += b;
    }
  if (msg != eom)
    RETERR(EMSGSIZE);
  details::setsection(handle, ns_s_max);
  return (0);
}

int x_ns_parserr(ns_msg *handle, ns_sect section, int rrnum, ns_rr *rr)
{
  int b;

  /* Make section right. */
  if (section < 0 || section >= ns_s_max)
    RETERR(ENODEV);
  if (section != handle->_sect)
    setsection(handle, section);

  /* Make rrnum right. */
  if (rrnum == -1)
    rrnum = handle->_rrnum;
  if (rrnum < 0 || rrnum >= handle->_counts[(int)section])
    RETERR(ENODEV);
  if (rrnum < handle->_rrnum)
    setsection(handle, section);
  if (rrnum > handle->_rrnum) {
    b = details::x_ns_skiprr(handle->_ptr, handle->_eom, section, rrnum - handle->_rrnum);
    if (b < 0)
      return (-1);
    handle->_ptr += b;
    handle->_rrnum = rrnum;
  }

  /* Do the parse. */
  b = dn_expand(handle->_msg, handle->_eom,
                handle->_ptr, rr->name, NS_MAXDNAME);
  if (b < 0)
    return (-1);
  handle->_ptr += b;
  if (handle->_ptr + NS_INT16SZ + NS_INT16SZ > handle->_eom)
    RETERR(EMSGSIZE);
  NS_GET16(rr->type, handle->_ptr);
  NS_GET16(rr->rr_class, handle->_ptr);
  if (section == ns_s_qd) {
    rr->ttl = 0;
    rr->rdlength = 0;
    rr->rdata = NULL;
  } else {
    if (handle->_ptr + NS_INT32SZ + NS_INT16SZ > handle->_eom)
      RETERR(EMSGSIZE);
    NS_GET32(rr->ttl, handle->_ptr);
    NS_GET16(rr->rdlength, handle->_ptr);
    if (handle->_ptr + rr->rdlength > handle->_eom)
      RETERR(EMSGSIZE);
    rr->rdata = handle->_ptr;
    handle->_ptr += rr->rdlength;
  }
  if (++handle->_rrnum > handle->_counts[(int)section])
    setsection(handle, (ns_sect)((int)section + 1));

  /* All done. */
  return (0);
}

/* Private. */

static void setsection(ns_msg *msg, ns_sect sect)
{
  msg->_sect = sect;
  if (sect == ns_s_max) {
    msg->_rrnum = -1;
    msg->_ptr = NULL;
  } else {
    msg->_rrnum = 0;
    msg->_ptr = msg->_sections[(int)sect];
  }
}


ns_answer::ns_answer( unsigned char *b, int len )
{
  const u_char *eom = b + len;
  int i;

  memset(&_answer, 0x5e, sizeof(ns_msg) );
  _answer._msg = b;
  _answer._eom = eom;
  if ( (b + (2 + ns_s_max) * NS_INT16SZ) > eom ) {
    throw std::range_error( "bad buffer length" );
  }
  ns16 b2;
  b2.c[0] = *b++; b2.c[1] = *b++;
  _answer._id = ntohs( b2.s );
  b2.c[0] = *b++; b2.c[1] = *b++;
  _answer._flags = ntohs( b2.s );
  for ( int i = 0; i < ns_s_max; i++ ) {
    b2.c[0] = *b++; b2.c[1] = *b++;
    _answer._counts[i] = ntohs( b2.s );
  }
  for ( int i = 0; i < ns_s_max; i++ ) {
    if (_answer._counts[i] == 0 ) {
      _answer._sections[i] = 0;
    } else {
      int n = details::x_ns_skiprr(b, eom, (ns_sect)i, _answer._counts[i]);

      if (n < 0)
        throw std::range_error( "bad buffer length" );
      _answer._sections[i] = b;
      b += n;
    }
  }
  if (b != eom)
    throw std::range_error( "bad buffer length" );
  setsection(&_answer, ns_s_max);
}


ns_answer::iterator ns_answer::begin( ns_sect s )
{
  iterator i(s);
  i.msg = &_answer;
  i.n = 0;
  if ( details::x_ns_parserr( i.msg, ns_s_an, 0, &i._rr ) < 0 ) {
    i.n = -1;
  }
  return i;
}

void priority_ip::penalty()
{
  pip_t v = ips.top();
  ips.pop();
  v.first = ++max_penalty;
  ips.push( v );
}

void priority_ip::push( penalty_type p, value_type v )
{
  if ( p > max_penalty ) {
    max_penalty = p;
  }
  ips.push( make_pair( p, v ) );
}

const priority_ip::value_type priority_ip::bad_ip = static_cast<priority_ip::value_type>(-1);

priority_ip& resolve::operator [](const string& domain )
{
#if 0
  pair<id_type,bool> p = str_map.insert( domain );
  domain_results& dr = r[p.first];

  if ( p.second && !dr.ips.empty() ) { // p.second is false if just inserted
    ++dr.hits;
  }

  return dr.ips;
#else
  domain_results& dr = r[domain];

  if ( !dr.ips.empty() ) {
    ++dr.hits;
  }

  return dr.ips;
#endif
}


resolve _resolver;


string decode_string( const unsigned char *first, const unsigned char *last, const unsigned char *ref_buf )
{
  const unsigned char *p = first;
  std::string s;

  while ( p < last ) {
    if ( (*p & 0xc0) == 0 ) {
      int n = (int)*p++;
      if ( n == 0 ) {
        break;
      }
      for ( int i = 0; i < n; ++i ) {
        s += (char)*p++;
      }
      s += '.';
    } else {
      int n = (int)(*p++ & 0x3f);
      n <<= 8;
      n |= (unsigned)(*p++);
      p = ref_buf + n;
    }
  }

  return s;
}

} // namespace details

} // namespace resolver
