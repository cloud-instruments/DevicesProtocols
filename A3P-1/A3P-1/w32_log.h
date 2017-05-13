#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <time.h>

// TODO: test filtering

// use this to log "function:line"
#define W32_LOG_FINFO __FUNCTION__ << ":" << __LINE__ << " "

// Example use without macros
//std::ofstream log_file("log.txt", std::ios::app);
//w32_log cwlog(log_file, w32_log::trace);
//cwlog << "warning!" << w32_log::warning << std::endl;

// example of global use
// std::ofstream *wlog_file = new std::ofstream("log.txt", std::ios::app);
// w32_log *pwlog = new w32_log(*wlog_file, w32_log::trace);
// *pwlog << W32_LOG_FINFO << "starting:" << argv[0] << w32_log::trace << std::endl;

class w32_log : public std::ostringstream
{
public:

	// log levels 
	typedef enum
	{
		none, 
		debug,
		trace,
		warning,
		error
	} w32_log_level;

private:

	w32_log_level m_log_level;
	w32_log_level m_current_level; // only until the next flush/endl
	
	char level_char(w32_log_level l) {
		switch (l){
		case error:return 'E';
		case warning:return 'W';
		case trace:return 'T';
		case debug:return 'D';
		default:return 'U';
		}
	}

	std::ostream & out;

public:

	// If level is high enough writes buffer to the log file.
	// Clears the buffer. resets setfill to ' '
	void flush(){
		if ((m_current_level >= m_log_level)&&(!str().empty())){

			// prefixes lines with a time stamp 
			out << time(NULL) << " " << level_char(m_current_level) << " " << str();
			out.flush();
		}

		str("");
		m_current_level = debug;
	}


	// inherits EVERYTHING that's  not explicitely overloaded in this class
	template <typename T> inline w32_log & operator<<(const T & t){
		(*(std::ostringstream*)this) << t;
		return *this;
	}

	inline w32_log & operator<<(const w32_log_level & level) { m_current_level = level; return *this; }

	// sets the acceptable message level until next flush/endl
	inline void set_level(w32_log_level level) { m_current_level = level; }
	inline w32_log_level get_level() const { return m_current_level; }
	
	// set/get filter level
	inline void change_log_level(w32_log_level level) { m_log_level = level; }
	inline w32_log_level get_log_level() const { return m_log_level; }

	// level for single message
	w32_log(std::ostream & log_stream, w32_log_level level = debug):
		m_log_level(level),
		m_current_level(debug),
		out(log_stream)
	{}

	virtual ~w32_log(){flush();}

	// stubs for manipulators
	typedef w32_log & (*w32_log_manip)(w32_log &);
	w32_log & operator<<(w32_log_manip manip) { return manip(*this); }
};


////////////////////////////////////////
//
// manipulators
//

// setlevel is reset after each 
// endl, therefore it applies to
// the current log line only.
struct __w32_log_level { w32_log::w32_log_level m_level; };
inline __w32_log_level setlevel(w32_log::w32_log_level _level)
{
	__w32_log_level level = { _level };
	return level;
}

inline w32_log & operator<<(w32_log & out, const __w32_log_level & level)
{
	out.set_level(level.m_level);
	return out;
}


// Overloads endl for w32_logs
namespace std {
	inline w32_log & endl(w32_log & out) {
		out.put('\n'); out.flush(); return out;
	}
}

