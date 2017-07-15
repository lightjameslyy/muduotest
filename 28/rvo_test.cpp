#include <iostream>

using namespace std;

struct Foo   
{   
	Foo() { cout << "Foo ctor" << endl; }
	Foo(const Foo&) { cout << "Foo copy ctor" << endl; }
	void operator=(const Foo&) { cout << "Foo operator=" << endl; }
	~Foo() { cout << "Foo dtor" << endl; }
};  

Foo make_foo()   
{   
	Foo f;//构造函数
	return f;//如果没有rvo优化的话，这里返回要调用拷贝构造函数，rvo优化直接把该对象返回回去，相当于这个对象被提升了
				//提升成不是一个局部对象，vs debug底下没有rvo优化,release就有rvo优化
	//return Foo();  
}
  
int main(void)
{
	make_foo();
	return 0;
}
