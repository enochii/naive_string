#pragma once

// Your code here
#include <iostream>
#include <cstring>

class MyString
{
private:
	char *_beg, *_end;
	int *shared_num;//引用计数
	//工具函数
	void new_n_copy(const char *b, const char *e);
	//完成实际的释放内存工作
	void _free();
	void free();
	char *renew(std::size_t len);
public:
	~MyString() { free(); }
	MyString():_beg(0),_end(0),shared_num(0){}
	MyString(const char *);

	char *begin() { return _beg; }
	char *end() { return _end; }
	const char *begin()const { return _beg; }
	const char *end()const { return _end; }
	std::size_t size()const { return _end - _beg; }

	MyString(const MyString&);
	MyString& operator=(const MyString&);

	const char& operator[](std::size_t pos)const { return *(_beg + pos); }
	char& operator[](std::size_t);

	MyString& append(const MyString&);
};
MyString::MyString(const char* s)
{
	const char* e = s + std::strlen(s);
	new_n_copy(s, e);
}

char& MyString::operator[](std::size_t pos)
{
	if (*shared_num != 1)
	{
		(*shared_num)--;//注意顺序
		new_n_copy(_beg, _end);//出来时成员已指向新的内存
	}
	return const_cast<char&>(
		static_cast<const MyString&>(*this)
		[pos]
	);
}

MyString& MyString::operator=(const MyString& s)
{
	free();

	if (s.shared_num)++(*s.shared_num);
	shared_num = s.shared_num;
	_beg = s._beg;
	_end = s._end;
	return *this;
}

MyString::MyString(const MyString& s)
{
	if(s.shared_num)++(*s.shared_num);
	shared_num = s.shared_num;
	_beg = s._beg;
	_end = s._end;
}

char *MyString::renew(std::size_t sz)
{
	if (!sz)return 0;
	return new char[sz];
}

MyString& MyString::append(const MyString& s)
{
	if (s.begin() == s.end())return *this;//空串

	char *data = renew(s.size() + size());
	char *dest = data;
	for (const char* ele = begin();
		ele != end();++ele)
		*dest++ = std::move(*ele);
	for (const char* ele = s.begin();
		ele != s.end();++ele)
		*dest++ = *ele;
	free();
	_beg = data;
	_end = dest;
	shared_num = new int(1);
	return *this;
}

void MyString::_free()
{
	delete[]_beg;
	delete shared_num;
	_beg = _end = 0;
	shared_num = 0;
}

void MyString::free()
{
	//没有指向任何内存
	if (!shared_num)return;
	//自己独占一块内存
	if (--(*shared_num) == 0)_free();
}

void MyString::new_n_copy(const char *b, const char *e)
{
	if (b == e)return;
	char *data = new char[e - b];

	char *dest = data;
	const char *ele = b;
	for (;ele != e;++ele)
		*dest++ = *ele;

	_beg = data;
	_end = dest;
	shared_num = new int(1);//申请计数器
}

std::ostream & operator<<(std::ostream &os, const MyString &s)
{
	for (const char *p = s.begin();p != s.end();++p)
		os << (*p == '\0' ? '?' : *p);

	return os;
}

bool operator <(const MyString &l, const MyString &r)
{
	const char *p1 = l.begin(), *p2 = r.begin();
	for (/*empty*/;p1 != l.end() && p2 != r.end();p1++, p2++)
	{
		if (*p1 < *p2)return true;
		else if (*p1 > *p2)return false;
	}
	return r.end() != p2&&l.end() == p1;
}

bool operator >(const MyString &l, const MyString &r)
{
	return r < l;
}

bool operator ==(const MyString &l, const MyString &r)
{
	return !(l < r) && !(l > r);
}

bool operator !=(const MyString &l, const MyString &r)
{
	return !(l == r);
}

bool operator <=(const MyString &l, const MyString &r)
{
	return (l < r) || (l == r);
}

bool operator >=(const MyString &l, const MyString &r)
{
	return (l > r) || (l == r);
}
