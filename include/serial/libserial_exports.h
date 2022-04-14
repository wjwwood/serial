#pragma once

#if defined(_WIN32)
#  if !defined(LIBSERIAL_DYNAMIC)
#    define LIBSERIAL_EXPORT_API
#  else
#    if defined(LIBSERIAL_EXPORTS)
#      define LIBSERIAL_EXPORT_API __declspec(dllexport)
#    else
#      define LIBSERIAL_EXPORT_API __declspec(dllimport)
#    endif
#  endif
#else
#  define LIBSERIAL_EXPORT_API
#endif

