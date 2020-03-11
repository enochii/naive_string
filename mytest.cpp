#include "mystringwithproxy.h"
#include <iostream>
#include <cassert>
#include <string>

using namespace std;

int main(void) {
#ifdef PROXY_TEST
	MyString s("xixixi");
	MyString s1(s);

	char c = s[0];
	assert(s.begin() == s.begin());

	s[0] = '.';
	assert(s.begin() != s1.begin());
#endif

	string s("xixixi");
	char *p = (&(*s.begin())+1);
	string s1(s);
	*p = 'h';
	cout << s << "  " << s1 << endl;

	return 0;
}