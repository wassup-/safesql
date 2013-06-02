#ifndef ASSERT_HPP_
#define ASSERT_HPP_

#include "config.hpp"

#include <iostream>

#undef assert

#if defined(FP_DEBUG)

#define FP_ASSERT(x, msg)	do { if(!(x)) { ::fp::assert((#x), (msg), (__FILE__), (__LINE__), (__PRETTY_FUNCTION__)); } } while(0);

#else

#define FP_ASSERT(x, msg)

#endif

namespace fp {
	enum class assert_method_e { halt_, continue_ };

	assert_method_e assert_method = assert_method_e::halt_;

	void assert(char const * cond, char const * msg, char const * file, int line, char const * fn) {
		std::cerr << file << ":" << line << " in " << fn << " :\n\tAssert " << cond << " failed: " << msg << std::endl;
		if(assert_method_e::halt_ == assert_method) {
			exit(0);
		}
	}
}

#endif