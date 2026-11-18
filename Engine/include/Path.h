#pragma once

#include <string>
#include <cstring>


inline std::string ResolvePath(const char* p) {
#ifdef GEKKO
	return p;
#else
	if (std::strncmp(p, "sd:/", 4) == 0) return std::string("assets/") + (p + 4);
	return p;
#endif
}