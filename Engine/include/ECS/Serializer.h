#pragma once

#include <cstdio>
#include <cstring>


class Writer {
	public:
		FILE* file = nullptr;
		template<typename T>
		void write(const T& val) {
			fwrite(&val, sizeof(T), 1, file);
		}
};

class Reader {
	public:
		FILE* file = nullptr;
		template<typename T>
		void read(T& val) {
			fread(&val, sizeof(T), 1, file);
		}
};