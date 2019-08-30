#if !defined(ASTEROIDS_INTRINSICS_H)

inline u32 AtomicCompareExchangeUInt32(u32 volatile *Value, u32 Expected, u32 New)
{
    u32 Result = _InterlockedCompareExchange((long *)Value, Expected, New);
    
    return Result;
}

#define ASTEROIDS_INTRINSICS_H
#endif