#pragma once

// create constexpr flags for whether the backend should obey alignment hints
#ifdef EOS_VM_ALIGN_MEMORY_OPS
inline constexpr bool should_align_memory_ops = true;
#else
inline constexpr bool should_align_memory_ops = false;
#endif

#ifdef EOS_VM_SOFTFLOAT
inline constexpr bool use_softfloat = true;
#else
inline constexpr bool use_softfloat = false;
#endif
