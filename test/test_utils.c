#include "../utils.c"
#include <assert.h>


int main() {
    char s1[] = " 123345 \t\n\r ";
    assert(mystrcmp("12345", "23456") ==  -1);
    assert(mystrcmp("12345", "02345") ==  1);
    assert(mystrcmp("12345", "12345") ==  0);
    assert(mystrcmp(mylstrip(s1, ""), "123345 \t\n\r ") ==  0);
    assert(mystrcmp(myrstrip(s1, ""), " 123345") ==  0);
    assert(mystrcmp(mystrip(s1, ""), "123345") ==  0);
    assert(mystrcmp(myrstrip(s1, ""), " 123345 ") ==  0);
}

