#include <cppli/internal/nodes.hpp>
#include <cppli/internal/visitor.hpp>

namespace cppli
{
namespace internal
{
  void ast_node::walk(visitor* visitor, bool visit /*= true*/)
    {
    }

    void command_node::walk(visitor* visitor, bool visit /*= true*/)
    {
      if (visit && visitor->visit(this) == visitor_result::stop)
      {
        return;
      }

      ast_node::walk(visitor, false);

      for (unsigned i = 0; i < m_expressions.size(); ++i)
      {
        m_expressions[i]->walk(visitor);
      }
    }

    void option_node::walk(visitor* visitor, bool visit /*= true*/)
    {
      if (visit && visitor->visit(this) == visitor_result::stop)
      {
        return;
      }

      ast_node::walk(visitor, false);

      for (unsigned i = 0; i < m_parameters.size(); ++i)
      {
        m_parameters[i]->walk(visitor);
      }
    }

    void parameter_node::walk(visitor* visitor, bool visit /*= true*/)
    {
      if (visit && visitor->visit(this) == visitor_result::stop)
      {
        return;
      }

      ast_node::walk(visitor, false);
    }
}
}