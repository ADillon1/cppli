#pragma once

#include <cppli/internal/module.hpp>
#include <string>

#ifdef _WIN32
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace cppli
{
enum variant_type
{
    integer,
    floating_point,
    string,
    boolean,
    invalid
};

class CPPLI_API variant_literal
{
  std::string m_as_string;
  int m_as_int;
  float m_as_float;
  bool m_as_bool;
  variant_type m_type;

public:
  variant_literal();
  variant_literal(const std::string& text, variant_type type);
  variant_type get_type() const;
  int get_int() const;
  float get_float() const;
  bool get_bool() const;
  const std::string& get_string() const;
};
}

#ifdef _WIN32
#  pragma warning( pop )
#endif