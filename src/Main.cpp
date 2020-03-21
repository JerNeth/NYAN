#include "LinAlg.h"
#include <iostream>
#include "Transform.h"
using namespace std;
using namespace bla;
std::tuple<int, int, int> tst() {
	return std::make_tuple<int, int, int>(0, 0, 0);
}
struct MixedData
{
	char Data1;
	short Data2;
	int Data3;
	char Data4;
};
struct AlignedMixedData
{
	char Data1;
	char Data4;
	short Data2;
	int Data3;
};
int main()
{
	
	auto [a, b, c] = tst();
	vec2 test({ 1,2 });
	//bla::vec2 test2( 1,2 );
	const float tmp = 2.f;
	vec2 test2;
	new(&test2) vec2(2);
	//bla::vec2 b = bla::vec2(2.f);
	vec2 b2 = vec2(tmp);
	cout << "Hello World" << endl;
	cout << "sizeof(vec2) " << sizeof(vec2) << endl;
	cout << "sizeof(vec3) " << sizeof(vec3) << endl;
	cout << "sizeof(vec4) " << sizeof(vec4) << endl;

	cout << "sizeof(MixedData) " << sizeof(MixedData) << endl;
	cout << "sizeof(AlignedMixedData) " << sizeof(AlignedMixedData) << endl;
	cout << "sizeof(Transform) " << sizeof(Transform) << endl;
	return 0;
}
