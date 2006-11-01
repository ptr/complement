// -*- C++ -*- Time-stamp: <04/04/21 11:51:18 ptr>

/*
 *
 * Copyright (c) 2004
 * Kaspersky Labs
 *
 * Licensed under the Academic Free License Version 2.0
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

#ifndef __mt__fbuf_h
#define __mt__fbuf_h

#if defined(WIN32) && !defined(_STLPORT_VERSION) && !defined(__GNUC__)

#include <fstream>

_STD_BEGIN

template <class _E, class _Tr = char_traits<_E> >
class __basic_filebuf :
    public basic_streambuf<_E, _Tr>
{
  public:
    typedef __basic_filebuf<_E, _Tr> _Myt;
    typedef basic_streambuf<_E, _Tr> _Mysb;
    typedef codecvt<_E, char, _Tr::state_type> _Cvt;

    __basic_filebuf(_Filet *_F = 0) :
        _Loc(), _Mysb()
      {_Init(_F, _Newfl); }

    __basic_filebuf(_Uninitialized) :
        _Loc(_Noinit), _Mysb(_Noinit)
      {}

    virtual ~__basic_filebuf()
      {
        if (_Closef)
          close();
        delete _Str;
      }

    enum _Initfl {_Newfl, _Openfl, _Closefl};

    bool is_open() const
      { return (_File != 0); }

    _Myt *open(const char *_S, ios_base::openmode _M)
      {
        _Filet *_Fp;
        if (_File != 0 || (_Fp = __Fiopen(_S, _M)) == 0)
          return (0);
        _Init(_Fp, _Openfl);
        _Initcvt();
        return (this);
      }

    _Myt *open(const char *_N, ios::open_mode _M)
      { return (open(_N, (ios::openmode)_M)); }

    _Myt *close()
      {
        if (_File != 0 && fclose(_File) == 0) {
          _Init(0, _Closefl);
          return (this);
        } else
          return (0);
      }

  protected:
    virtual int_type overflow(int_type _C = _Tr::eof())
      {
        if (_Tr::eq_int_type(_Tr::eof(), _C))
          return (_Tr::not_eof(_C));
        else if (pptr() != 0 && pptr() < epptr()) {
          *_Pninc() = _Tr::to_char_type(_C);
          return (_C);
        } else if (_File == 0)
          return (_Tr::eof());
        else if (_Pcvt == 0)
          return (_Fputc(_Tr::to_char_type(_C), _File) ? _C : _Tr::eof());
        else
        {
          const int _NC = 8;
          const _E _X = _Tr::to_char_type(_C);
          const _E *_S;
          char *_D;
          _Str->erase();
          for (size_t _I = _NC; ; _I += _NC) {
            _Str->append(_NC, '\0');
            switch (_Pcvt->out(_State, &_X, &_X + 1, _S, _Str->begin(), _Str->end(), _D)) {
              case codecvt_base::partial:
                if (_S == &_X)
                  return (_Tr::eof());
              case codecvt_base::ok:  // can fall through
              {
                size_t _N = _D - _Str->begin();
                return (fwrite(_Str->begin(), 1, _N, _File) == _N ? _C : _Tr::eof());
              }
              case codecvt_base::noconv:
                return (_Fputc(_X, _File) ? _C : _Tr::eof());
              default:
                return (_Tr::eof());
            }
          }
        }
      }

    virtual int_type pbackfail(int_type _C = _Tr::eof())
      {
        if (gptr() != 0 && eback() < gptr()
            && (_Tr::eq_int_type(_Tr::eof(), _C)
                || _Tr::eq_int_type(_Tr::to_int_type(gptr()[-1]), _C))) {
          _Gndec();
          return (_Tr::not_eof(_C));
        } else if (_File == 0 || _Tr::eq_int_type(_Tr::eof(), _C))
          return (_Tr::eof());
        else if (_Pcvt == 0)
          return (_Ungetc(_Tr::to_char_type(_C), _File) ? _C : _Tr::eof());
        else if (0 < _Str->size() && _Ungetc(*_Str->begin(), _File, _Str->size())) {
          _Str->erase();
          _State = _State0;
          return (_C);
        } else
          return (_Tr::eof());
      }

    virtual int_type underflow()
      {
        if (gptr() != 0 && gptr() < egptr())
          return (_Tr::to_int_type(*gptr()));
        else
          return (pbackfail(uflow()));
      }

    virtual int_type uflow()
      {
        if (gptr() != 0 && gptr() < egptr())
          return (_Tr::to_int_type(*_Gninc()));
        else if (_File == 0)
          return (_Tr::eof());
        else if (_Pcvt == 0) {
          _E _C;
          return (_Fgetc(_C, _File) ? _Tr::to_int_type(_C) : _Tr::eof());
        } else
          for (_State0 = _State, _Str->erase(); ; ) {
            _E _X, *_D;
            const char *_S;
            int _C = fgetc(_File);
            if (_C == EOF)
              return (_Tr::eof());    // partial char?
            _Str->append(1, (char)_C);
            _State = _State0;
            switch (_Pcvt->in(_State, _Str->begin(), _Str->end(), _S, &_X, &_X + 1, _D)) {
              case codecvt_base::partial:
                break;
              case codecvt_base::noconv:
                if (_Str->size() < sizeof (_E))
                  break;
                memcpy(&_X, _Str->begin(), sizeof (_E));
              case codecvt_base::ok:  // can fall through
                return (_Tr::to_int_type(_X));
              default:
                return (_Tr::eof());
            }
          }
      }

    virtual pos_type seekoff(off_type _O, ios_base::seekdir _Way, ios_base::openmode = (ios_base::openmode)(ios_base::in | ios_base::out))
      {
        fpos_t _Fp;
        if (_File == 0 || fseek(_File, _O, _Way) != 0 || fgetpos(_File, &_Fp) != 0)
          return (pos_type(_BADOFF));
        return (pos_type(_State, _Fp));
      }

    virtual pos_type seekpos(pos_type _P, ios_base::openmode = (ios_base::openmode)(ios_base::in | ios_base::out))
      {
        fpos_t _Fp = _P.get_fpos_t();
        off_type _Off = (off_type)_P - _FPOSOFF(_Fp);
        if (_File == 0 || fsetpos(_File, &_Fp) != 0 || _Off != 0
            && fseek(_File, _Off, SEEK_CUR) != 0 || fgetpos(_File, &_Fp) != 0)
          return (pos_type(_BADOFF));
        if (_Str != 0)
          _State = _P.state(), _Str->erase();
        return (pos_type(_State, _Fp));
      }

    virtual _Mysb *setbuf(_E *_S, streamsize _N)
      { return (_File == 0 || setvbuf(_File, (char *)_S, _IOFBF, _N * sizeof (_E)) != 0 ? 0 : this); }

    virtual int sync()
      { return (_File == 0 || 0 <= fflush(_File) ? 0 : -1); }

    void _Init(_Filet *_Fp, _Initfl _Which)
      {
        static _Tr::state_type _Stinit;
        _Closef = _Which == _Openfl;
        if (_Which == _Newfl) {
          _Loc.locale::~locale();
          new (&_Loc) locale;
          _Str = 0;
        }

        _Mysb::_Init();
        if (_Fp != 0 && !_Closef && sizeof (_E) == 1) {
          _Mysb::_Init((_E **)&_Fp->_base,
                       (_E **)&_Fp->_ptr, &_Fp->_cnt,
                       (_E **)&_Fp->_base, (_E **)&_Fp->_ptr,
                       &_Fp->_cnt);
        }
        _File = _Fp;
        _State = _Stinit;
        _State0 = _Stinit;
        _Pcvt = 0;
      }
    
    void _Initcvt()
      {
        _Pcvt = (_Cvt *)&_USE(getloc(), _Cvt);
        _Loc = _ADDFAC(_Loc, _Pcvt);
        if (_Pcvt->always_noconv())
          _Pcvt = 0;
        if (_Str == 0)
          _Str = new string;
      }

  protected:
    _Cvt *_Pcvt;
    _Tr::state_type _State0;
    _Tr::state_type _State;
    string *_Str;
    bool _Closef;
    locale _Loc;
    _Filet *_File;
};

#ifdef _DLL
#pragma warning(disable:4231) /* the extern before template is a non-standard extension */
extern template class _CRTIMP __basic_filebuf<char, char_traits<char> >;
extern template class _CRTIMP __basic_filebuf<wchar_t, char_traits<wchar_t> >;
#pragma warning(default:4231) /* restore previous warning */
#endif          // _DLL

_STD_END

#endif // WIN32 && !_STLPORT_VERSION && !__GNUC__

#endif // __mt__fbuf_h
