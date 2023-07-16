#include <cppli/cppli.hpp>
#include <cppli/internal/nodes.hpp>
#include <cppli/internal/variant.hpp>
#include <tokenize/tokenize.hpp>
#include <cppli/internal/visitor.hpp>
#include <exception>

namespace cppli
{
namespace internal
{
    struct get_options_and_parameters_visitor : public visitor
    {
      std::string m_option_to_find;
      bool m_found = false;
      std::vector<variant_literal> m_option_parameters;

      virtual visitor_result visit(option_node* node) 
      {
        if (node->m_name == m_option_to_find)
        {
          m_found = true;

          for (size_t i = 0; i < node->m_parameters.size(); ++i)
          {
            m_option_parameters.push_back(node->m_parameters[i]->m_varient);
          }
        }

        return visitor_result::stop;
      }
    };

  struct get_all_options_visitor : public visitor
  {
    std::vector<std::string>& m_options;

    get_all_options_visitor(std::vector<std::string>& options)
      : m_options(options)
    {
    }

    virtual visitor_result visit(option_node* node)
    {
      m_options.push_back(node->m_name);
      return visitor_result::stop;
    }
  };

  struct get_command_and_parameters_visitor : public visitor
  {
    std::vector<variant_literal> m_command_parameters;

    virtual visitor_result visit(option_node* node) 
    {
      return visitor_result::stop; 
    }

    virtual visitor_result visit(parameter_node* node) 
    {
      m_command_parameters.push_back(node->m_varient);
      return visitor_result::continued;
    }
  };
}

  bool raw_command_line::parse_path(tokenize::parsing_context& context, std::string& out_path)
  {
    int begin = context.m_current_token;
    out_path.clear();

    if (context.accept(tokenize::token_id::string_literal))
    {
      out_path.assign(context.get_previous_token().m_stream + 1, context.get_previous_token().m_length - 2);
      return true;
    }

    while (!context.end_of_token_stream())
    {
      if (
        context.accept(tokenize::token_id::identifier, false)     ||
        context.accept(tokenize::token_id::forward_slash, false)  ||
        context.accept(tokenize::token_id::division, false)       ||
        context.accept(tokenize::token_id::colon, false)          ||
        context.accept(tokenize::token_id::member_access, false))
      {
        out_path += std::string(context.get_previous_token().m_stream, context.get_previous_token().m_length);
      }
      else
      {
        break;
      }
    }

    if (out_path.size() != 0)
    {
      if (!context.end_of_token_stream())
      {
        context.accept(tokenize::token_id::whitespace);
      }

      return true;
    }

    context.set_current_token_index(begin);
    return false;
  }
  
  std::unique_ptr<internal::command_node> raw_command_line::parse_command()
  {
    try
    {
      tokenize::dfa_cpp dfa;
      tokenize::parsing_context context;
      tokenize::from_string(m_command_string, dfa, context.m_token_context);
      std::string command_string;
      if (parse_path(context, command_string))
      {
        // Remove whitespace.
        context.remove_tokens(tokenize::token_id::new_line);
        context.remove_tokens(tokenize::token_id::single_line_comment);
        context.remove_tokens(tokenize::token_id::multi_line_comment);

        std::unique_ptr<internal::command_node> command_node = std::make_unique<internal::command_node>();
        command_node->m_name = command_string;

        int current_token_index = context.m_current_token;

        while (!context.end_of_token_stream())
        {
          if (std::unique_ptr<internal::ast_node> next_expression = parse_expression(context))
          {
            command_node->m_expressions.push_back(std::move(next_expression));
          }

          // hacky fix - break out of an infinite loop if parsing didn't advance in the stream.
          int new_token_index = context.m_current_token;
          if (new_token_index == current_token_index)
          {
            throw std::exception("failed to parse command!");
          }
        }

        return std::move(command_node);
      }
    }
    catch(const std::exception& e)
    {
      if (m_logging_callback)
      {
        m_logging_callback(e.what());
      }
      return nullptr;
    }
    
    return nullptr;
  }
  
