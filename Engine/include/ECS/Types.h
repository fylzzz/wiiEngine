#pragma once

#include <bitset>
#include <cstdint>

using Entity = uint32_t;

constexpr Entity MAX_ENTITIES = 5000;

constexpr Entity INVALID_ENTITY = std::numeric_limits<Entity>::max();

constexpr uint8_t MAX_COMPONENTS = 32;

using Signature = std::bitset<MAX_COMPONENTS>;