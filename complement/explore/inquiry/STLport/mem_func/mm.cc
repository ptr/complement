#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>

class Person
{ 
  private: 
    std::string name;

  public: 
    //... 
    void print () const
      { std::cout << name << std::endl; }

    void printWithPrefix (std::string prefix) const
      { std::cout << prefix << name << std::endl; }
};
 
void foo (const std::vector<Person>& coll) 
{ 
  using std::for_each; 
  using std::bind2nd; 
  using std::mem_fun_ref; 
 
   // call member function print() for each element 
  for_each( coll.begin(), coll.end(), mem_fun_ref(&Person::print) );
 
  // call member function printWithPrefix() for each element 
  // - "person: " is passed as an argument to the member function 
  for_each( coll.begin(), coll.end(), bind2nd(mem_fun_ref(&Person::printWithPrefix), "person: ") );
  for_each( coll.begin(), coll.end(), bind2nd(mem_fun_ref(&Person::printWithPrefix), std::string("person: ") ) );
}

int main()
{
  return 0;
}