  std::unique_ptr<internal::ast_node> raw_command_line::parse_expression(tokenize::parsing_context& context)
  {
    std::unique_ptr<internal::ast_node> expression;

    if (expression = parse_option(context))
    {
      return std::move(expression);
    }

    if (expression = parse_parameter(context))
    {
      return std::move(expression);
    }

    return nullptr;
  }
  
  std::unique_ptr<internal::option_node> raw_command_line::parse_option(tokenize::parsing_context& context)
  {
    if (context.accept(tokenize::token_id::subtraction, false) || context.accept(tokenize::token_id::decrement, false))
    {
      context.expect(tokenize::token_id::identifier, std::string("Expected an identifier for command line option."));
      std::unique_ptr<internal::option_node> option_node = std::make_unique<internal::option_node>();
      option_node->m_name = std::string(context.get_previous_token().m_stream, context.get_previous_token().m_length);

      bool equal_sign = false; 
      
      if (!context.end_of_token_stream())
      {
        equal_sign = context.accept(tokenize::token_id::direct_assignment);
      }

      while (!context.end_of_token_stream())
      {
        std::unique_ptr<internal::parameter_node> next_parameter = parse_parameter(context);

        if (!next_parameter)
        {
          break;
        }

        option_node->m_parameters.push_back(std::move(next_parameter));
      }

      if (equal_sign)
      {
        context.expect(option_node->m_parameters.size() > 0, "An option with an equal sign must have at least 1 argument.");
      }

      return std::move(option_node);
    }

    return nullptr;
  }
  
  std::unique_ptr<internal::parameter_node> raw_command_line::parse_parameter(tokenize::parsing_context& context)
  {
    std::string parameter_path;

    if (parse_path(context, parameter_path))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name = parameter_path;
      parameter_node->m_varient = parameter_node->m_name;
      return std::move(parameter_node);
    }

