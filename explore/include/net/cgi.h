// -*- C++ -*- Time-stamp: <02/09/29 20:22:07 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __net_cgi_h
#define __net_cgi_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <list>
#include <utility>

namespace cgi {

class environment
{
  public:
    explicit environment();

    const char *ServerSoftware() const
      { return server_software.c_str(); }
    const char *ServerName() const
      { return server_name.c_str(); }
    const char *GatewayInterface() const
      { return gateway_interface.c_str(); }

    const char *Protocol() const
      { return server_protocol.c_str(); }
    int Port() const
      { return server_port; }
    const char *PathInfo() const
      { return path_info.c_str(); }
    const char *PathTranslated() const
      { return path_translated.c_str(); }
    const char *ScriptName() const
      { return script_name.c_str(); }
    const char *RequestMethod() const
      { return request_method.c_str(); }
    const char *Query() const
      { return query_string.c_str(); }
    const char *RemoteHost() const
      { return remote_host.c_str(); }
    const char *RemoteAddr() const
      { return remote_addr.c_str(); }
    const char *AuthType() const
      { return auth_type.c_str(); }
    const char *RemoteUser() const
      { return remote_user.c_str(); }
    const char *RemoteIdent() const
      { return remote_ident.c_str(); }
    const char *ContentType() const
      { return content_type.c_str(); }
    size_t ContentLength() const
      { return content_length; }
    const char *HTTPUserAgent() const
      { return http_user_agent.c_str(); }


    static std::string hexify( const std::string& );
    static std::string unhexify( const std::string& );

    const std::string& value( const char * ) const;

  protected:

    // The following environment variables are not request-specific and
    // are set for all requests:
    mutable std::string server_software;
    mutable std::string server_name;
    mutable std::string gateway_interface;

    // The following environment variables are specific to the request
    // being fulfilled by the gateway program:
    mutable std::string server_protocol;
    mutable int server_port;
    mutable std::string path_info;
    mutable std::string path_translated;
    mutable std::string script_name;
    mutable std::string request_method;
    mutable std::string query_string;
    mutable std::string remote_host;
    mutable std::string remote_addr;
    mutable std::string auth_type;
    mutable std::string remote_user;
    mutable std::string remote_ident;
    mutable std::string content_type;
    mutable size_t content_length;

    mutable std::string http_user_agent;

  private:
    static const char *get( const char *name );
    static std::string decode( const std::string& );

    typedef std::pair<std::string,std::string> value_type;
    typedef std::list<value_type> container_type;
    container_type pars;
};

} // namespace cgi

#endif // __net_cgi_h
