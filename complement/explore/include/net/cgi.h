// -*- C++ -*- Time-stamp: <02/09/29 20:22:07 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.0
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 *
 */

#ifndef __net_cgi_h
#define __net_cgi_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#pragma ident "@(#)$Id$"
#  endif
#endif

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <list>
#include <utility>

namespace cgi {

using namespace std;

class environment
{
  public:
    explicit environment();

    const char *ServerSoftware() const
      { return server_software.data(); }
    const char *ServerName() const
      { return server_name.data(); }
    const char *GatewayInterface() const
      { return gateway_interface.data(); }

    const char *Protocol() const
      { return server_protocol.data(); }
    int Port() const
      { return server_port; }
    const char *PathInfo() const
      { return path_info.data(); }
    const char *PathTranslated() const
      { return path_translated.data(); }
    const char *ScriptName() const
      { return script_name.data(); }
    const char *RequestMethod() const
      { return request_method.data(); }
    const char *Query() const
      { return query_string.data(); }
    const char *RemoteHost() const
      { return remote_host.data(); }
    const char *RemoteAddr() const
      { return remote_addr.data(); }
    const char *AuthType() const
      { return auth_type.data(); }
    const char *RemoteUser() const
      { return remote_user.data(); }
    const char *RemoteIdent() const
      { return remote_ident.data(); }
    const char *ContentType() const
      { return content_type.data(); }
    size_t ContentLength() const
      { return content_length; }
    const char *HTTPUserAgent() const
      { return http_user_agent.data(); }


    static string hexify( const string& );
    static string unhexify( const string& );

    const string& value( const char * ) const;

  protected:

    // The following environment variables are not request-specific and
    // are set for all requests:
    mutable string server_software;
    mutable string server_name;
    mutable string gateway_interface;

    // The following environment variables are specific to the request
    // being fulfilled by the gateway program:
    mutable string server_protocol;
    mutable int server_port;
    mutable string path_info;
    mutable string path_translated;
    mutable string script_name;
    mutable string request_method;
    mutable string query_string;
    mutable string remote_host;
    mutable string remote_addr;
    mutable string auth_type;
    mutable string remote_user;
    mutable string remote_ident;
    mutable string content_type;
    mutable size_t content_length;

    mutable string http_user_agent;

  private:
    static const char *get( const char *name );
    static string decode( const string& );

    typedef pair<string,string> value_type;
    typedef list<value_type> container_type;
    container_type pars;
};

} // namespace cgi

#endif // __net_cgi_h
