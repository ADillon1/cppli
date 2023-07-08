#pragma once

#ifdef _WIN32
  #ifdef CPPLI_EXPORT
    #define CPPLI_API __declspec(dllexport)
  #else
    #define CPPLI_API __declspec(dllimport)
  #endif
#else
  #define CPPLI_API
#endif