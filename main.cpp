#include "json.hpp"

using namespace DataModel;
using namespace Json;

int main()
{	
	auto obj = Object::New({
		{"title", "Hello\t World\n 天地"},
		{"list", { 10.0, 20.0, 30.0 }},
		{"obj", ObjectT({
			{ "item1", 10.0},
			{ "item2", "cheers!"}
		})}
	});

	auto jsonstr = Strigify(obj, true);
	auto t = Parse(jsonstr.c_str());
	jsonstr = Strigify(t);
	
	FILE* fp = fopen("test.json", "w");
	fprintf(fp, "%s\n", jsonstr.c_str());
	fclose(fp);

	/*FILE* fp = fopen("test_parsing/y_string_allowed_escapes.json", "rb");
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	std::vector<char> buf(size + 1, 0);
	fseek(fp, 0, SEEK_SET);
	fread(buf.data(), 1, size, fp);
	fclose(fp);

	auto t = Parse(buf.data());
	auto jsonstr = Strigify(t);

	{
		FILE* fp = fopen("test.json", "w");
		fprintf(fp, "%s\n", jsonstr.c_str());
		fclose(fp);
	}*/

	return 0;
}

