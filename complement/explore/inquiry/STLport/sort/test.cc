// -*- C++ -*- Time-stamp: <04/06/06 14:00:39 ptr>

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include <cstring>

#define stricmp strcasecmp

class test
{
  public:
    test(std::string str) :
        str_(str)
      { }
    char const *c_str() const
      { return str_.c_str(); }

    bool operator <(test const* rhs) const
      { return stricmp(c_str(), rhs->c_str()) < 0; }

  private:
    std::string str_;
};



namespace _STL /* std */ {

template<>
struct less<test *> : public binary_function<test *, test *, bool>
{
    bool operator()(test const*lhs, test const*rhs) const
      { return stricmp(lhs->c_str(), rhs->c_str()) < 0; }
};

}


void printDelimeter(){
  std::cout<<"=================================\n";
}


void output(test const* obj)
{ std::cout << obj->c_str() << std::endl; }


int main( int, char * const * )
{
  std::string strings[] = {
    "Vol'ery (Nomera)",
    "Vol'ery (Priemnaq)",
    "Vol'ery (Tropinka)",
    "KP_Alleq (Ukazatel')",
    "KP_Bunker (Doska po4eta)",
    "KP_Bunker (Termometr)",
    "KP_Nomera (Tabli4ka-1)",
    "KP_Nomera (Tabli4ka-2)",
    "KP_Nomera (Tabli4ka-3)",
    "KP_Nomera (Tabli4ka-4)",
    "KP_Nomera (Tabli4ka-5)",
    "KP_Plats (Ob'qvlenie)",
    "Park (Alleq) (1)",
    "Park (Alleq) (2)",
    "Park (Vorota)",
    "Park (Most)",
    "Poligon (Bunker)",
    "Poligon (Panteon)",
    "Poligon (Plats)",
  };


  std::vector<test*> vec;
  for( int i = 0; i < sizeof(strings)/sizeof(std::string); ++i ) {
    vec.push_back(new test(strings[i]));
  }



  std::for_each(vec.begin(), vec.end(), output);
  printDelimeter();       
  std::sort(vec.begin(), vec.end());
  std::for_each(vec.begin(), vec.end(), output);

  return 0;
}
