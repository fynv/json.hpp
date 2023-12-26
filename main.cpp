#include "json.hpp"

int main()
{
	auto obj = JObject::New();
	auto str = JString::New("Hello World");
	obj->value()["title"] = str;
	auto lst = JArray::New();
	lst->value() = { JNumber::New(10.0), JNumber::New(20.0), JNumber::New(30.0) };
	obj->value()["list"] = lst;


	auto jsonstr = Strigify(obj);
	auto t = Parse(jsonstr.c_str());
	jsonstr = Strigify(t);

	FILE* fp = fopen("test.json", "w");
	fprintf(fp, "%s\n", jsonstr.c_str());
	fclose(fp);


	/*auto t = Parse("{\"list\": [\"Hello World\", 1234.5]}");
	printf("%f\n", t->To<JObject>()->value()["list"]->To<JArray>()->value()[1]->To<JNumber>()->value());*/
	
	return 0;
}

