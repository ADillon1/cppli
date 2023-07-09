#include <cppli/cppli.hpp>
#include <cppli/internal/nodes.hpp>
#include <cppli/internal/variant.hpp>
#include <tokenize/tokenize.hpp>
#include <cppli/internal/visitor.hpp>
#include <exception>
#include <iostream>

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
            m_option_parameters.push_back(variant_literal(node->m_parameters[i]->m_name, node->m_parameters[i]->m_type));
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
      m_command_parameters.push_back(variant_literal(node->m_name, node->m_type));
      return visitor_result::continued;
    }
  };

  struct command_parsing_context
  {
    token_stream_context m_token_context;
    int m_current_token = 0;
    int m_previous_token = 0;

    bool end_of_token_stream()
    {
      return m_current_token >= m_token_context.m_tokens.size();
    }

    const token& get_current_token()
    {
      return m_token_context.m_tokens[m_current_token];
    }

    const token& get_previous_token()
    {
      return m_token_context.m_tokens[m_previous_token];
    }

    void set_current_token_index(int new_index = 0)
    {
      m_current_token = std::min(static_cast<int>(m_token_context.m_tokens.size()) - 1, new_index);
      m_previous_token = m_current_token;
    }

    void advance_token_stream(bool skip_whitespace_and_comments = true)
    {
      m_previous_token = m_current_token;
      ++m_current_token;

      if (skip_whitespace_and_comments && !end_of_token_stream())
      {
        while (m_token_context.m_tokens[m_current_token].m_id == e_token_id::new_line ||
          m_token_context.m_tokens[m_current_token].m_id == e_token_id::whitespace ||
          m_token_context.m_tokens[m_current_token].m_id == e_token_id::single_line_comment ||
          m_token_context.m_tokens[m_current_token].m_id == e_token_id::multi_line_comment)
        {
          ++m_current_token;

          if (end_of_token_stream())
          {
            break;
          }
        }
      }
    }

    bool accept(const std::string& identifier, bool skip_whitespace_and_comments = true)
    {
      expect(!end_of_token_stream(), "Unexpected end of stream.");

      if (std::string(m_token_context.m_tokens[m_current_token].m_stream, m_token_context.m_tokens[m_current_token].m_length) == identifier)
      {
        advance_token_stream(skip_whitespace_and_comments);
        return true;
      }

      return false;
    }

    bool accept(e_token_id id, bool skip_whitespace_and_comments = true)
    {
      expect(!end_of_token_stream(), "Unexpected end of stream.");

      if (m_token_context.m_tokens[m_current_token].m_id == id)
      {
        advance_token_stream(skip_whitespace_and_comments);
        return true;
      }

     return false;
    }

    bool expect(e_token_id id, const std::string& error_message, bool skip_whitespace_and_comments = true)
    {
      return expect(accept(id, skip_whitespace_and_comments), error_message);
    }

    bool expect(bool expression, const std::string& error_message)
    {
      if (expression)
      {
        return true;
      }

      // TODO: Exception Handler Class
      if (end_of_token_stream())
      {
        throw std::exception(error_message.c_str());
        //throw CHeaderToolException(CurrentFilePath, CurrentLineNumber, InErrorMessage);
      }
      else
      {
        token& error_token = m_token_context.m_tokens[m_previous_token];
        //throw CHeaderToolException(ErrorToken.FilePath, ErrorToken.LineNumber, InErrorMessage);
        throw std::exception(error_message.c_str());
      }
    }

    void remove_identifier_tokens(const std::vector<std::string>& identifiers)
    {
      for (size_t i = 0; i < m_token_context.m_tokens.size();)
      {
        bool found = false;

        if (m_token_context.m_tokens[i].m_id == e_token_id::identifier)
        {
          std::string token_string = std::string(m_token_context.m_tokens[i].m_stream, m_token_context.m_tokens[i].m_length);

          for (int j = 0; j < identifiers.size(); ++j)
          {
            if (token_string == identifiers[j])
            {
              remove_tokens(i, i + 1);
              found = true;
              break;
            }
          }
        }

        if (!found)
        {
          ++i;
        }
      }
    }

    void remove_tokens(int start_index, int end_index)
    {
      if (start_index >= end_index || start_index < 0 || end_index >= m_token_context.m_tokens.size())
      {
        return;
      }

      m_token_context.m_tokens.erase(m_token_context.m_tokens.begin() + start_index + 1, m_token_context.m_tokens.begin() + end_index);
    }

    void remove_tokens(e_token_id id)
    {
      std::vector<token> new_list;
      new_list.reserve(m_token_context.m_tokens.size());
      for (size_t i = 0; i < m_token_context.m_tokens.size(); ++i)
      {
        if (m_token_context.m_tokens[i].m_id != id)
        {
          new_list.push_back(m_token_context.m_tokens[i]);
        }
      }

      m_token_context.m_tokens = new_list;
    }

  };
}

  bool raw_command_line::parse_path(internal::command_parsing_context& context, std::string& out_path)
  {
    int begin = context.m_current_token;
    out_path.clear();

    if (context.accept(e_token_id::string_literal))
    {
      out_path.assign(context.get_previous_token().m_stream + 1, context.get_previous_token().m_length - 2);
      return true;
    }

    while (!context.end_of_token_stream())
    {
      if (
        context.accept(e_token_id::identifier, false)     ||
        context.accept(e_token_id::forward_slash, false)  ||
        context.accept(e_token_id::division, false)       ||
        context.accept(e_token_id::colon, false)          ||
        context.accept(e_token_id::member_access, false))
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
        context.accept(e_token_id::whitespace);
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
      dfa_cpp dfa;
      internal::command_parsing_context context;
      tokenize::string(m_command_string, dfa, context.m_token_context);
      std::string command_string;
      if (parse_path(context, command_string))
      {
        // Remove whitespace.
        context.remove_tokens(e_token_id::new_line);
        context.remove_tokens(e_token_id::single_line_comment);
        context.remove_tokens(e_token_id::multi_line_comment);

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
      std::cout << e.what() << '\n';
      return nullptr;
    }
    
  }
  
  std::unique_ptr<internal::ast_node> raw_command_line::parse_expression(internal::command_parsing_context& context)
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
  
  std::unique_ptr<internal::option_node> raw_command_line::parse_option(internal::command_parsing_context& context)
  {
    if (context.accept(e_token_id::subtraction, false) || context.accept(e_token_id::decrement, false))
    {
      context.expect(e_token_id::identifier, std::string("Expected an identifier for command line option."));
      std::unique_ptr<internal::option_node> option_node = std::make_unique<internal::option_node>();
      option_node->m_name = std::string(context.get_previous_token().m_stream, context.get_previous_token().m_length);

      while (!context.end_of_token_stream())
      {
        std::unique_ptr<internal::parameter_node> next_parameter = parse_parameter(context);

        if (!next_parameter)
        {
          break;
        }

        option_node->m_parameters.push_back(std::move(next_parameter));
      }

      return std::move(option_node);
    }

    return nullptr;
  }
  
  std::unique_ptr<internal::parameter_node> raw_command_line::parse_parameter(internal::command_parsing_context& context)
  {
    std::string parameter_path;

    if (parse_path(context, parameter_path))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name = parameter_path;
      parameter_node->m_type = variant_type::string;
      return std::move(parameter_node);
    }

    if (context.accept(e_token_id::string_literal))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream + 1, context.get_previous_token().m_length - 2);
      parameter_node->m_type = variant_type::string;
      return std::move(parameter_node);
    }
    else if (context.accept(e_token_id::integer_literal))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream, context.get_previous_token().m_length);
      parameter_node->m_type = variant_type::integer;
      return std::move(parameter_node);
    }
    if (context.accept(e_token_id::float_literal))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream, context.get_previous_token().m_length);
      parameter_node->m_type = variant_type::floating_point;
      return std::move(parameter_node);
    }
    else if (context.accept(e_token_id::character_literal))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream + 1, context.get_previous_token().m_length - 2);
      parameter_node->m_type = variant_type::string;
      return std::move(parameter_node);
    }
    else if (context.accept(e_token_id::hex_literal))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream, context.get_previous_token().m_length);
      parameter_node->m_type = variant_type::integer;
      return std::move(parameter_node);
    }
    else if (context.accept(e_token_id::binary_literal))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream, context.get_previous_token().m_length);
      parameter_node->m_type = variant_type::integer;
      return std::move(parameter_node);
    }
    else if (context.accept(e_token_id::_true))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream, context.get_previous_token().m_length);
      parameter_node->m_type = variant_type::boolean;
      return std::move(parameter_node);
    }
    else if (context.accept(e_token_id::_false))
    {
      std::unique_ptr<internal::parameter_node> parameter_node = std::make_unique<internal::parameter_node>();
      parameter_node->m_name.assign(context.get_previous_token().m_stream, context.get_previous_token().m_length);
      parameter_node->m_type = variant_type::boolean;
      return std::move(parameter_node);
    }

    return nullptr;
  }

  command_line::command_line(const command_line_config& config)
  {

  }

  bool command_line::execute(int argc, char** argv)
  {
    return false;
  }

  bool command_line::execute(std::string& command)
  {
    return false;
  }

  raw_command_line::raw_command_line()
    : m_command_string()
    , m_root_command()
  {
  }

  raw_command_line::raw_command_line(int argc, char** argv)
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

    m_root_command = parse_command();
  }

  raw_command_line::raw_command_line(const std::string& command)
  {
    m_command_string = command;
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
