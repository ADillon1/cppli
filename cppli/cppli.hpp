#pragma once

#ifdef _WIN32
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

#include <cppli/internal/module.hpp>
#include <cppli/internal/nodes.hpp>
#include <cppli/internal/variant.hpp>
#include <vector>
#include <functional>
#include <string>
#include <typeinfo>
#include <memory>

namespace tokenize
{
  struct parsing_context;
}

namespace cppli
{
  typedef void (*logger_callback_fn)(const char* logging_message);

    struct command_line_config
  {
    std::string m_application_name;
    std::string m_application_description;
    int m_major_version;
    int m_minor_version;
    int m_patch_version;
    logger_callback_fn m_logging_callback;
  };

namespace internal
{
  struct ast_node;
  struct command_node;
  struct option_node;
  struct parameter_node;

  template<typename> struct function_traits;

	template <typename Function>
	struct function_traits : public function_traits<decltype(&std::remove_reference<Function>::type::operator())>
  {
  };

	template <typename ClassType, typename ReturnType, typename... Arguments>
	struct function_traits<ReturnType(ClassType::*)(Arguments...) const> 
    : function_traits<ReturnType(*)(Arguments...)>
  {
  };

  /* support the non-const operator ()
   * this will work with user defined functors */
	template <typename ClassType, typename ReturnType, typename... Arguments>
	struct function_traits<ReturnType(ClassType::*)(Arguments...)> 
    : function_traits<ReturnType(*)(Arguments...)> 
  {
  };

	template <typename ReturnType, typename... Arguments>
	struct function_traits<ReturnType(*)(Arguments...)>
  {
		typedef ReturnType result_type;
		template <std::size_t Index>
		using argument = typename std::tuple_element<Index, std::tuple<Arguments...>>::type;
		static const std::size_t arity = sizeof...(Arguments);
	};

    struct function_wrapper_base
    {
      std::string m_shorthand;
      std::string m_name;
      std::string m_description;
      bool m_is_required;

      function_wrapper_base(
        std::string shorthand,
        std::string name,
        std::string description,
        bool is_required
        )
          : m_shorthand(shorthand)
          , m_name(name)
          , m_description(description)
          , m_is_required(is_required)
        {
        }

      virtual ~function_wrapper_base() = default;

      virtual bool execute(const command_line_config& config, const std::vector<variant_literal>& arguments) = 0;
    };

    template <typename param_type>
    struct function_wrapper_single_param : public function_wrapper_base
    {

      std::function<bool(param_type)> m_function;

      function_wrapper_single_param(
        const std::string& shorthand,
        const std::string& name,
        const std::string& description,
        bool is_required,
        std::function<bool(param_type)>&& f) 
        : function_wrapper_base(shorthand, name, description, is_required)
        , m_function(f) 
        {
        }
      
      virtual bool execute(const command_line_config& config, const std::vector<variant_literal>& arguments)
      {
        if (arguments.size() != 1)
        {
          // Mismatched argument count.
          return false;
        }

        if (!std::holds_alternative<param_type>(arguments[0]))
        {
          // Param does not match type.
          return false;
        }

        return m_function(std::get<param_type>(arguments[0]));
      }
    };
  }

  class CPPLI_API raw_command_line
  {
    std::string m_command_string;
    std::unique_ptr<internal::command_node> m_root_command;
    logger_callback_fn m_logging_callback = nullptr;

    bool parse_path(tokenize::parsing_context& context, std::string& out_path);
    std::unique_ptr<internal::command_node> parse_command();
    std::unique_ptr<internal::ast_node> parse_expression(tokenize::parsing_context& context);
    std::unique_ptr<internal::option_node> parse_option(tokenize::parsing_context& context);
    std::unique_ptr<internal::parameter_node> parse_parameter(tokenize::parsing_context& context);

  public:
    raw_command_line();

    raw_command_line(int argc, char** argv, logger_callback_fn logging_callback = nullptr);

    raw_command_line(const std::string& command, logger_callback_fn logging_callback = nullptr);

    raw_command_line(const nullptr_t& null);

    raw_command_line(raw_command_line&& rhs);

    raw_command_line& operator=(const raw_command_line& rhs);

    bool is_empty() const;

    std::string get_command() const;

    bool get_all_options(std::vector<std::string>& out_options) const;

    bool has_option(const std::string& in_option) const;

    bool get_option_arguments(const std::string& in_option, std::vector<variant_literal>& OutArguments) const;

    bool get_option_arguments(const std::vector<std::string>& in_options, std::vector<variant_literal>& out_arguments) const;

    void get_command_line_arguments(std::vector<variant_literal>& OutArguments);
  };

  class CPPLI_API command_line
  {
    command_line_config m_config;
    std::unique_ptr<raw_command_line> m_internal_command_line;
    std::vector<std::shared_ptr<internal::function_wrapper_base>> m_options;

    bool internal_execute();

    public:

    command_line(const command_line_config& config);
    bool execute(int argc, char** argv);
    bool execute(std::string& command);

    template<typename fn>
    void add_option(const std::string& short_hand, const std::string& name, const std::string& description, bool required, fn callback)
    {
      using traits = internal::function_traits<fn>;
      static_assert(traits::arity == 1, "Command line callbacks can only support either a single argument or a vector of a supported argument type.");
      static_assert(std::is_same_v<typename traits::result_type, bool>, "failed...");

      using option_type = typename traits::template argument<0>;
      constexpr bool is_bool = std::is_same_v<option_type, bool>;
      constexpr bool is_int = std::is_same_v<option_type, int>;
      constexpr bool is_float = std::is_same_v<option_type, float>;
      constexpr bool is_string = std::is_same_v<option_type, std::string>;
      constexpr bool is_vector_bool = std::is_same_v<option_type, std::vector<bool>>;
      constexpr bool is_vector_int =  std::is_same_v<option_type, std::vector<int>>;
      constexpr bool is_vector_float = std::is_same_v<option_type, std::vector<float>>;
      constexpr bool is_vector_string = std::is_same_v<option_type, std::vector<std::string>>;
      constexpr bool is_vector = is_vector_bool || is_vector_int || is_vector_float || is_vector_string;

      static_assert(
        is_bool          ||
        is_int           ||
        is_float         ||
        is_string        ||
        is_vector_bool   ||
        is_vector_int    ||
        is_vector_float  ||
        is_vector_string 
      , "The argument for the callback can only be a bool, int, float, string, or a vector of any of the previously mentioned types.");

      if constexpr (traits::arity == 1)
      {
        m_options.push_back(std::make_shared<internal::function_wrapper_single_param<option_type>>(
              short_hand, 
              name, 
              description, 
              required, 
              std::move(callback)));
      }
    }
  };
}

#ifdef _WIN32
#  pragma warning( pop )
#endif