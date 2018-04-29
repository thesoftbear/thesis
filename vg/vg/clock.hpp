#pragma once

#include "glad\glad.h"

namespace thesis
{
	class clock
	{
		public:

			clock();
			~clock();
			void start_timer();
			void stop_timer();
			float get_time();

		private:

			GLuint timer_queries[2];
	};
}