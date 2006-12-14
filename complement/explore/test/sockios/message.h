// -*- C++ -*- Time-stamp: <05/12/20 10:10:21 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 2.1
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

#ifndef __message_h
#define __message_h

#include <string>
#include <mt/xmt.h>

extern std::string message;
extern std::string message_rsp;

extern std::string message_a;
extern std::string message_b;

extern std::string message1;
extern std::string message_rsp1;

extern std::string message2;
extern std::string message_rsp2;

extern int port;

extern xmt::Mutex pr_lock;

#define OUT_MSG(msg) pr_lock.lock(); cerr << msg << endl; pr_lock.unlock()

namespace test_area {

extern int bin_buff1_size;
extern char *bin_buff1;

extern int ni1;
extern int ni2;

} // namespace test_area

#endif // __message_h
