#pragma once

#include <variant>
#include <string>

namespace cppli
{
    typedef std::variant<bool, int, float, std::string> variant_literal;
}