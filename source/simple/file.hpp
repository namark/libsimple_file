#ifndef _SIMPLE_FILE_HPP
#define _SIMPLE_FILE_HPP

#include <iosfwd>
#include <type_traits>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>

namespace simple
{ namespace file
{
	//TODO: wow, turns out there are actual file descriptors in POSIX, so need those here too, as an option

	// TODO:
	// class c_stream_deleter
	// {
	// 	public:
	// 	void operator ()(FILE* file) const noexcept;
	// };
	// using c_stream = std::unique_ptr<FILE, c_stream_deleter>;

	using namespace std::literals;
	using std::string;
	using stream = std::fstream;
	using buffer_type = std::vector<char>;
	using size_type = std::streamoff;
	template<typename Alloc = std::allocator<char>>
	using buffer [[deprecated]] = std::vector<char, Alloc>;

	static const string path_delimeters = R"(/\)"s;

	inline size_type size(stream& file);
	inline size_type size(stream&& file);

	inline void dump(stream& from, char* to, size_type this_much);
	template<typename Buffer = buffer_type>
	inline void dump(stream& from, Buffer& to);
	template<typename Buffer = buffer_type>
	inline auto dump(stream& from) -> Buffer;

	inline void dump(const char* from, size_t this_much, stream& to);
	template<int N>
	inline void dump(const char from[N], stream& to);
	template<typename Buffer = buffer_type>
	inline void dump(const Buffer& from, stream& to);

namespace operators
{

	template<typename Buffer = buffer_type>
	inline void operator <<= (Buffer& to, stream& from);
	template<typename Buffer = buffer_type>
	inline stream& operator <<= (stream& to, const Buffer& from);
	inline stream& operator <<= (stream& to, stream& from);

} // namespace operators

	struct flags
	{
		stream::openmode io;
		stream::iostate exceptions = stream::goodbit;
		bool buffering = true;

		flags& throws(stream::iostate these = stream::badbit | stream::failbit);
		flags& reads();
		flags& writes();
		flags& binary();
		flags& no_buffer();
	};

	inline stream open(const string& name, flags options = {stream::in | stream::out});
	inline stream ropen(const string& name);
	inline stream wopen(const string& name);

	inline stream bopen(const string& name, flags options = {stream::in | stream::out});
	inline stream bropen(const string& name);
	inline stream bwopen(const string& name);

	inline stream opex(const string& name, flags options = {stream::in | stream::out});
	inline stream ropex(const string& name);
	inline stream wopex(const string& name);

	inline stream bopex(const string& name, flags options = {stream::in | stream::out});
	inline stream bropex(const string& name);
	inline stream bwopex(const string& name);

namespace string_stack
{

	template<typename String = string>
	inline auto top(const String& stack, const String& delimeters = path_delimeters)
	-> typename String::size_type;

	template<typename String = string>
	inline String pop(String& stack, const String& delimeters = path_delimeters);

	template<typename String = string>
	inline void drop(String& stack, const String& delimeters = path_delimeters);

	template<typename String = string>
	inline void push(String& stack, const String& content, const String& delimeters = path_delimeters);

	template <typename String = string>
	class manipulator
	{
		public:

		manipulator(String& on, String delimeters = path_delimeters);

		manipulator(manipulator&& other);
		manipulator&& operator=(manipulator&& other);

		~manipulator();

		void release();

		String pop();

		void drop();

		manipulator& push(const String& value) &;

		template <typename ...Args>
		manipulator& push(const String& value, Args... args) &;

		manipulator&& push(const String& value) &&;

		template <typename ...Args>
		manipulator&& push(const String& value, Args... args) &&;

		private:

		String* stack;
		unsigned size = 0;
		const String delimeters;

		unsigned dec(unsigned amount = 1);

		manipulator(const manipulator& other) = default;
		manipulator& operator=(const manipulator& other) = default;

	};

} // namespace string_stack


/************************ implementation ************************/

	inline size_type size(stream& file)
	{
		auto current = file.tellg();
		auto ret = size(std::move(file));
		file.seekg(current);
		return ret;
	}

	inline size_type size(stream&& file)
	{
		file.seekg(0, file.end);
		auto ret = file.tellg();
		return ret;
	}

	inline void dump(stream& from, char* to, size_type this_much)
	{
		auto _size = size(from);
		if(-1 != _size)
			from.read(to, std::min(_size, this_much));
	}

	template<typename Buffer>
	inline void dump(stream& from, Buffer& to)
	{
		auto _size = size(from);
		if(-1 == _size)
			return;
		to.resize(_size);
		from.read(reinterpret_cast<char*>(to.data()), sizeof(*to.data())*to.size());
	}

	template<typename Buffer>
	inline auto dump(stream& from) -> Buffer
	{
		Buffer to;
		dump(from, to);
		return to;
	}

	inline void dump(const char* from, size_t this_much, stream& to)
	{
		to.write(from, this_much);
	}

	template<int N>
	inline void dump(const char from[N], stream& to)
	{
		to.write(from, N);
	}

	template<typename Buffer>
	inline void dump(const Buffer& from, stream& to)
	{
		dump(reinterpret_cast<const char*>(from.data()), sizeof(*from.data())*from.size(), to);
	}

	template<typename Buffer>
	inline void operators::operator <<= (Buffer& to, stream& from)
	{
		dump(from, to);
	}

	template<typename Buffer>
	inline stream& operators::operator <<= (stream& to, const Buffer& from)
	{
		dump(from, to);
		return to;
	}

	inline stream& operators::operator <<= (stream& to, stream& from)
	{
		return to <<= dump(from);
	}

	inline flags& flags::throws(stream::iostate these)
	{
		exceptions |= these;
		return *this;
	}

	inline flags& flags::reads()
	{
		io |= stream::in;
		return *this;
	}

	inline flags& flags::writes()
	{
		io |= stream::out;
		return *this;
	}

	inline flags& flags::binary()
	{
		io |= stream::binary;
		return *this;
	}

	inline flags& flags::no_buffer()
	{
		buffering = false;
		return *this;
	}

	inline stream open (const string& name, flags options)
	{
		stream file;
		file.exceptions(options.exceptions);
		if(!options.buffering)
			file.rdbuf()->pubsetbuf(0, 0);
		file.open(name, options.io);
		return file;
	}

	inline stream ropen(const string& name)
	{
		return open(name, flags{}.reads());
	}

	inline stream wopen(const string& name)
	{
		return open(name, flags{}.writes());
	}

	inline stream bopen(const string& name, flags options)
	{
		return open(name, options.binary());
	}

	inline stream bropen(const string& name)
	{
		return open(name, flags{}.reads().binary());
	}

	inline stream bwopen(const string& name)
	{
		return open(name, flags{}.writes().binary());
	}

	inline stream opex(const string& name, flags options)
	{
		return open(name, options.throws());
	}

	inline stream ropex(const string& name)
	{
		return open(name, flags{}.reads().throws());
	}

	inline stream wopex(const string& name)
	{
		return open(name, flags{}.writes().throws());
	}

	inline stream bopex(const string& name, flags options)
	{
		return open(name, options.binary().throws());
	}

	inline stream bropex(const string& name)
	{
		return open(name, flags{}.reads().binary().throws());
	}

	inline stream bwopex(const string& name)
	{
		return open(name, flags{}.writes().binary().throws());
	}

namespace string_stack
{

	template<typename String>
	inline auto top(const String& stack, const String& delimeters)
	-> typename String::size_type
	{
		auto end = stack.find_last_not_of(delimeters);
		auto top = stack.find_last_of(delimeters, end);

		// this edge case turns out to be handled rather nicely by the usual case, but leaving the code commented here to indicate the intent
		// if(string::npos == top)
		// 	top = 0;
		// else
		++top;

		return top;
	}

	template<typename String>
	inline String pop(String& stack, const String& delimeters)
	{
		auto _top = top(stack, delimeters);
		auto name = stack.substr(_top);
		stack.resize(_top);
		return name;
	}

	template<typename String>
	inline void drop(String& stack, const String& delimeters)
	{
		stack.resize(top(stack, delimeters));
	}

	template<typename String>
	inline void push(String& stack, const String& content, const String& delimeters)
	{
		if(!stack.empty() && delimeters.find(stack.back()) == string::npos)
			stack += delimeters.front();
		stack += content;
	}

	template <typename String>
	inline unsigned manipulator<String>::dec(unsigned amount)
	{
		return size -= std::min(size, amount);
	}

	template <typename String>
	inline manipulator<String>::manipulator(String& on, String delimeters)
		: stack(&on), delimeters(delimeters)
	{}

	template <typename String>
	inline manipulator<String>::manipulator(manipulator&& other) : manipulator(other)
	{
		other.release();
	}

	template <typename String>
	inline manipulator<String>&& manipulator<String>::operator=(manipulator&& other)
	{
		*this = other;
		other.release();
		return *this;
	}

	template <typename String>
	inline manipulator<String>::~manipulator()
	{
		while(size)
			drop();
	}

	template <typename String>
	inline void manipulator<String>::release()
	{
		size = 0;
	}

	template <typename String>
	inline String manipulator<String>::pop()
	{
		dec();
		return string_stack::pop(*stack, delimeters);
	};

	template <typename String>
	inline void manipulator<String>::drop()
	{
		string_stack::drop(*stack, delimeters);
		dec();
	};

	template <typename String>
	inline auto manipulator<String>::push(const String& value) &
	-> manipulator<String>&
	{
		if(!value.empty())
		{
			string_stack::push(*stack, value, delimeters);
			++size;
		}
		return *this;
	}

	template <typename String>
	template <typename ...Args>
	inline auto manipulator<String>::push(const String& value, Args... args) &
	-> manipulator<String>&
	{
		push(value);
		return push(args...);
	}

	template <typename String>
	inline auto manipulator<String>::push(const String& value) &&
	-> manipulator<String>&&
	{
		push(value);
		return std::move(*this);
	}

	template <typename String>
	template <typename ...Args>
	inline auto manipulator<String>::push(const String& value, Args... args) &&
	-> manipulator<String>&&
	{
		std::move(*this).push(value);
		return std::move(*this).push(args...);
	}

} // namespace string_stack


	// rvalue variants

	inline void dump(stream&& from, char* to, size_type this_much)
	{ dump(from, to, this_much); }

	template<typename Buffer = buffer_type>
	inline void dump(stream&& from, Buffer& to)
	{ dump(from, to); }

	template<typename Buffer = buffer_type>
	inline auto dump(stream&& from) -> Buffer
	{ return dump<Buffer>(from); }

	inline void dump(const char* from, size_t this_much, stream&& to)
	{ dump(from, this_much, to); }

	template<int N>
	inline void dump(const char from[N], stream&& to)
	{ dump<N>(from, to); }

	template<typename Buffer = buffer_type>
	inline void dump(const Buffer& from, stream&& to)
	{ dump(from, to); }

namespace operators
{
	inline stream& operator <<= (stream& to, stream&& from)
	{ return to <<= from; }
	template<typename Buffer = buffer_type>
	inline void operator <<= (Buffer& to, stream&& from)
	{ to <<= from; }
	template<typename Buffer = buffer_type>
	inline stream&& operator <<= (stream&& to, const Buffer& from)
	{ return std::move(to <<= from); }
	inline stream&& operator <<= (stream&& to, stream&& from)
	{ return std::move(to <<= from); }
}


}} // namespace simple::file

#endif /* end of include guard */

