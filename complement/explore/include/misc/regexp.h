// -*- C++ -*- Time-stamp: <00/10/03 15:00:06 ptr>

/*
 *
 * Copyright (c) 2000
 * Petr Ovchenkov
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifndef __regexp_h
#define __regexp_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <stdexcept>
#include <sys/types.h>
#include <regex.h>

class RegExp
{
  private:
    regex_t preg;
    regmatch_t *pmatch;
    bool _status;

  public:
    enum cflags {
      basic    = 0,
      extended = REG_EXTENDED,
      icase    = REG_ICASE,
      nosub    = REG_NOSUB,
      newline  = REG_NEWLINE
    };

    enum eflags {
      beol = 0,
      nbol = REG_NOTBOL,  // begin of line forbidden(^ sign)
      neol = REG_NOTEOL   // end of line forbidden ($ sign)
    };

    typedef std::string::size_type size_type;
    typedef std::pair<size_type,size_type> match_range_type;

    RegExp() :
        pmatch( 0 ),
        _status( false )
      { }

    RegExp( const char *pattern, cflags f = basic ) :
        _status( false )
      { regexp( pattern, f ); }

    RegExp( const std::string& pattern, cflags f = basic ) :
        _status( false )
      { regexp( pattern.c_str(), f ); }

    ~RegExp()
      { RegExp::clear(); }

    void regexp( const char *pattern, cflags f = basic )
      {
        RegExp::clear();
        int err = regcomp( &preg, pattern, (int)f );
        if ( err ) {
          size_t n = regerror( err, &preg, 0, 0 );
          std::string err_str;
          err_str.resize( n );
          regerror( err, &preg, (char *)err_str.c_str(), n );
          throw std::runtime_error( err_str );
        }
        pmatch = (f & nosub) ? 0 : new regmatch_t[preg.re_nsub + 1];
        _status = true;
      }

    void regexp( const std::string& pattern, cflags f = basic )
      { RegExp::regexp( pattern.c_str(), f ); }

    void clear()
      {
        if ( _status ) {
          regfree( &preg );
          if ( pmatch ) {
            delete [] pmatch;
            pmatch = 0;
          }
          _status = false;
        }
      }

    bool find( const char *str, eflags f = beol )
      {
        if ( !_status ) { // no regular expression
          return false; // REG_NOMATCH;
        }
        int err;
        if ( pmatch ) {
          for ( size_t i = 0; i <= preg.re_nsub; ++i ) {
            pmatch[i].rm_so = -1;
            pmatch[i].rm_eo = -1;
          }
          err = regexec( &preg, str, preg.re_nsub + 1, pmatch, (int)f );
        } else { // nosub in force
          err = regexec( &preg, str, 0, 0, (int)f );
        }
        
        return err == 0;
      }

    bool find( const char *str, size_type b, eflags f = beol )
      {
        if ( !_status ) { // no regular expression
          return false; // REG_NOMATCH;
        }
        int err;
        if ( pmatch ) {
          for ( size_t i = 0; i <= preg.re_nsub; ++i ) {
            pmatch[i].rm_so = -1;
            pmatch[i].rm_eo = -1;
          }
          err = regexec( &preg, str + b, preg.re_nsub + 1, pmatch, (int)f );
          for ( size_t i = 0; i <= preg.re_nsub; ++i ) {
            if ( pmatch[i].rm_so != -1 ) {
              pmatch[i].rm_so += b;
            }
            if ( pmatch[i].rm_eo = -1 ) {
              pmatch[i].rm_eo += b;
            }
          }          
        } else { // nosub in force
          err = regexec( &preg, str + b, 0, 0, (int)f );
        }
        
        return err == 0;
      }

    bool find( const std::string& str, eflags f = beol )
      { return RegExp::find( str.c_str(), f ); }

    bool find( const std::string& str, size_type b, eflags f = beol )
      { return RegExp::find( str.c_str(), b, f ); }

    match_range_type operator []( size_t i )
      {
        if ( (pmatch == 0) || (i > preg.re_nsub) ) {
          throw std::out_of_range( "RegExp" );
        }
        regmatch_t& m = pmatch[i];
        return match_range_type( static_cast<size_type>(m.rm_so),
                                 static_cast<size_type>(m.rm_eo) );
      }

//  private:
//    regex_t preg;
//    regmatch_t *pmatch;
};

#endif // __regexp_h
