#include <cppli/internal/variant.hpp>

namespace cppli
{
variant_literal::variant_literal()
  : m_type(variant_type::invalid)
  , m_as_string()
  , m_as_int(0)
  , m_as_float(0)
  , m_as_bool(false)
{
}

variant_literal::variant_literal(const std::string& text, variant_type type)
  : m_type(type)
  , m_as_string()
  , m_as_int(0)
  , m_as_float(0)
  , m_as_bool(false)
{
  if (type == variant_type::string)
  {
    m_as_string = text;
  }
  else if (type == variant_type::integer)
  {
    m_as_int = atoi(text.c_str());
  }
  else if (type == variant_type::floating_point)
  {
    m_as_float = static_cast<float>(atof(text.c_str()));
  }
  else if (type == variant_type::boolean)
  {
    m_as_bool = text == "true" ? true : false;
  }
}

variant_type variant_literal::get_type() const
{
  return m_type;
}

int variant_literal::get_int() const
{
  return m_as_int;
}

float variant_literal::get_float() const
{
  return m_as_float;
}

bool variant_literal::get_bool() const
{
  return m_as_bool;
}

const std::string& variant_literal::get_string() const
{
  return m_as_string;
}
}