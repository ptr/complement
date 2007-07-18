// -*- C++ -*- Time-stamp: <05/12/20 10:13:15 ptr>

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

#include "message.h"

std::string message( "Hello!" );
std::string message_rsp( "Welcome!" );

std::string message_a( "Hi!" );
std::string message_b( "How are you?" );

std::string message1( "Are you ok?" );
std::string message_rsp1( "Yes, I am fine!" );

std::string message2( "Good bye!" );
std::string message_rsp2( "I will waiting you!" );

int port = 2048;

namespace test_area {

int bin_buff1_size = 0;
char *bin_buff1 = 0;

int ni1;
int ni2;

} // namespace test_area