    if (context.accept(tokenize::token_id::string_literal))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream + 1, context.get_previous_token().m_length - 2);
      parameter_node->m_varient = parameter_node->m_name;
      return std::move(parameter_node);
    }
    else if (context.accept(tokenize::token_id::integer_literal))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream, context.get_previous_token().m_length);
      parameter_node->m_varient = atoi(parameter_node->m_name.c_str());
      return std::move(parameter_node);
    }
    if (context.accept(tokenize::token_id::float_literal))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream, context.get_previous_token().m_length);
      parameter_node->m_varient = static_cast<float>(atof(parameter_node->m_name.c_str()));
      return std::move(parameter_node);
    }
    else if (context.accept(tokenize::token_id::character_literal))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream + 1, context.get_previous_token().m_length - 2);
      parameter_node->m_varient = parameter_node->m_name;
      return std::move(parameter_node);
    }
    else if (context.accept(tokenize::token_id::hex_literal))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream, context.get_previous_token().m_length);
      parameter_node->m_varient = atoi(parameter_node->m_name.c_str());
      return std::move(parameter_node);
    }
    else if (context.accept(tokenize::token_id::binary_literal))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream, context.get_previous_token().m_length);
      parameter_node->m_varient = atoi(parameter_node->m_name.c_str());
      return std::move(parameter_node);
    }
    else if (context.accept(tokenize::token_id::_true))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream, context.get_previous_token().m_length);
      parameter_node->m_varient = parameter_node->m_name == "true" ? true : false;
      return std::move(parameter_node);
    }
    else if (context.accept(tokenize::token_id::_false))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream, context.get_previous_token().m_length);
      parameter_node->m_varient = parameter_node->m_name == "true" ? true : false;
      return std::move(parameter_node);
    }

    return nullptr;
  }

  bool command_line::internal_execute()
  {
    if (m_internal_command_line->is_empty())
    {
      return false;
    }

    bool success = true;

    // loop through each option, check if its represented and execute if able.

    for (auto& option : m_options)
    {
      std::vector<variant_literal> arguments;
      if (m_internal_command_line->get_option_arguments({option->m_shorthand, option->m_name}, arguments))
      {
        success &= option->execute(m_config, arguments);
      }
      else if (option->m_is_required)
      {
        success = false;
      }
    }

    return success;
  }

  command_line::command_line(const command_line_config& config)
    : m_config(config)
    , m_internal_command_line(nullptr)
    , m_options()
  {

  }

  bool command_line::execute(int argc, char** argv)
  {
    if (!m_internal_command_line)
    {
      m_internal_command_line = std::make_unique<raw_command_line>(argc, argv, m_config.m_logging_callback);
      return internal_execute();
    }

    return false;
  }

  bool command_line::execute(std::string& command)
  {
    if (!m_internal_command_line)
    {
      m_internal_command_line = std::make_unique<raw_command_line>(command, m_config.m_logging_callback);
      return internal_execute();
    }

    return false;
  }

  raw_command_line::raw_command_line()
    : m_command_string()
    , m_root_command()
    , m_logging_callback()
  {
  }
  
  raw_command_line::raw_command_line(raw_command_line&& rhs)
    : m_command_string(std::move(rhs.m_command_string))
    , m_root_command(std::move(rhs.m_root_command))
    , m_logging_callback(std::move(rhs.m_logging_callback))
  {
  }

  raw_command_line::raw_command_line(int argc, char** argv, logger_callback_fn logging_callback)
  {
    m_command_string = '"';
    m_command_string += argv[0];
    m_command_string += '"';
    m_command_string += ' ';

    for (int i = 1; i < argc; ++i)
    {
      m_command_string += argv[i];
      m_command_string += ' ';
    }

    m_logging_callback = logging_callback;
    m_root_command = parse_command();
  }

  raw_command_line::raw_command_line(const std::string& command, logger_callback_fn logging_callback)
  {
    m_command_string = command;
    m_logging_callback = logging_callback;
    m_root_command = parse_command();
  }

  raw_command_line::raw_command_line(const nullptr_t& null)
    : m_command_string()
    , m_root_command()
  {

  }

  raw_command_line& raw_command_line::operator=(const raw_command_line& rhs)
  {
    m_command_string = rhs.m_command_string;
    m_root_command = parse_command();
    return *this;
  }
  
  bool raw_command_line::is_empty() const
  {
    return !m_root_command.get();
  }

  std::string raw_command_line::get_command() const
  {
    return m_root_command.get() ? m_root_command->m_name : "";
  }

  bool raw_command_line::get_all_options(std::vector<std::string>& out_options) const
  {
    if (is_empty())
    {
      return false;
    }

    internal::get_all_options_visitor visitor(out_options);
    m_root_command->walk(&visitor);
    return true;
  }

  bool raw_command_line::has_option(const std::string& in_option) const
  {
    if (is_empty())
    {
      return false;
    }

    internal::get_options_and_parameters_visitor visitor;
    visitor.m_option_to_find = in_option;
    m_root_command->walk(&visitor);
    return visitor.m_found;
  }

  bool raw_command_line::get_option_arguments(const std::string& in_option, std::vector<variant_literal>& out_arguments) const
  {
    if (is_empty())
    {
      return false;
    }

    out_arguments.clear();

    internal::get_options_and_parameters_visitor visitor;
    visitor.m_option_to_find = in_option;
    visitor.m_found = false;
    m_root_command->walk(&visitor);
    out_arguments.insert(out_arguments.end(), visitor.m_option_parameters.begin(), visitor.m_option_parameters.end());
    return visitor.m_found;
  }

  bool raw_command_line::get_option_arguments(const std::vector<std::string>& in_options, std::vector<variant_literal>& out_arguments) const
  {
    if (is_empty())
    {
      return false;
    }

    out_arguments.clear();

    bool has_option = false;

    for (int i = 0; i < in_options.size(); ++i)
    {
      std::vector<variant_literal> option_arguments;
      has_option |= get_option_arguments(in_options[i], option_arguments);
      out_arguments.insert(out_arguments.end(), option_arguments.begin(), option_arguments.end());
    }

    return has_option;
  }

  void raw_command_line::get_command_line_arguments(std::vector<variant_literal>& out_arguments)
  {
    if (is_empty())
    {
      return;
    }

    internal::get_command_and_parameters_visitor visitor;
    m_root_command->walk(&visitor);
    out_arguments = visitor.m_command_parameters;
  }
}
