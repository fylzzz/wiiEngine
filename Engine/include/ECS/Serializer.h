#pragma once

#include <bit>
#include <cstdint>
#include <cstdio>
#include <string>
#include <type_traits>
#include "raylib.h"


namespace ser {
	template<typename T>
	constexpr T swapToDisk(T v) {
		static_assert(std::is_unsigned_v<T>, "swap unsigned ints only");
		if constexpr (std::endian::native == std::endian::big || sizeof(T) == 1) return v;
		else if constexpr (sizeof(T) == 2) return static_cast<T>(__builtin_bswap16(v));
		else if constexpr (sizeof(T) == 4) return static_cast<T>(__builtin_bswap32(v));
		else return static_cast<T>(__builtin_bswap64(v));
	}

	constexpr uint32_t FourCC(char a, char b, char c, char d) {
		return (uint32_t(uint8_t(a)) << 24) | (uint32_t(uint8_t(b)) << 16) | (uint32_t(uint8_t(c)) << 8) | uint32_t(uint8_t(d));
	}
}

constexpr uint32_t SCENE_MAGIC = ser::FourCC('E', 'G', '7', 'S');
constexpr uint16_t SCENE_VER = 1;

class Writer {
	public:
		explicit Writer(const char* path) : mFile(std::fopen(path, "wb")) {}
		~Writer() { if (mFile) std::fclose(mFile); }
		Writer(const Writer&) = delete;
		Writer& operator=(const Writer&) = delete;

		bool ok() const { return mFile && !mError; };

		void u8(uint8_t v) { raw(&v, 1); }
		void u16(uint16_t v) { v = ser::swapToDisk(v); raw(&v, 2); }
		void u32(uint32_t v) { v = ser::swapToDisk(v); raw(&v, 4); }
		void i32(int32_t v) { u32(std::bit_cast<uint32_t>(v)); }
		void f32(float v) { u32(std::bit_cast<uint32_t>(v)); }
		void b(bool v) { u8(v ? 1 : 0); }

		void str(const std::string& s) { u32(uint32_t(s.size())); raw(s.data(), s.size()); }

		void vec2(Vector2 v) { f32(v.x); f32(v.y); }
		void vec3(Vector3 v) { f32(v.x); f32(v.y); f32(v.z); }
		void color(Color c) { u8(c.r); u8(c.g); u8(c.b); u8(c.a); }

	private:
		void raw(const void* p, size_t n) {
			if (!ok() || n == 0) return;
			if (std::fwrite(p, 1, n, mFile) != n) mError = true;
		}

		FILE* mFile = nullptr;
		bool mError = false;
};

class Reader {
public:
	explicit Reader(const char* path) : mFile(std::fopen(path, "rb")) {}
	~Reader() { if (mFile) std::fclose(mFile); }
	Reader(const Reader&) = delete;
	Reader& operator=(const Reader&) = delete;

	bool ok() const { return mFile && !mError; }

	uint8_t  u8() { uint8_t v = 0;  raw(&v, 1); return v; }
	uint16_t u16() { uint16_t v = 0; raw(&v, 2); return ser::swapToDisk(v); }
	uint32_t u32() { uint32_t v = 0; raw(&v, 4); return ser::swapToDisk(v); }
	int32_t  i32() { return std::bit_cast<int32_t>(u32()); }
	float    f32() { return std::bit_cast<float>(u32()); }
	bool     b() { return u8() != 0; }

	std::string str(uint32_t maxLen = 4096)
	{
		uint32_t n = u32();
		if (n > maxLen) { mError = true; return {}; }   // corrupt length guard
		std::string s(n, '\0');
		raw(s.data(), n);
		return s;
	}

	Vector2 vec2() { Vector2 v; v.x = f32(); v.y = f32(); return v; }
	Vector3 vec3() { Vector3 v; v.x = f32(); v.y = f32(); v.z = f32(); return v; }
	Color   color() { Color c; c.r = u8(); c.g = u8(); c.b = u8(); c.a = u8(); return c; }

private:
	void raw(void* p, size_t n)
	{
		if (!ok() || n == 0) return;
		if (std::fread(p, 1, n, mFile) != n) mError = true;
	}

	FILE* mFile = nullptr;
	bool mError = false;
};