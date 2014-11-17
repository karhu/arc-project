#include "log.hpp"

#include <iostream>

namespace arc { namespace log {

	Logger* global_instance = nullptr;

	static const uint32 BUFFER_SIZE = 1024;
	static ARC_THREAD_LOCAL char t_buffer[BUFFER_SIZE];

	Slice<char> DefaultLogger::buffer()
	{
		return Slice<char>(t_buffer, BUFFER_SIZE);
	}

	char priority_tag(int priority)
	{
		switch (priority)
		{
		case PRIORITY_VERBOSE: return 'V';
		case PRIORITY_DEBUG: return 'D';
		case PRIORITY_INFO: return 'I';
		case PRIORITY_WARNING: return 'W';
		case PRIORITY_ERROR: return 'E';
		case PRIORITY_CRITICAL: return 'C';
		case PRIORITY_NONE: return 'N';
		default: return '?';
		}
	}

	void DefaultLogger::send(Message m)
	{
		std::cout << "[" << priority_tag(m.priority) << "]";
		if (m.tag) std::cout << "[" << m.tag << "]";
		std::cout << "[" << m.file << ":" << m.line << "]";
		std::cout << " " << m.text << "\n";
	}

	DefaultLogger::~DefaultLogger()
	{
		if (global_instance == this)
		{
			global_instance = nullptr;
		}
	}

	void set_logger(Logger& logger)
	{
		global_instance = &logger;
	}


}} // namespace arc::log