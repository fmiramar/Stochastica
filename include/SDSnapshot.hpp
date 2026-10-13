#pragma once
#include "SDServer.hpp"

namespace sd {
// Guard the existing resolver's float-to-uint32 conversion and local-buffer
// indexing. Never inspect mutable SndBuf fields until its reader lock is held.
inline bool snapshotMono(Unit* unit, float number, double* destination, int count,
                         bool requireFinite = false)
{
    if (!std::isfinite(number) || number < 0.0f || double(number) >= 4294967296.0)
        return false;
    const auto index = static_cast<uint32_t>(number);
    if (index >= unit->mWorld->mNumSndBufs) {
        if (!unit->mParent || unit->mParent->localBufNum < 0
            || index - unit->mWorld->mNumSndBufs > static_cast<uint32_t>(unit->mParent->localBufNum))
            return false;
    }
    SndBuf* buffer = resolveBuffer(unit, number);
    if (!buffer) return false;
    LOCK_SNDBUF_SHARED(buffer);
    if (!buffer->data || buffer->channels != 1 || buffer->samples < count)
        return false;
    for (int i = 0; i < count; ++i) {
        if (requireFinite && !finite(buffer->data[i])) return false;
        destination[i] = finiteOr(buffer->data[i], 0.0);
    }
    return true;
}
} // namespace sd
