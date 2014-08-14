#pragma once

#include <thread>

#include "buffer_writer.hpp"
#include "arc/collections/Slice.hpp"

namespace arc { namespace log {

	// Logging priorities
	const int PRIORITY_VERBOSE = 0;
	const int PRIORITY_DEBUG = 1;
	const int PRIORITY_INFO = 2;
	const int PRIORITY_WARNING = 3;
	const int PRIORITY_ERROR = 4;
	const int PRIORITY_CRITICAL = 5;
	const int PRIORITY_NONE = 6;

	struct Message
	{
		uint64 thread_id;
		const char* tag;
		const char* text;
		int priority;
	};

	class Logger
	{
	public:
		virtual ~Logger() {}
		virtual Slice<char> buffer() = 0;
		virtual void send(Message m) = 0;
	};

	class DefaultLogger : public Logger
	{
	public:
		~DefaultLogger();
		Slice<char> buffer() override;
		void send(Message m);
	};

	// The global Logger instance
	extern Logger* global_instance;

	void set_logger(Logger& logger);

	// helper functions

	inline void _unroll_write(arc::buffer_writer& b)  {}

	template<typename T, typename ...Args> inline
	void _unroll_write(arc::buffer_writer& b, T&& v, Args&& ...args)
	{
		if (b.write(v))
		{
			arc::log::_unroll_write(b, std::forward<Args>(args)...);
		}
	}

	template<typename TAG, typename ...Args> inline
	void _message(int priority, Args&& ...args)
	{
		if (global_instance == nullptr) return;

		// TODO is there a better way than this?
		uint64 thread_id = std::hash<std::thread::id>()(std::this_thread::get_id());

		// get buffer
		Slice<char> buffer = global_instance->buffer();

		buffer_writer writer(buffer.ptr(), (uint32)buffer.size());
		_unroll_write(writer, std::forward<Args>(args)...);

		Message message;
		message.thread_id = thread_id;
		message.tag = TAG::name();
		message.text = writer.str();
		message.priority = priority;

		// format the message

		// send the message off
		global_instance->send(message);
	}

	// API ///////////////////////////////////////////////////////////

	#define LOG_VERBOSE(cat,...) \
		{ if (arc::log::PRIORITY_VERBOSE >= cat::priority() ) log::_message<cat>(arc::log::PRIORITY_VERBOSE,__VA_ARGS__); }
	
	#define LOG_DEBUG(cat,...) \
		{ if (arc::log::PRIORITY_DEBUG >= cat::priority() ) log::_message<cat>(arc::log::PRIORITY_DEBUG,__VA_ARGS__); }

	#define LOG_INFO(cat,...) \
		{ if (arc::log::PRIORITY_INFO >= cat::priority() ) log::_message<cat>(arc::log::PRIORITY_INFO,__VA_ARGS__); }

	#define LOG_WARNING(cat,...) \
		{ if (arc::log::PRIORITY_WARNING >= cat::priority() ) log::_message<cat>(arc::log::PRIORITY_WARNING,__VA_ARGS__); }

	#define LOG_ERROR(cat,...) \
		{ if (arc::log::PRIORITY_ERROR >= cat::priority() ) log::_message<cat>(arc::log::PRIORITY_ERROR,__VA_ARGS__); }

	#define LOG_CRITICAL(cat,...) \
		{ if (arc::log::PRIORITY_CRITICAL >= cat::priority() ) log::_message<cat>(arc::log::PRIORITY_CRITICAL,__VA_ARGS__); }

}}