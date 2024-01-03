#pragma once

#include <memory>
#include <cctype>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <cstdio>

namespace DataModel
{
	template<typename T> class TToken;
	template <typename T> class TTokenPtr;

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

	typedef std::shared_ptr<Token> TokenPtr;

	template<typename T>
	class TToken : public Token
	{
	public:
		static TTokenPtr<T> New(T&& v = T())
		{
			return TTokenPtr<T>(new TToken<T>(T(v)));
		}

		static TTokenPtr<T> New(const T& v)
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

		TToken(const T& v) : m_value(v)
		{

		}
	};

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

	class SPToken : public TokenPtr
	{
	public:
		using TokenPtr::TokenPtr;

		SPToken()
		{
		}

		SPToken(double v)
			: TokenPtr(TToken<double>::New(v))
		{
		}

		SPToken(bool v)
			: TokenPtr(TToken<bool>::New(v))
		{
		}

		SPToken(const char* v)
			: TokenPtr(TToken<std::string>::New(v))
		{
		}

		SPToken(std::initializer_list<SPToken> v)
			: TokenPtr(TToken<std::vector<SPToken>>::New(v))
		{
		}

		SPToken(std::unordered_map<std::string, SPToken>&& v)
			: TokenPtr(TToken<std::unordered_map<std::string, SPToken>>::New(v))
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

	inline int _utf8_to_unicode(const char*& p8)
	{
		int c1 = (int)*(p8++) & 0xFF;
		int u = 0;

		int kind = (int)c1 >> 4;
		if (kind < 0x8) // ascii 7 bit
		{
			u = c1;
		}
		else
		{
			int c2 = (int) * (p8++) & 0xFF;
			if (kind < 0xe) // 5+6=11 bit
				u = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
			else
			{
				int c3 = (int) * (p8++) & 0xFF;
				if (kind < 0xf) // BMP 4 + 6 + 6 = 16bit
					u = ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
				else // 3 + 6 +6 +6 = 21bit 
				{
					int c4 = (int) * (p8++) & 0xFF;
					u = ((c1 & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
				}
			}
		}
		return u;
	}

	inline std::vector<int> _decode_utf8(const char* str)
	{
		std::vector<int> ret;
		const char* p8 = str;
		while (*p8 != 0)
		{
			int code= _utf8_to_unicode(p8);
			ret.push_back(code);
		}
		return ret;
	}

	inline std::string _utf8_from_unicode(int code)
	{
		std::string s;
		int c1;
		if (code < 0x80)
		{
			c1 = code;
		}
		else
		{
			c1 = (code & 0x3F) | 0x80; code >>= 6;
			int c2;
			if (code < 0x20)
				c2 = code | 0xC0;
			else
			{
				c2 = (code & 0x3F) | 0x80; code >>= 6;
				int c3;
				if (code < 0x10)
				{
					c3 = code | 0xE0;
				}
				else
				{
					c3 = (code & 0x3F) | 0x80; code >>= 6;
					int c4;
					c4 = (code & 0x07) | 0xF0;
					s += (char)c4;
				}
				s += (char)c3;
			}
			s += (char)c2;
		}
		s += (char)c1;
		return s;
	}

	inline std::string escape(const std::string& s, bool escape_unicode)
	{
		std::vector<int> unicode = _decode_utf8(s.c_str());

		static std::unordered_map<int, std::string> escape_map = {
			{ '\"', "\\\""},
			{ '\\', "\\\\"},
			{ '\b', "\\b"},
			{ '\f', "\\f"},
			{ '\n', "\\n"},
			{ '\r', "\\r"},
			{ '\t', "\\t"},
		};


		std::string ret;
		for (size_t i = 0; i < unicode.size(); i++)
		{
			int code = unicode[i];
			auto iter = escape_map.find(code);
			if (iter != escape_map.end())
			{
				ret += iter->second;
				continue;
			}

			if (code < 32 || (escape_unicode && code > 127))
			{
				char buf[7];
				sprintf(buf, "\\u%04X", code);
				ret += buf;				
				continue;
			}			
			ret += _utf8_from_unicode(code);
		}
		return ret;
	}	

	inline std::string unescape(const std::string& s)
	{
		static std::unordered_map<char, char> unescape_map = {
			{ '\"', '\"'},
			{ '\\', '\\'},
			{ '/', '/'},
			{ 'b', '\b'},
			{ 'f', '\f'},
			{ 'n', '\n'},
			{ 'r', '\r'},
			{ 't', '\t'},
		};

		std::string ret;
		for (size_t i = 0; i < s.size(); i++)
		{
			char c = s[i];
			if (c == '\\')
			{
				i++;
				if (i >= s.size()) break;
				c = s[i];
				if (c == 'u')
				{
					if (i + 4 > s.size()) break;
					std::string uc4 = s.substr(i + 1, 4);					
					int code = std::stoi(uc4, nullptr, 16);
					ret += _utf8_from_unicode(code);
					i += 4;
					continue;
				}
				auto iter = unescape_map.find(c);
				if (iter != unescape_map.end())
				{
					ret += iter->second;
					continue;
				}
			}
			ret += c;
		}

		return ret;
	}

	inline std::string Strigify(SPToken token, bool escape_unicode = false, int indent = 0)
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
				return std::string("\"") + escape(t->value(), escape_unicode) + "\"";
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
					str += Strigify(arr[i], escape_unicode, indent);
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
					
					str += std::string("\"") + escape(iter->first, escape_unicode) + "\": ";
					str += Strigify(iter->second, escape_unicode, indent);
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
			return String::New(unescape(sub));
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

				std::string str = sub.substr(key_start, key_len);
				std::string key = unescape(str);
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

		return Number::New(std::stof(s));
	}
}

