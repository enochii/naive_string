#pragma once

// Your code here
#include <utility>
#include <memory>
#include <iostream>
/*������м��'\0'����'?'����*/


class MyString
{//------------------����������ˮƽ��ֻΪ�±������ʵ��COW-------------------------
private:
	//�����ڴ�ĳ�Ա
	static std::allocator<char> alloc;
	//ָ���ڴ�Ŀ�ͷ����β��λ��
	char *_beg, *_end;
	//���ߺ���������ڴ�ʱ����
	std::pair<char *, char *>alloc_n_copy(const char *, const char *);
	//Ϊ�ǿ�ʱ��ʾ�Լ���û���ڴ棬ֻ�ǿ������Mystring��ָ���Ա
	MyString *copying_others;
	//�Լ��Ƿ���ǳ���ƣ�Ĭ���޹���ʱ�ÿգ��˴�Ϊ�˼򻯣�ĳһʱ��ֻ����һ��stringǳ����
	MyString *be_copyed;
	void reallocate(std::size_t sz);
	void free();
	void free_or_not();
	void chk_s_copy(const MyString &s);
public:
	//���ַ���ָ�빹��string
	MyString(const char *s);
	//Ĭ�Ϲ��캯��
	MyString():_beg(0),_end(0),copying_others(0),be_copyed(0){}
	//�������캯��
	MyString(const MyString &s);
	//������ֵ�����
	MyString& operator=(const MyString &s);
	//��������
	~MyString() { free_or_not(); }
	std::size_t size()const { return _end - _beg; }
	//�±������
	const char & operator[](std::size_t i)const;
	char & operator[](std::size_t i);

	char *begin()const { return _beg;}
	char *end()const { return _end; }

	MyString &append(const MyString &s);

};
//��̬��Ա�����ⶨ��
std::allocator<char> MyString::alloc;

char & MyString::operator[](std::size_t i)
{
	chk_s_copy(*this);

	return *(_beg + i);
}
//һ�������Ƿ������ͷ��ڴ�ĺ���
void MyString::free_or_not()
{
	//��this��ռһ���ڴ棬ֱ���ͷ�
	if (!be_copyed && !copying_others)
		free();
	//���this�������������ڴ棬����ڴ����Ȩת���ö���
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
	if (&s == this)return *this;//�Ը�ֵ

	chk_s_copy(s);//s���غ�ӵ��һ������ڴ�
	
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
	//������Ա�֤ĳһʱ��ֻ��һ������ǳ���ƣ����Ҳ������Ƕ��ǳ���Ƶ��������Ȼ�߼�̫����
	std::pair<char *, char *> data = alloc_n_copy(s._beg, s._end);
	if (s.copying_others)//sǳ���Ʊ�Ķ���
	{	
		MyString &s1 = const_cast<MyString&>(s);
		s1._beg = data.first;
		s1._end = data.second;

		s1.copying_others->be_copyed = 0;//��ձ�־
		s1.copying_others = 0;
	}
	else if (s.be_copyed)//������ǳ����
	{
		s.be_copyed->_beg = data.first;
		s.be_copyed->_end = data.second;
		//��ձ�־
		s.be_copyed->copying_others = 0;
		const_cast<MyString&>(s).be_copyed = 0;
	}
}

MyString::MyString(const MyString &s)
{
	chk_s_copy(s);//���غ�s�Լ�����һ������ڴ�

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
	//�ƶ�ԭ�е�Ԫ��
	for (/**/;ele != _end;++ele)
		alloc.construct(dest++, std::move(*ele));

	free();//�ͷ�ԭ���ڴ�
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

//�����ò���Ҫ����COW
const char & MyString::operator[](std::size_t i)const
{
	return *(_beg + i);
}

//����<<�����
std::ostream & operator<<(std::ostream &os, const MyString &s)
{
	for (char *p = s.begin();p != s.end();p++)
		os << (*p == '\0' ? '?' : *p);

	return os;
}

std::pair<char *, char *> MyString::alloc_n_copy(const char *b, const char *e)
{
	if (!b || !e)return std::make_pair(nullptr, nullptr);
	auto newdata = alloc.allocate(e - b);//����Ԫ��
	//��uninitialized_copy������Ӧ��Χ��Ԫ��
	char *_e = newdata;
	const char *i = b;
	//return std::make_pair(newdata, std::uninitialized_copy(b, e, newdata));
	for (;i != e;i++)
		alloc.construct(_e++, *i);

	return std::make_pair(newdata, _e);
}

void MyString::free()
{
	//û���ڴ�Ҫ�ͷ�
	if (_beg == _end)return;

	for (auto p = _end;p != _beg;/**/)
		alloc.destroy(--p);
	
	alloc.deallocate(_beg, _end - _beg);
}

//�˴����贫���s�ǺϷ��ģ�����һ��'\0'Ϊ��β
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