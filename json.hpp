#pragma once

#include <memory>
#include <cctype>
#include <string>
#include <vector>
#include <list>
#include <functional>
#include <unordered_map>
#include <cstring>
#include <cstdio>

enum class Type
{
	Null,
	Number,
	Boolean,
	String,
	Array,
	Object
};

class JToken;
typedef std::shared_ptr<JToken> SPToken;

class JToken
{
public:
	virtual Type type() const = 0;
	
	template<typename T>
	T* To()
	{
		return dynamic_cast<T*>(this);
	}

protected:
	JToken(){}
	virtual ~JToken(){}
};


class JNull;
typedef std::shared_ptr<JNull> SPNull;

class JNull : public JToken
{
public:
	static SPNull New()
	{
		return SPNull(new JNull);
	}

	Type type() const override
	{
		return Type::Null;
	}
	
protected:
	JNull()
	{

	}
};

class JNumber;
typedef std::shared_ptr<JNumber> SPNumber;

class JNumber : public JToken
{
public:
	static SPNumber New(double v = 0.0)
	{
		return SPNumber(new JNumber(v));
	}

	Type type() const override
	{
		return Type::Number;
	}
	
	double& value()
	{
		return m_value;
	}

	const double& value() const
	{
		return m_value;
	}

protected:
	double m_value;
	JNumber(double v): m_value(v)
	{

	}
};

class JBoolean;
typedef std::shared_ptr<JBoolean> SPBoolean;

class JBoolean : public JToken
{
public:
	static SPBoolean New(bool v = false)
	{
		return SPBoolean(new JBoolean(v));
	}

	Type type() const override
	{
		return Type::Boolean;
	}

	bool& value()
	{
		return m_value;
	}

	const bool& value() const
	{
		return m_value;
	}

protected:
	bool m_value;
	JBoolean(bool v) : m_value(v)
	{

	}
};


class JString;
typedef std::shared_ptr<JString> SPString;

class JString : public JToken
{
public:
	static SPString New(const char* str = "")
	{
		return SPString(new JString(str));
	}

	Type type() const override
	{
		return Type::String;
	}

	std::string& value()
	{
		return m_value;
	}

	const std::string& value() const
	{
		return m_value;
	}

protected:
	std::string m_value;
	JString(const char* str) : m_value(str)
	{
		
	}
};

class JArray;
typedef std::shared_ptr<JArray> SPArray;

class JArray : public JToken
{
public:
	static SPArray New(size_t size = 0)
	{
		return SPArray(new JArray(size));
	}

	Type type() const override
	{
		return Type::Array;
	}

	std::vector<SPToken>& value()
	{
		return m_value;
	}

	const std::vector<SPToken>& value() const
	{
		return m_value;
	}

protected:
	std::vector<SPToken> m_value;
	JArray(size_t size) : m_value(size)
	{

	}
};

class JObject;
typedef std::shared_ptr<JObject> SPObject;

class JObject : public JToken
{
public:
	static SPObject New()
	{
		return SPObject(new JObject);
	}

	Type type() const override
	{
		return Type::Object;
	}

	std::unordered_map<std::string, SPToken>& value()
	{
		return m_value;
	}

	const std::unordered_map<std::string, SPToken>& value() const
	{
		return m_value;
	}

protected:
	std::unordered_map<std::string, SPToken> m_value;
	JObject(){}

};

