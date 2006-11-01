 // -*- C++ -*- Time-stamp: <05/04/01 09:02:05 ptr>

 /*
  *
  * Copyright (c) 2002, 2005
  * Petr Ovtchenkov
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


%option c++

%{
#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#pragma ident "@(#)$Id$"
#  endif
#endif

#include <iostream>
#include <iomanip>
#include <sstream>
#include <list>
#include <cstdio>

using namespace std;

struct Lex {
    Lex() :
      brace( 0 ),
      mode( 0 )
    { }

  void x() 
    { }

  enum encoding {
    CP1251
  };

  int brace;
  list<int> level;
  encoding enc;
  int mode;
};

Lex la;

%}

%s PREAMBLE FONTBL COLORTBL FONT STYLESHEET LISTBL INFO LIST LISTOVER
%s TITLE AUTHOR OPERATOR TIME COMPANY GENERATOR RSIDTBL

%%

\{       la.brace++; // cerr << la.brace << endl;

\\\'[0-9a-f][[0-9a-f] {
  istringstream i( YYText(), YYLeng() );
  unsigned c;
  // char ch;
  i.seekg( 2, ios_base::beg );
  i /* >> ch >> ch */ >> hex >> c;
  if ( c == 0x93 ) {
    cout << "<<";
  } else if ( c == 0x94 ) {
    cout << ">>";
  } else if ( c == 0x85 ) {
    cout << "-";
  } else if ( c == 0xb9 ) {
    cout << "\\No";
  } else {
    cout /* << hex << c << ":" */ << (char)c;
  }
}

<INITIAL>\}       la.brace--; // cerr << la.brace << endl;

\\rtf1\\ansi { 
  BEGIN(PREAMBLE);
  la.mode = PREAMBLE;
  la.level.push_back( la.brace - 1 );
  // cerr << "\n===== Start preamble\n\n";
}

