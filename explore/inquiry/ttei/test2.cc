#include <iostream>

struct X 
{ 
        template<typename T> 
        struct Y 
        { 
                typedef char type; 
        }; 


        // template<> struct Y<int>;
        typedef Y<int>::type type; 
}; 


template<>
struct X::Y<int> 
{ 
        typedef int type; 
}; 


 

int main(int argc, char* argv[]) 
{ 
        std::cout << typeid(X::type).name() << std::endl; 
        return 0; 
} 
