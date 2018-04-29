#include "clock.hpp"

namespace thesis
{
	clock::clock()
	{
		glGenQueries(2, timer_queries);
	}

	clock::~clock()
	{
		glDeleteQueries(2, timer_queries);
	}

	void clock::start_timer()
	{
		glQueryCounter(timer_queries[0], GL_TIMESTAMP);
	}

	void clock::stop_timer()
	{
		glQueryCounter(timer_queries[1], GL_TIMESTAMP);
	}

	float clock::get_time()
	{
		int timer_available = 0;
		while (!timer_available) glGetQueryObjectiv(timer_queries[1], GL_QUERY_RESULT_AVAILABLE, &timer_available);

		GLuint64 start_time, stop_time;
		glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &start_time);
		glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &stop_time);

		return (stop_time - start_time) / 1000000.0;
	}
}