#include <iostream>
#include <functional>
#include <utility>

using namespace std;

void (*func) () = []() ->void 
{
    printf("This is a function with empty parameter list, you may call this function with any parameter you like\n");
};

int main()
{
    // 这种调用方法在C++中是不允许的，但是在C语言中是可以编译通过的
    // func(20);
}