inline std::string Strigify(SPToken token)
{
	std::string str;
	
	int indent = 0;
	std::function<void(SPToken)> func;

	func = [&func, &str, &indent](SPToken token) {
		if (token->type() == Type::Null)
		{
			str += "null";
		}
		else if (token->type() == Type::Number)
		{
			char buf[256];
			sprintf(buf, "%f", token->To<JNumber>()->value());
			str += buf;
		}
		else if (token->type() == Type::Boolean)
		{
			str += token->To<JBoolean>()->value() ? "true" : "false";
		}
		else if (token->type() == Type::String)
		{
			str += std::string("\"") + token->To<JString>()->value() + "\"";
		}
		else if (token->type() == Type::Array)
		{
			auto& arr = token->To<JArray>()->value();
			str += "[\n";
			indent++;
			for (size_t i = 0; i < arr.size(); i++)
			{				
				for (int j = 0; j < indent; j++)
				{					
					str += "\t";
				}				
				func(arr[i]);
				if (i < arr.size() - 1)
				{
					str += ",\n";
				}
			}
			str += "]\n";
			indent--;
		}
		else if (token->type() == Type::Object)
		{
			auto obj = token->To<JObject>()->value();
			str += "{\n";
			indent++;
			auto iter = obj.begin();
			while (iter != obj.end())
			{
				for (int j = 0; j < indent; j++)
				{
					str += "\t";
				}
				str += std::string("\"") + iter->first + "\": ";
				func(iter->second);
				iter++;
				if (iter != obj.end())
				{
					str += ",\n";
				}
			}

			str += "}\n";
			indent--;
		}
	};
	func(token);
	return str;
}

inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
}

inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

inline void trim(std::string& s) {
	rtrim(s);
	ltrim(s);
}

inline SPToken Parse(const char* str)
{
	static std::unordered_map<char, char> match_list = {
		{'\"', '\"'},
		{'[', ']'},
		{'{', '}'},
	};

	std::string s = str;
	trim(s);

	if (s.length() < 1)
	{
		return JNull::New();
	}

	if (s[0] == '\"')
	{
		std::string sub = s.substr(1, s.length() - 2);
		return JString::New(sub.c_str());
	}
	
	if (s[0] == '[')
	{
		SPArray arr = JArray::New();
		std::string inside = s.substr(1, s.length() - 2);
		std::list<char> stack;
		int start = 0;
		int end = start;
		for (; end <= inside.length(); end++)
		{
			if (end < inside.length())
			{
				char c = inside[end];
				if (stack.size() > 0 && (c == '\"' || c == ']' || c == '}'))
				{
					char c_top = stack.front();
					if (c_top == match_list[c])
					{
						stack.pop_front();
					}
					continue;
				}
				
				if (c == '\"' || c == '[' || c == '{')
				{
					stack.push_front(c);					
					continue;
				}
				
				if (stack.size()>0 || c != ',')
				{
					continue;
				}
			}

			std::string sub = inside.substr(start, end - start);			
			arr->value().emplace_back(Parse(sub.c_str()));
			start = end + 1;
			end = start;		
			
		}
		return arr;
	}
	
	if (s[0] == '{')
	{
		SPObject obj = JObject::New();
		std::string inside = s.substr(1, s.length() - 2);
		std::list<char> stack;
		int start = 0;
		int end = start + 1;
		for (; end <= inside.length(); end++)
		{
			if (end < inside.length())
			{
				char c = inside[end];
				if (stack.size() > 0 && (c == '\"' || c == ']' || c == '}'))
				{
					char c_top = stack.front();
					if (c_top == match_list[c])
					{
						stack.pop_front();
					}
					continue;
				}

				if (c == '\"' || c == '[' || c == '{')
				{
					stack.push_front(c);
					continue;
				}

				if (stack.size() > 0 || c != ',')
				{
					continue;
				}
			}

			std::string sub = inside.substr(start, end - start);			
			size_t mid = sub.find(':', 0);
			if (mid == std::string::npos)
			{
				if (sub.length() > 1 && sub[0] == '\"')
				{
					obj->value()[sub.substr(1, sub.length() - 2)] = JNull::New();
				}
				else
				{
					obj->value()[sub] = JNull::New();
				}
			}
			else
			{
				std::string key_s = sub.substr(0, mid);
				trim(key_s);
				std::string value = sub.substr(mid + 1);
				if (key_s.length() > 1 && key_s[0] == '\"')
				{
					obj->value()[key_s.substr(1, key_s.length() - 2)] = Parse(value.c_str());
				}
				else
				{
					obj->value()[key_s] = Parse(value.c_str());
				}
			}			
			start = end + 1;
			end = start;
		}

		return obj;

	}
	
	if (s == "null")
	{
		return JNull::New();
	}
	if (s == "true")
	{
		return JBoolean::New(true);
	}
	
	if (s == "false")
	{
		return JBoolean::New(false);
	}
	
	return JNumber::New(atof(s.c_str()));
}
