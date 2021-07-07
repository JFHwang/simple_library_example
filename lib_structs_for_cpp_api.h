#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#elif defined(__GNUC__) || defined(__GNUG__)
// Can't turn off the variadic macro warning emitted from -pedantic
#  pragma GCC system_header
#elif defined(_MSC_VER)
// Doesn't seem to emit the warning
#else
// Don't know the compiler... just let it go through
#endif

// clang-format off
#define sandbox_fields_reflection_lib_class_ImageHeader(f, g, ...)          \
  f(unsigned int, status_code, FIELD_NORMAL, ##__VA_ARGS__) g()	               \
  f(unsigned int, width, FIELD_NORMAL, ##__VA_ARGS__) g()	               \
  f(unsigned int, height, FIELD_NORMAL, ##__VA_ARGS__) g()	               

#define sandbox_fields_reflection_lib_allClasses(f, ...)                   \
  f(ImageHeader, lib, ##__VA_ARGS__)                                        

// clang-format on

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#elif defined(_MSC_VER)
#else
#endif
