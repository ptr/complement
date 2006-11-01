/*
 * The conversation with Matti Rintala on STLport forum 2005-08-24:
 *
 * Do you mean ISO/IEC 14882 3.6.3 [basic.start.term]? 
 *
 * Yes. "Destructors (12.4) for initialized objects of static storage duration
 * (declared at block scope or at namespace scope) are called as a result
 * of returning from main and as a result of calling exit (18.3). These objects
 * are destroyed in the reverse order of the completion of their constructor
 * or of the completion of their dynamic initialization."
 *
 * I found a confirmation on the web that gcc may not strictly conform
 * to this behaviour in certains cases unless -fuse-cxa-atexit is used.
 *
 * Test below give (without -fuse-cxa-atexit)

Init::Init()
Init::use_it
It ctor done
Init::use_it done
Init ctor done
It dtor done    <--
Init dtor done  <--

 * but should:

Init::Init()
Init::use_it
It ctor done
Init::use_it done
Init ctor done
Init dtor done  <--
It dtor done    <--

 */
#include <iostream>

using namespace std;

class Init
{
  public:
    Init();
    ~Init();

    static void use_it();
};

class Init2
{
  public:
    Init2();
    ~Init2();

};

static Init init;
static Init2 init2;

class It
{
  public:
    It();
    ~It();
};

Init::Init()
{
  cerr << "Init::Init()" << endl;
  use_it();
  cerr << "Init ctor done" << endl;
}

Init::~Init()
{
  cerr << "Init dtor done" << endl;
}

void Init::use_it()
{
  cerr << "Init::use_it" << endl;

  static It it;

  cerr << "Init::use_it done" << endl;
}

Init2::Init2()
{
  cerr << "Init2 ctor done" << endl;
}

Init2::~Init2()
{
  cerr << "Init2 dtor done" << endl;
}

It::It()
{
  cerr << "It ctor done" << endl;
}

It::~It()
{
  cerr << "It dtor done" << endl;
}

int main()
{
  return 0;
}
