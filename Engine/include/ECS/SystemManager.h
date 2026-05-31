#pragma once

#include "TypeId.h"
#include "Types.h"
#include "System.h"

#include <array>
#include <cassert>
#include <memory>
#include <set>


constexpr uint8_t MAX_SYSTEMS = 32;

class SystemIdGenerator {
	public:
		template<typename T>
		static uint8_t get() {
			static uint8_t id = mNextId++;
			return id;
		}

	private:
		static inline uint8_t mNextId = 0;
};

class SystemManager {
	public:
		template<typename T>
		std::shared_ptr<T> registerSystem() {
			uint8_t id = SystemIdGenerator::get<T>();
			assert(!mSystems[id] && "System registered twice");
			auto system = std::make_shared<T>();
			mSystems[id] = system;
			return system;
		}

		template<typename T>
		void setSignature(Signature sig) {
			uint8_t id = SystemIdGenerator::get<T>();
			assert(mSystems[id] && "System used before registration");
			mSignatures[id] = sig;
		}

		void entityDestroyed(Entity e);
		void entitySignatureChanged(Entity e, Signature sig);

	private:
		std::array<std::shared_ptr<System>, MAX_SYSTEMS> mSystems{};
		std::array<Signature, MAX_SYSTEMS> mSignatures{};
};