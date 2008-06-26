/* Time-stamp: <07/08/21 10:39:52 ptr> */

/*
 * Copyright (c) 2003, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __config__gcc_h
#define __config__gcc_h

/*
#if (__GNUC__==3) // gcc 3.1.1 at least
#  define __FIT_TYPENAME_TEMPLATE_PARAMETER_RET
#endif
*/
#if ( __GNUC__ == 2 ) && (__GNUC_MINOR__ == 95)
/* at least 2.95.3
#define __FIT_STD_AS_GLOBAL
*/
#endif

/* 3.3.6, but 3.4.6 and 4.1.1 free from this */
#if (__GNUC__ < 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ < 4))
#  define __FIT_NO_INLINE_TEMPLATE_STATIC_INITIALISATION
#endif

#define __FIT_CPP_DEMANGLE

#endif /* __config__gcc_h */
