#pragma once

namespace cppli
{
namespace internal
{
  struct command_node;
  struct option_node;
  struct parameter_node;

  enum visitor_result
  {
    stop,
    continued
  };

  struct visitor
  {
    virtual visitor_result visit(command_node* node) { return visitor_result::continued; };
    virtual visitor_result visit(option_node* node) { return visitor_result::continued; };
    virtual visitor_result visit(parameter_node* node) { return visitor_result::continued; };
  };
}
}