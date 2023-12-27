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

namespace DataModel
{
	template<typename T> class TToken;

	class Token
	{
	public:
		template<typename T>
		T& To()
		{
			return *dynamic_cast<T*>(this);
		}

		template<typename T>
		const T& To() const
		{
			return *dynamic_cast<const T*>(this);
		}

		template<typename T>
		T& ToValue()
		{
			return To<TToken<T>>().value();
		}

		template<typename T>
		const T& ToValue() const
		{
			return To<TToken<T>>().value();
		}

	protected:
		Token(){}
		virtual ~Token(){}
	};
	typedef std::shared_ptr<Token> SPToken;

	template <typename T>
	class TTokenPtr : public std::shared_ptr<TToken<T>>
	{
	public:
		using std::shared_ptr<TToken<T>>::shared_ptr;

		T& operator*() const
		{
			return std::shared_ptr<TToken<T>>::get()->value();
		}

		T* operator->() const
		{
			return &std::shared_ptr<TToken<T>>::get()->value();
		}
	};

	template<typename T>
	class TToken : public Token
	{
	public:
		static TTokenPtr<T> New(T&& v = T())
		{
			return TTokenPtr<T>(new TToken<T>(T(v)));
		}

		T& value()
		{
			return m_value;
		}

		const T& value() const
		{
			return m_value;
		}
		
	protected:
		T m_value;
		TToken(T&& v) : m_value(v)
		{

		}
	};

	typedef double NumberT;
	typedef TToken<NumberT> Number;
	typedef TTokenPtr<NumberT> SPNumber;
	typedef bool BooleanT;
	typedef TToken<BooleanT> Boolean;
	typedef TTokenPtr<BooleanT> SPBoolean;
	typedef std::string StringT;
	typedef TToken<StringT> String;
	typedef TTokenPtr<StringT> SPString;
	typedef std::vector<SPToken> ArrayT;
	typedef TToken<ArrayT> Array;
	typedef TTokenPtr<ArrayT> SPArray;
	typedef std::unordered_map<std::string, SPToken> ObjectT;
	typedef TToken<ObjectT> Object;
	typedef TTokenPtr<ObjectT> SPObject;

	// for extensions, not used
	class Wrapper : public Token
	{
	public:
		void* ptr()
		{
			return m_cptr;
		}

	protected:
		void* m_cptr;
		Wrapper(void* cptr) : m_cptr(cptr)
		{

		}
	};
	typedef std::shared_ptr<Wrapper> SPWrapper;
}

namespace Json
{
	using namespace DataModel;

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

	inline std::string Strigify(SPToken token, int indent = 0)
	{
		if (token == nullptr)
		{
			return "null";
		}

		{
			Number* t = dynamic_cast<Number*>(token.get());
			if (t != nullptr)
			{
				char buf[256];
				sprintf(buf, "%f", t->value());
				return buf;
			}
		}

		{
			Boolean* t = dynamic_cast<Boolean*>(token.get());
			if (t != nullptr)
			{
				return t->value() ? "true" : "false";
			}
		}

		{
			String* t = dynamic_cast<String*>(token.get());
			if (t != nullptr)
			{
				// TODO: need escape here
				return std::string("\"") + t->value() + "\"";
			}
		}

		{
			Array* t = dynamic_cast<Array*>(token.get());
			if (t != nullptr)
			{
				const auto& arr = t->value();
				std::string str = "[\n";
				indent++;
				for (size_t i = 0; i < arr.size(); i++)
				{
					for (int j = 0; j < indent; j++)
					{
						str += "\t";
					}
					str += Strigify(arr[i], indent);
					if (i < arr.size() - 1)
					{
						str += ",\n";
					}
				}
				indent--;
				str += "\n";
				for (int j = 0; j < indent; j++)
				{
					str += "\t";
				}
				str += "]";
				return str;
			}
		}

		{
			Object* t = dynamic_cast<Object*>(token.get());
			if (t != nullptr)
			{
				const auto& obj = t->value();
				std::string str = "{\n";
				indent++;
				auto iter = obj.begin();
				while (iter != obj.end())
				{
					for (int j = 0; j < indent; j++)
					{
						str += "\t";
					}
					
					// TODO: need escape here
					str += std::string("\"") + iter->first + "\": ";

					str += Strigify(iter->second, indent);					
					iter++;
					if (iter != obj.end())
					{
						str += ",\n";
					}
				}
				indent--;
				str += "\n";
				for (int j = 0; j < indent; j++)
				{
					str += "\t";
				}
				str += "}";
				return str;
			}
		}
		return "null";
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
			return nullptr;
		}

		if (s[0] == '\"')
		{
			std::string sub = s.substr(1, s.length() - 2);
			// TODO: need unescape here
			return String::New(sub.c_str());
		}

		if (s[0] == '[')
		{
			auto arr = Array::New();
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
						if (c == match_list[c_top])
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
						if (c == '\\')
						{
							end++;
						}
						continue;
					}
				}

				std::string sub = inside.substr(start, end - start);
				arr->emplace_back(Parse(sub.c_str()));
				start = end + 1;
				end = start;
			}

			return arr;
		}

		if (s[0] == '{')
		{
			SPObject obj = Object::New();
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
						if (c == match_list[c_top])
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
						if (c == '\\')
						{
							end++;
						}
						continue;
					}
				}

				std::string sub = inside.substr(start, end - start);

				int qcount = 0;
				size_t key_start = 0;
				size_t key_end = std::string::npos;
				size_t value_start = std::string::npos;
				for (size_t i = 0; i < sub.size(); i++)
				{
					char c = sub[i];
					if (c == '\\')
					{
						i++;
						continue;
					}

					if (c == '\"')
					{
						if (qcount == 0)
						{
							key_start = i + 1;
						}
						else if (qcount == 1)
						{
							key_end = i;
						}
						qcount++;
						continue;
					}

					if (qcount == 2 && c == ':')
					{
						value_start = i + 1;
						break;
					}
				}

				size_t key_len;
				if (key_end == std::string::npos)
				{
					key_len = sub.length() - key_start;
				}
				else
				{
					key_len = key_end - key_start;
				}

				std::string key = sub.substr(key_start, key_len);
				// TODO: need unescape here

				std::string value = sub.substr(value_start);								
				(*obj)[key] = Parse(value.c_str());

				start = end + 1;
				end = start;
			}

			return obj;
		}

		if (s == "null")
		{
			return nullptr;
		}
		if (s == "true")
		{
			return Boolean::New(true);
		}

		if (s == "false")
		{
			return Boolean::New(false);
		}

		return Number::New(atof(s.c_str()));
	}
}

