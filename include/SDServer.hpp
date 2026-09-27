#pragma once

#include "SC_PlugIn.h"
#include "SDCommon.hpp"
#include "SDRandom.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

extern InterfaceTable* ft;

namespace sd {

inline float inputAt(Unit* unit, int inputIndex, int sampleIndex)
{
    return INRATE(inputIndex) == calc_FullRate ? IN(inputIndex)[sampleIndex] : IN0(inputIndex);
}

inline uint32_t seedForUnit(Unit* unit, int inputIndex)
{
    uint32_t seed = explicitSeedFromFloat(IN0(inputIndex));
    if (seed == 0U && unit->mParent && unit->mParent->mRGen)
        seed = unit->mParent->mRGen->trand();
    return seed == 0U ? 0x853c49e6U : seed;
}

template <typename T>
inline T* allocateRT(Unit* unit, std::size_t count)
{
    if (count == 0)
        return nullptr;
    T* result = static_cast<T*>(RTAlloc(unit->mWorld, count * sizeof(T)));
    if (result)
        std::fill(result, result + count, T {});
    return result;
}

template <typename T>
inline void freeRT(Unit* unit, T*& pointer)
{
    if (pointer) {
        RTFree(unit->mWorld, pointer);
        pointer = nullptr;
    }
}

inline SndBuf* resolveBuffer(Unit* unit, float number)
{
    if (!std::isfinite(number) || number < 0.0f)
        return nullptr;
    const uint32_t index = static_cast<uint32_t>(number);
    if (index < unit->mWorld->mNumSndBufs)
        return unit->mWorld->mSndBufs + index;
    if (!unit->mParent)
        return nullptr;
    const uint32_t localIndex = index - unit->mWorld->mNumSndBufs;
    if (localIndex > static_cast<uint32_t>(unit->mParent->localBufNum))
        return nullptr;
    return unit->mParent->mLocalSndBufs + localIndex;
}

inline void warnOnce(Unit* unit, bool& warned, const char* message)
{
    if (!warned && unit->mWorld->mVerbosity > -1) {
        Print("%s\n", message);
        warned = true;
    }
}

} // namespace sd
