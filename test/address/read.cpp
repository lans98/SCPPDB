#include <cstdio>

#include <core/address.hpp>
#include <address/foo.hpp>

using namespace foo;
using namespace address;

void test1() {
    printf("Test 1:\n");

    auto foo = new Foo();
    Address<Foo> address(0UL, nullptr);
    cout << *address.read("foo_data") << '\n';
}

void test2() {
    printf("Test 2:\n");

    auto foo = new Foo();
    Address<Foo> address(sizeof(*foo), nullptr);
    cout << *address.read("foo_data") << '\n';
}

int main() {
    test1();
    test2();
}
