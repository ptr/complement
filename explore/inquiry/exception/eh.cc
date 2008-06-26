// -*- C++ -*- Time-stamp: <05/03/28 22:42:50 ptr>
// $Id$

int main()
{
  try {
    throw 1;
  }
  catch ( int i ) {
  }
  catch ( ... ) {
  }

  return 0;
}

