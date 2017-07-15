#include <set>
#include <iostream>

using namespace std;

int main(void)
{
	int a[] = { 1, 2, 3, 4, 5};
	set<int> s(a, a+5);//自动排序

	cout<<*s.lower_bound(2)<<endl;//返回第一个值大于等于2的位置
	cout<<*s.upper_bound(2)<<endl;//返回第一个值大于2的位置
	return 0;
}
