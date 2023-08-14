#ifndef PDIAGNOSTIC_POP_H
#define PDIAGNOSTIC_POP_H

#if defined(__CC_ARM) && defined(PDIAGNOSTIC_DEFINED_NDEBUG) && defined(NDEBUG)
#if PDIAGNOSTIC_DEFINED_NDEBUG
#undef NDEBUG // disable ignore assert() detect
#endif // PDIAGNOSTIC_DEFINED_NDEBUG
#endif // defined(__CC_ARM) && defined(PDIAGNOSTIC_DEFINED_NDEBUG) && defined(NDEBUG)

#if defined(PDIAGNOSTIC_PUSHED_MSVC)
#if PDIAGNOSTIC_PUSHED_MSVC
#pragma warning( pop )
#endif // PDIAGNOSTIC_PUSHED_MSVC
#undef PDIAGNOSTIC_PUSHED_MSVC
#endif // defined(PDIAGNOSTIC_PUSHED_MSVC)

#if defined(PDIAGNOSTIC_PUSHED_CLANG)
#if PDIAGNOSTIC_PUSHED_CLANG
#pragma clang diagnostic pop
#endif // PDIAGNOSTIC_PUSHED_CLANG
#undef PDIAGNOSTIC_PUSHED_CLANG
#endif // defined(PDIAGNOSTIC_PUSHED_CLANG)

#if defined(PDIAGNOSTIC_PUSHED_GCC)
#if PDIAGNOSTIC_PUSHED_GCC
#if defined(__CC_ARM)
#pragma pop
#else
#pragma GCC diagnostic pop
#endif // defined(__CC_ARM )
#endif // PDIAGNOSTIC_PUSHED_GCC
#undef PDIAGNOSTIC_PUSHED_GCC
#endif // defined(PDIAGNOSTIC_PUSHED_GCC)

#endif /* PDIAGNOSTIC_POP_H */
