#pragma once

#define	LOG_DEBUG	__noop
#define	LOG_INFO	__noop
#define	LOG_ERROR	__noop

#define	OG_ASSERTR		__noop
#define	OG_ASSERTCRASH	__noop
#define	OG_ASSERTCRASHR	__noop


class Log
{
public:
	Log();
	~Log();
};


class LogSystem
{
public:
	static	void	Flush() {}
};