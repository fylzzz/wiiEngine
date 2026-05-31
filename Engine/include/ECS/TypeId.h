#pragma once

#include <cstdint>


using ComponentType = uint8_t;

class TypeIdGenerator {
	public:
		template<typename T>
		static ComponentType get() {
			static ComponentType id = mNextId++;
			return id;
		}
	private:
		static inline ComponentType mNextId = 0;
};