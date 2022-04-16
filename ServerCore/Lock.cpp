#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"

//#include "Windows.h"

void Lock::WriteLock()
{
    const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
    if (LThreadId == lockThreadId) {
        _writeCount++;
        return;
    }

    const int64 beginTick = ::GetTickCount64();
    const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
    while (true) {
        for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount) {
            uint32 expected = EMPTY_FLAG;
            if (_lockFlag.compare_exchange_strong(OUT expected, desired)) {
                _writeCount++;
                return;
            }
        }

        if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
            CRASH("LOCK_TIMEOUT");

        std::this_thread::yield();
    }
}

void Lock::WriteUnlock()
{ 
    // DEBUG
    if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
        CRASH("INVALID_UNLOCK_ORDER");

    const int32 lockCount = --_writeCount;
    if (lockCount == 0)
        _lockFlag.store(EMPTY_FLAG);
        //_lockFlag.store(_lockFlag.load() & READ_COUNT_MASK);
}

void Lock::ReadLock()
{
    const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
    if (LThreadId == lockThreadId) {
        _lockFlag.fetch_add(1);
        return;
    }

    const int64 beginTick = ::GetTickCount64();
    while (true) {
        for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount) {
            uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
            if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
                return;
        }

        if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
            CRASH("LOCK_TIMEOUT");

        std::this_thread::yield();
    }
}

void Lock::ReadUnlock()
{
    if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
        CRASH("MULTIPLE_UNLOCK");
}
