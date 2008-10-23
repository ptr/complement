#ifndef __MY_H
#define __MY_H

template <class T>
class A
{
  public:
    A();

    T& use_it();

  private:
    static T val;
};

template <class T>
A<T>::A()
{ }

template <class T>
T& A<T>::use_it()
{ return val; }

template <class T>
T A<T>::val = 0;

#endif // __MY_H
