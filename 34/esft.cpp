#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <cassert>

class Y: public boost::enable_shared_from_this<Y>
{
public:
	boost::shared_ptr<Y> f()
	{
		return shared_from_this();
	}

	Y* f2()
	{
		return this;
	}
};

int main()
{
	boost::shared_ptr<Y> p(new Y);//p的引用计数为1，
	boost::shared_ptr<Y> q = p->f();//引用计数为2
 
	Y* r = p->f2();
	assert(p == q);//正确
	assert(p.get() == r);//p的指针（即new Y)=r

	std::cout<<p.use_count()<<std::endl;//2
	boost::shared_ptr<Y> s(r);//等于又构造了一个独立share_ptr对象,并不是把某个share_ptr对象赋值给另外一个，后者的引用计数才+1=3
	std::cout<<s.use_count()<<std::endl;//1
	assert(p == s);

	return 0;
}