<PREAMBLE>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(INITIAL);
      la.mode = INITIAL;
      // cerr << "\n ===== End preamble\n";
      la.level.pop_back();
    }
  }

  \\ansicpg1251 la.enc = Lex::CP1251; // cerr << "Encoding\n";
  
  \{\\fonttbl { 
    BEGIN(FONTBL);
    la.mode = FONTBL;
    la.brace++;
    la.level.push_back( la.brace - 1 );
    // cerr << "\n===== Start Font Table\n\n";
  }

  \{\\colortbl {
    BEGIN(COLORTBL);
    la.mode = COLORTBL;
    la.brace++;
    la.level.push_back( la.brace - 1 );
    // cerr << "\n===== Start Color Table\n\n";
  }

  \{\\stylesheet {
    BEGIN(STYLESHEET);
    la.mode = STYLESHEET;
    la.brace++;
    la.level.push_back( la.brace - 1 );
    // cerr << "\n===== Stylesheet\n";
  }

  \{\\list {
    BEGIN(LIST);
    la.brace++;
    la.level.push_back( la.brace - 1 );
    // cerr << "\n===== Start List\n";
  }

  \{\\listoverride {
    BEGIN(LISTOVER);
    la.brace++;
    la.level.push_back( la.brace - 1 );
    // cerr << "\n===== Start ListOver\n";
  }

  \{\\\*\\rsidtbl\ * {
    BEGIN(RSIDTBL);
    la.brace++;
    la.level.push_back( la.brace - 1 );
    // cerr << "\n===== Start ListOver\n";
  }

  \{\\\*\\generator\ * {
    BEGIN(GENERATOR);
    la.brace++;
    la.level.push_back( la.brace - 1 );
    cout << "% Generator: ";
    // cerr << "\n===== Start ListOver\n";
  }

  \{\\info {
    BEGIN(INFO);
    la.brace++;
    la.level.push_back( la.brace - 1 );
    // cerr << "\n===== Start Info\n";
  }

  \\uc[0-9]+ |
  \\deff[0-9]+ |
  \\deflang[0-9]+ |
  \\deflangfe[0-9]+

  \\\*

  \\listtable

  \\listoverridetable

  \\paperw[0-9]+ |
  \\paperh[0-9]+ |
  \\margl[0-9]+ |
  \\margr[0-9]+ |
  \\margt[0-9]+ |
  \\margb[0-9]+ |
  \\widowctrl |
  \\ftnbj |
  \\aenddoc |
  \\noxlattoyen |
  \\expshrtn |
  \\noultrlspc |
  \\dntblnsbdb |
  \\nospaceforul |
  \\hyphcaps[0-9]+ |
  \\formshade |
  \\horzdoc |
  \\dghspace[0-9]+ |
  \\dgvspace[0-9]+ |
  \\dghorigin[0-9]+ |
  \\dgvorigin[0-9]+ |
  \\dghshow[0-9]+ |
  \\dgvshow[0-9]+ |
  \\jexpand |
  \\viewkind[0-9]+ |
  \\viewscale[0-9]+ |
  \\pgbrdrhead |
  \\pgbrdrfoot |
  \\nolnhtadjtbl\ ? |
  \\fet[0-9]+ |
  \\sectd\ ? |
  \\linex[0-9]+ |
  \\endnhere |
  \\sectdefaultcl\ ? |
  \\pnseclvl[0-9]+ |
  \\pnucrm |
  \\pnstart[0-9] |
  \\pnindent[0-9]+ |
  \\pnhang |
  \\pntxta.. |
  \\pnucltr |
  \\pndec |
  \\pnlcltr |
  \\pntxtb.. |
  \\pnlcrm |
  \\trowd |
  \\trgaph[0-9]+ |
  \\trleft-?[0-9]+ |
  \\trkeep |
  \\trftsWidth[0-9]+ |
  \\trpaddl[0-9]+ |
  \\trpaddr[0-9]+ |
  \\trpaddfl[0-9]+ |
  \\trpaddfr[0-9]+ |
  \\clvertalt |
  \\clbrdrt |
  \\brdrs |
  \\brdrw[0-9]+ |
  \\brdrcf[0-9] |
  \\clbrdrl |
  \\brdrnone\ * |
  \\clbrdrb\ * |
  \\clbrdrr\ * |
  \\cltxlrtb\ * |
  \\clftsWidth[0-9]+ |
  \\clwWidth[0-9]+ |
  \\cellx[0-9]+ |
  \\pard |
  \\plain\ * |
  \\qc\ * |
  \\li[0-9]+ |
  \\ri[0-9]+ |
  \\widctlpar |
  \\intbl |
  \\aspalpha |
  \\aspnum |
  \\faauto |
  \\adjustright |
  \\rin[0-9]+\ * |
  \\lin[0-9]+\ * |
  \\fs[0-9]+\ * |
  \\lang[0-9]+ |
  \\langfe[0-9]+ |
  \\cgrid | 
  \\langnp[0-9]+ |
  \\langfenp[0-9]+ |
  \\b\ * |
  \\f[0-9]+ |
  \\ql\ * |
  \\cell\ * |
  \\row\ * |
  \\pararsid[0-9]+\ *

  \\par\ * cout << /* "% par\n" */ "\n\n";

  \\clcfpat[0-9]+ |
  \\clcbpat[0-9]+ |
  \\clshdng[0-9]+\ ? |
  \\tab |
  \\qj |
  \\trbrdrb |
  \\s[0-9]+\ * |
  \\itap[0-9]+ |
  \\stshfdbch[0-9] |
  \\stshfloch[0-9] |
  \\stshfhich[0-9] |
  \\stshfbi[0-9]

  \\endash cout << "--";
  \\emdash cout << "---";

  \\rsidroot[0-9]+\ * |
  \\sftnbj\ * |
  \\keepn\ * |
  \\outlinelevel[0-9]+\ * |
  \\insrsid[0-9]+\ * |
  \\charrsid[0-9]+\ * |
  \\fi-?[0-9]+\ * |
  \\i |
  \\ul

  \r\n

    \ *\\line\ * cout << "\n";
}

<FONTBL>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(PREAMBLE);
      la.mode = PREAMBLE;
      // cerr << "\n===== End Font Table\n\n";
      la.level.pop_back();
    }
           }
  \{\\f[0-9]+ {
    BEGIN(FONT);
    la.mode = FONT;
    la.brace++;
    istringstream s( YYText(), YYLeng() );
    s.seekg( 3, ios_base::beg );
    int n;
    s >> n;
    la.level.push_back( la.brace - 1 );
    // cerr << "\n===== Start Font " << n << "\n\n";
  }
}

<FONT>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(FONTBL);
      la.mode = FONTBL;
      // cerr << "\n===== End Font\n\n";
      la.level.pop_back();
    }
  }

  \\froman |
  \\fswiss |
  \\fmodern |
  \\fcharset[0-9]+ |
  \\fprq[0-9]+ |
  \\\* |
  \\panose\ [0-9a-f]* |
  [a-zA-Z0-9 ()]+;
}

<STYLESHEET>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(PREAMBLE); // BEGIN_I
      la.mode = PREAMBLE;
      // cerr << "\n===== End Stylesheet\n\n";
      la.level.pop_back();
    }
  }

  \\ql |
  \\li[0-9]+ |
  \\ri[0-9]+ |
  \\widctlpar |
  \\aspalpha |
  \\aspnum |
  \\faauto |
  \\adjustright |
  \\s[0-9]+  |
  \\rin[0-9]+ |
  \\lin[0-9]+ |
  \\itap[0-9]+\ * |
  \\f[0-9]+ |
  \\fs[0-9]+ |
  \\lang[0-9]+ |
  \\langfe[0-9]+ |
  \\cgrid |
  \\langnp[0-9]+ |
  \\langfenp[0-9]+ |
  \\sbasedon[0-9]+ |
  \\snext[0-9]+\ * |
  \\\* |
  [a-zA-Z0-9 ()]+; |
  \\cs[0-9]+\ * |
  \\additive\ * |
  \\i |
  \\qr\ * |
  \\keepn |
  \\outlinelevel[0-9]+ |
  \\b |
  \\ssemihidden\ * |
  \\ts[0-9]+ |
  \\tsrowd |
  \\trpaddl[0-9]+ |
  \\trpaddr[0-9]+ |
  \\trpaddfl[0-9]+ |
  \\trpaddft[0-9]+ |
  \\trpaddfb[0-9]+ |
  \\trpaddfr[0-9]+ |
  \\tscellwidthfts[0-9]+ |
  \\tsvertalt |
  \\tsbrdr[^\\]+ |
  \\trfts[^\\]+ |
  \\tx[0-9]+ |
  \\ul |
  \\fi-?[0-9]+ |
  \\qj
}

