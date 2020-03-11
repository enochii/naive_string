#pragma once

// Your code here
#include <utility>
#include <memory>
#include <iostream>
/*如果是中间的'\0'则以'?'代替*/


class MyString
{//------------------声明：限于水平，只为下标运算符实现COW-------------------------
private:
	//申请内存的成员
	static std::allocator<char> alloc;
	//指向内存的开头，和尾后位置
	char *_beg, *_end;
	//工具函数，深拷贝内存时调用
	std::pair<char *, char *>alloc_n_copy(const char *, const char *);
	//为非空时表示自己并没有内存，只是拷贝别的Mystring的指针成员
	MyString *copying_others;
	//自己是否被人浅复制，默认无共享时置空，此处为了简化，某一时刻只允许被一个string浅复制
	MyString *be_copyed;
	void reallocate(std::size_t sz);
	void free();
	void free_or_not();
	void chk_s_copy(const MyString &s);
public:
	//从字符串指针构造string
	MyString(const char *s);
	//默认构造函数
	MyString():_beg(0),_end(0),copying_others(0),be_copyed(0){}
	//拷贝构造函数
	MyString(const MyString &s);
	//拷贝赋值运算符
	MyString& operator=(const MyString &s);
	//析构函数
	~MyString() { free_or_not(); }
	std::size_t size()const { return _end - _beg; }
	//下标运算符
	const char & operator[](std::size_t i)const;
	char & operator[](std::size_t i);

	char *begin()const { return _beg;}
	char *end()const { return _end; }

	MyString &append(const MyString &s);

};
//静态成员的类外定义
std::allocator<char> MyString::alloc;

char & MyString::operator[](std::size_t i)
{
	chk_s_copy(*this);

	return *(_beg + i);
}
//一个管理是否真正释放内存的函数
void MyString::free_or_not()
{
	//若this独占一块内存，直接释放
	if (!be_copyed && !copying_others)
		free();
	//如果this与其他对象共享内存，则把内存管理权转交该对象
	else if (be_copyed)
	{
		be_copyed->copying_others = 0;
		be_copyed = 0;
	}
	else if (copying_others)
	{
		copying_others->be_copyed = 0;
		copying_others = 0;
	}
}

MyString& MyString::operator=(const MyString &s)
{
	if (&s == this)return *this;//自赋值

	chk_s_copy(s);//s返回后拥有一块独立内存
	
	free_or_not();

	MyString &s1 = const_cast<MyString&>(s);
	s1.be_copyed = this;
	copying_others = &s1;
	_beg = s1._beg;
	_end = s1._end;

	return *this;
}

void MyString::chk_s_copy(const MyString &s)
{
	if (!s.copying_others && !s.be_copyed)return;
	//代码可以保证某一时刻只被一个对象浅复制，并且不会出现嵌套浅复制的情况，不然逻辑太复杂
	std::pair<char *, char *> data = alloc_n_copy(s._beg, s._end);
	if (s.copying_others)//s浅复制别的对象
	{	
		MyString &s1 = const_cast<MyString&>(s);
		s1._beg = data.first;
		s1._end = data.second;

		s1.copying_others->be_copyed = 0;//清空标志
		s1.copying_others = 0;
	}
	else if (s.be_copyed)//被别人浅复制
	{
		s.be_copyed->_beg = data.first;
		s.be_copyed->_end = data.second;
		//清空标志
		s.be_copyed->copying_others = 0;
		const_cast<MyString&>(s).be_copyed = 0;
	}
}

MyString::MyString(const MyString &s)
{
	chk_s_copy(s);//返回后s自己享有一块独立内存

	MyString &s1 = const_cast<MyString&>(s);
	s1.be_copyed = this;
	copying_others = &s1;
	_beg = s1._beg;
	_end = s1._end;
}

void MyString::reallocate(std::size_t sz)
{
	char *p = alloc.allocate(sz);
	char *dest = p;
	char *ele = _beg;
	//移动原有的元素
	for (/**/;ele != _end;++ele)
		alloc.construct(dest++, std::move(*ele));

	free();//释放原有内存
	_beg = p;
	_end = dest;
}

MyString & MyString::append(const MyString &s)
{
	std::size_t newcapacity = s.size() + this->size();
	reallocate(newcapacity);

	for (const char *ele = s._beg;
		ele != s._end;++ele)
		alloc.construct(_end++, *ele);

	return *this;
}

//常引用不需要考虑COW
const char & MyString::operator[](std::size_t i)const
{
	return *(_beg + i);
}

//重载<<运算符
std::ostream & operator<<(std::ostream &os, const MyString &s)
{
	for (char *p = s.begin();p != s.end();p++)
		os << (*p == '\0' ? '?' : *p);

	return os;
}

std::pair<char *, char *> MyString::alloc_n_copy(const char *b, const char *e)
{
	if (!b || !e)return std::make_pair(nullptr, nullptr);
	auto newdata = alloc.allocate(e - b);//分配元素
	//用uninitialized_copy拷贝对应范围的元素
	char *_e = newdata;
	const char *i = b;
	//return std::make_pair(newdata, std::uninitialized_copy(b, e, newdata));
	for (;i != e;i++)
		alloc.construct(_e++, *i);

	return std::make_pair(newdata, _e);
}

void MyString::free()
{
	//没有内存要释放
	if (_beg == _end)return;

	for (auto p = _end;p != _beg;/**/)
		alloc.destroy(--p);
	
	alloc.deallocate(_beg, _end - _beg);
}

//此处假设传入的s是合法的，即以一个'\0'为结尾
MyString::MyString(const char *s)
{
	//std::cout << "char*" << std::endl;
	const char *e = s + strlen(s);
	auto data = alloc_n_copy(s, e);
	_beg = data.first;
	_end = data.second;
	copying_others = 0;
	be_copyed = 0;
}

bool operator <(const MyString &l, const MyString &r)
{
	char *p1 = l.begin(), *p2 = r.begin();
	for (/*empty*/;p1 != l.end()&&p2 != r.end();p1++, p2++)
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