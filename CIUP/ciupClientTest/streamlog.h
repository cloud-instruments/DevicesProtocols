// (C)2017 Matteo Lucarelli All rights reserved
// streamlog implementation (use stream-like sintax to write to file)

#ifndef STREAMLOG_H_INCLUDED
#define STREAMLOG_H_INCLUDED

#include <fstream>
#include <sstream>
#include <ctime>

// use this to log "function:line"
#define STREAMLOG_FINFO __FUNCTION__ << ":" << __LINE__ << " "

// Example use without macros:
//   std::ofstream log_file("log.txt", std::ios::app);
//   streamlog cwlog(log_file, streamlog::trace);
//   cwlog << "warning!" << streamlog::warning << std::endl;
//
// Example of global use:
//   std::ofstream *wlog_file = new std::ofstream("log.txt", std::ios::app);
//   streamlog *pwlog = new streamlog(*wlog_file, streamlog::trace);
//   *pwlog << STREAMLOG_FINFO << "starting:" << argv[0] << streamlog::trace << std::endl;

class streamlog : public std::ostringstream
{
public:

	// log levels 
	typedef enum
	{
		debug,
		trace,
		warning,
		error,
		none
	} level;

public:

	// CONSTRUCTOR
	// log_stream: std::ofstream of output file
	// lfilter: log level filter (print only messages with level >= filter)
	// ldefault: default level tu use if not specified
	streamlog(std::ostream &log_stream, level lfilter = debug, level ldefault = debug):
		m_filter(lfilter),
		m_current_level(ldefault),
		m_default_level(ldefault),
		out(log_stream)
	{}

	// DESTRUCTOR
	virtual ~streamlog(){
		flush();
	}

	// set level for single message (until flush/endl)
	// Ex:
	//   slog << streamlog::debug << "debug message" << std::endl;
	//   slog << "error message" << streamlog::error << std::endl;
	//   slog << streamlog::warning;
	inline streamlog & operator<<(const level & level) { m_current_level = level; return *this; }

	// set/get filter and default levels
	inline void set_filter(level level) { m_filter = level; }
	inline level get_filter() const { return m_filter; }
	inline void set_default(level level) { m_default_level = level; }
	inline level get_default() const { return m_default_level; }
	
	// stubs for manipulators
	typedef streamlog & (*streamlog_manip)(streamlog &);
	streamlog & operator<<(streamlog_manip manip) { return manip(*this); }
	
	// If level is high enough writes buffer to the log file.
	void flush(){
	
		// if there's a message and level>=filter print message
		if ( ( !str().empty()) && (m_current_level >= m_filter) ){
			
			//out << time(NULL) << " " << level_char(m_current_level) << " " << str();

			// UTC formatted time
			struct tm utctime;
			time_t rawtime;
			time(&rawtime);
			gmtime_s(&utctime,&rawtime);
			char ftime[64];
			strftime(ftime, 64, "%F %T ", &utctime);

			// prefixes lines with a time stamp and level char
			out << ftime << level_char(m_current_level) << " " << str();
			out.flush();
		}
		
		// clean message and reset default level
		str("");
		m_current_level = m_default_level;
	}

	// inherits EVERYTHING that's  not explicitely overloaded in this class
	template <typename T> inline streamlog & operator<<(const T & t){
		(*(std::ostringstream*)this) << t;
		return *this;
	}
	
	// return a char representing level
	char level_char(level l) {
		switch (l){
			case error:return 'E';
			case warning:return 'W';
			case trace:return 'T';
			case debug:return 'D';
			default:return 'U';
		}
	}
	
private:

	level m_filter;
	level m_current_level;
	level m_default_level;
	std::ostream & out;
};

// manipulators ////////////////////////////////////////////////////////////////

// level is reset after each endl therefore it applies to the current log line only.
struct __level { streamlog::level m_level; };
inline __level setlevel(streamlog::level _level){
	__level level = { _level };
	return level;
}

// operator << to set line level
inline streamlog & operator<<(streamlog & out, const __level & level){
	out.set_filter(level.m_level);
	return out;
}

// overloads endl for flush
namespace std {
	inline streamlog & endl(streamlog & out) {
		out.put('\n'); out.flush(); return out;
	}
}

#endif // STREAMLOG_H_INCLUDED