<COLORTBL>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(PREAMBLE);
      la.mode = PREAMBLE;
      // cerr << "\n===== End Color Table\n\n";
      la.level.pop_back();
    }
  }

  \\red[0-9]+
  \\green[0-9]+
  \\blue[0-9]+
  \;
}

<LISTBL>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(PREAMBLE);
      la.mode = PREAMBLE;
      // cerr << "\n===== End List Table\n\n";
      la.level.pop_back();
    }
  }

  \{\\list {
    la.brace++;
    BEGIN(LIST);
    la.level.push_back( la.brace - 1 );
    la.mode = LIST;
    // cerr << "\n===== Start LIST\n\n";
  }
}

<LIST>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(PREAMBLE);
      la.mode = PREAMBLE;
      // cerr << "\n===== End LIST\n\n";
      la.level.pop_back();
    }
  }

  \\listtemplateid-?[0-9]+ |
  \\listsimple |
  \\listlevel |
  \\levelnfc[0-9]+ |
  \\levelnfcn[0-9]+ |
  \\leveljc[0-9]+ |
  \\leveljcn[0-9]+ |
  \\levelfollow[0-9]+ |
  \\levelstartat[0-9]+ |
  \\levelspace[0-9]+ |
  \\levelindent[0-9]+ |
  \\leveltext[^;]*; |
  \\u-?[0-9]+ |
  \\levelnumbers[^;]*; |
  \\f[0-9]+ |
  \\chbrdr |
  \\brdrnone |
  \\brdrcf[0-9]+ |
  \\chshdng[0-9]+ |
  \\chcfpat[0-9]+ |
  \\chcbpat[0-9]+ |
  \\fbias[0-9]+ |
  \\fi-?[0-9]+ |
  \\li[0-9]+ |
  \\jclisttab |
  \\tx[0-9]+ |
  \\listname\ [^}]*; |
  \\listid[0-9]+ |
  \\b[0-9]+ |
  \\i[0-9]+ |
  \\fs[0-9]+ |
  \\levelold |
  \\lin[0-9]+\ *
}

<LISTOVER>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(PREAMBLE);
      la.mode = PREAMBLE;
      // cerr << "\n===== End LISTOVER\n\n";
      la.level.pop_back();
    }
  }

  \\listid-?[0-9]+ |
  \\listoverridecount[0-9]+ |
  \\ls[0-9]
}

<RSIDTBL>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(PREAMBLE);
      la.mode = PREAMBLE;
      // cerr << "\n===== End List Table\n\n";
      la.level.pop_back();
    }
  }

  \\rsid[0-9]+
}

<GENERATOR>{
  \;\ *\} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(PREAMBLE);
      la.mode = PREAMBLE;
      // cerr << "\n===== End List Table\n\n";
      la.level.pop_back();
      cout << "\n";
    }
  }

  \\rsid[0-9]+
}

