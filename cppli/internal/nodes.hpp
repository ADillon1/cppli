#pragma once

#include <cppli/internal/variant.hpp>
#include <string>
#include <vector>
#include <memory>

namespace cppli
{
namespace internal
{
  struct visitor;
  struct parameter_node;

  struct ast_node
  {
    std::string m_name;
    ast_node* m_parent_node;
    virtual void walk(visitor* visitor, bool visit = true);
  };

  struct command_node : public ast_node
  {
    std::vector<std::unique_ptr<ast_node>> m_expressions;
    virtual void walk(visitor* visitor, bool visit = true) override;
  };

  struct option_node : ast_node
  {
    std::vector<std::unique_ptr<parameter_node>> m_parameters;
    virtual void walk(visitor* visitor, bool visit = true) override;
  };

  struct parameter_node : ast_node
  {
    variant_type m_type;
    virtual void walk(visitor* visitor, bool visit = true) override;
  };
}
}