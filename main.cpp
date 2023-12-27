#include "json.hpp"

using namespace DataModel;
using namespace Json;

int main()
{
	auto s = String::New("Hello World");
	auto a = Array::New();
	*a = { Number::New(10.0), Number::New(20.0), Number::New(30.0) };

	auto obj = Object::New();
	(*obj)["title"] = s;
	(*obj)["list"] = a;

	auto jsonstr = Strigify(obj);
	auto t = Parse(jsonstr.c_str());
	jsonstr = Strigify(t);
	
	FILE* fp = fopen("test.json", "w");
	fprintf(fp, "%s\n", jsonstr.c_str());
	fclose(fp);
	
	return 0;
}