<INFO>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(PREAMBLE);
      la.mode = PREAMBLE;
      // cerr << "\n===== End Info\n\n";
      la.level.pop_back();
    }
  }

  "\{\\title " {
    BEGIN(TITLE);
    la.brace++;
    la.level.push_back( la.brace - 1 );
    // cerr << "\n===== Start Title\n\n";
    cout << "% Title: ";
  }

  "\{\\author " {
    BEGIN(AUTHOR);
    la.brace++;
    la.level.push_back( la.brace - 1 );
    // cerr << "\n===== Start Author\n\n";
    cout << "% Author: ";
  }

  "\{\\operator " {
    BEGIN(OPERATOR);
    la.brace++;
    la.level.push_back( la.brace - 1 );
    // cerr << "\n===== Start Operator\n\n";
    cout << "% Operator: ";
  }

  "\{\\creatim" {
    BEGIN(TIME);
    la.brace++;
    la.level.push_back( la.brace - 1 );
    cout << "% Created: ";
  }

  "\{\\revtim" {
    BEGIN(TIME);
    la.brace++;
    la.level.push_back( la.brace - 1 );
    cout << "% Revised: ";
  }

  "\{\\printim" {
    BEGIN(TIME);
    la.brace++;
    la.level.push_back( la.brace - 1 );
    cout << "% Printed: ";
  }

  \\version[0-9]+ {
    istringstream i( YYText(), YYLeng() );
    unsigned c;
    i.seekg( 8, ios_base::beg );
    i >> c;
    cout << "% Version: " << c << "\n";
  }

  \\edmins[0-9]+ {
    istringstream i( YYText(), YYLeng() );
    unsigned c;
    i.seekg( 7, ios_base::beg );
    i >> c;
    cout << "% Edmins (?): " << c << "\n";
  }

  \\nofpages[0-9]+ {
    istringstream i( YYText(), YYLeng() );
    unsigned c;
    i.seekg( 9, ios_base::beg );
    i >> c;
    cout << "% Pages: " << c << "\n";
  }

  \\nofwords[0-9]+ |
  \\nofchars[0-9]+ |
  \\nofcharsws[0-9]+ |
  \\vern[0-9]+

  "\{\\*\\company " {
    BEGIN(COMPANY);
    la.brace++;
    la.level.push_back( la.brace - 1 );
    cout << "% Company: ";
  }
}

<TITLE>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(INFO);
      la.mode = INFO;
      // cerr << "\n===== End Title\n\n";
      la.level.pop_back();
      cout << "\n";
    }
  }
}

<AUTHOR>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(INFO);
      la.mode = INFO;
      // cerr << "\n===== End Author\n\n";
      la.level.pop_back();
      cout << "\n";
    }
  }
}

<OPERATOR>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(INFO);
      la.mode = INFO;
      // cerr << "\n===== End Operator\n\n";
      la.level.pop_back();
      cout << "\n";
    }
  }
}

<TIME>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(INFO);
      la.mode = INFO;
      la.level.pop_back();
      cout << "\n";
    }
  }
  \\yr[0-9]{4} {
    istringstream i( YYText(), YYLeng() );
    unsigned c;
    i.seekg( 3, ios_base::beg );
    i >> c;
    cout << c;
  }
  \\mo[0-9]{1,2} {
    istringstream i( YYText(), YYLeng() );
    unsigned c;
    i.seekg( 3, ios_base::beg );
    i >> c;
    cout << "-" << setw(2) << setfill('0') << c;
  }
  \\dy[0-9]{1,2} {
    istringstream i( YYText(), YYLeng() );
    unsigned c;
    i.seekg( 3, ios_base::beg );
    i >> c;
    cout << "-" << setw(2) << setfill('0') << c;
  }
  \\hr[0-9]{1,2} {
    istringstream i( YYText(), YYLeng() );
    unsigned c;
    i.seekg( 3, ios_base::beg );
    i >> c;
    cout << " " << setw(2) << setfill('0') << c;
  }
  \\min[0-9]{1,2} {
    istringstream i( YYText(), YYLeng() );
    unsigned c;
    i.seekg( 4, ios_base::beg );
    i >> c;
    cout << ":" << setw(2) << setfill('0') << c;
  }
}

<COMPANY>{
  \} {
    la.brace--;
    if ( la.brace == la.level.back() ) {
      BEGIN(INFO);
      la.mode = INFO;
      la.level.pop_back();
      cout << "\n";
    }
  }
}

%%

void parse()
{
  yyFlexLexer l( &cin, &cout );

  while ( l.yylex() != 0 ) {
    ;
  }
}
