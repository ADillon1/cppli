#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>
#include <cppli/cppli.hpp>
#include <iostream>

bool test_fn(int testing)
{
  return false;
}

static int logger_count = 0;

void logger_callback_test(const char* logging_message)
{
  ++logger_count;
  std::cout << "Console command failed with returned error: " << logging_message << std::endl;

  if (logger_count == 1)
  {
    std::cout << "Note, you should only really see one log message from the tests at the moment." << std::endl;
  }
}

TEST_CASE("Command line config default values.")
{
  cppli::command_line_config config;
  REQUIRE(config.m_application_name.empty());
  REQUIRE(config.m_application_description.empty());
  REQUIRE(config.m_logging_callback == nullptr);
  REQUIRE(config.m_major_version == -1);
  REQUIRE(config.m_minor_version == -1);
  REQUIRE(config.m_patch_version == -1);
}

TEST_CASE("Command Line single parameter bool test.")
{
  std::string command = "test.exe --option true";
  cppli::command_line_config config;
  cppli::command_line cli(config);
  bool value = false;
  cli.add_option(
    "o", 
    "option", 
    "this is my first option. Isn't it neat?", 
    false,
  [&value](bool arg) 
  {
    value = arg;
    return true;
  });

  REQUIRE(cli.execute(command));
  REQUIRE(value == true);
}

TEST_CASE("Command Line single parameter int test.")
{
  std::string command = "test.exe --option 1";
  cppli::command_line_config config;
  cppli::command_line cli(config);
  int value = 0;
  cli.add_option(
    "o", 
    "option", 
    "this is my first option. Isn't it neat?", 
    false,
  [&value](int arg) 
  {
    value = arg;
    return true;
  });

  REQUIRE(cli.execute(command));
  REQUIRE(value == 1);
}

TEST_CASE("Command Line single parameter float test.")
{
  std::string command = "test.exe --option 1.0";
  cppli::command_line_config config;
  cppli::command_line cli(config);
  float value = 0;
  cli.add_option(
    "o",
    "option",
    "this is my first option. Isn't it neat?",
    false,
  [&value](float arg) 
  {
    value = arg;
    return true;
  });

  cli.add_option(
    "o", 
    "option", 
    "this is my first option. Isn't it neat?", 
    false,
  [&value](float arg) 
  {
    value = arg;
    return true;
  });

  REQUIRE(cli.execute(command));
  REQUIRE(value == 1.0f);
}

TEST_CASE("Command Line single parameter string test.")
{
  std::string command = "test.exe --option project/assets";
  cppli::command_line_config config;
  cppli::command_line cli(config);
  std::string value;
  cli.add_option(
    "o", 
    "option", 
    "this is my first option. Isn't it neat?", 
    false,
  [&value](std::string arg) 
  {
    value = arg;
    return true;
  });

  REQUIRE(cli.execute(command));
  REQUIRE(value == "project/assets");
}

TEST_CASE("Command line vector with no parameters.")
{
  std::string command = "text.exe --option";
  cppli::command_line_config config;
  cppli::command_line cli(config);
  std::vector<bool> values;

  cli.add_option(
    "o",
    "option",
    "this is my first option. Isn't it neat?",
    false,
    [&values](const std::vector<bool>& args)
    {
      values = args;
      return true;
    }
  );

  REQUIRE(cli.execute(command));
  REQUIRE(values.size() == 0);
}

TEST_CASE("Command line vector of bool parameters.")
{
  std::string command = "test.exe --option true false true";
  cppli::command_line_config config;
  cppli::command_line cli(config);
  std::vector<bool> values;
  cli.add_option(
    "o",
    "option",
    "this is my first option. Isn't it neat?",
    false,
    [&values](const std::vector<bool>& args)
    {
      values = args;
      return true;
    }
  );

  REQUIRE(cli.execute(command));
  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == true);
  REQUIRE(values[1] == false);
  REQUIRE(values[2] == true);
}

TEST_CASE("Command line vector of int parameters.")
{
  std::string command = "test.exe --option 10 15 20";
  cppli::command_line_config config;
  cppli::command_line cli(config);
  std::vector<int> values;
  cli.add_option(
    "o",
    "option",
    "this is my first option. Isn't it neat?",
    false,
    [&values](const std::vector<int>& args)
    {
      values = args;
      return true;
    }
  );

  REQUIRE(cli.execute(command));
  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == 10);
  REQUIRE(values[1] == 15);
  REQUIRE(values[2] == 20);
}

TEST_CASE("Command line vector of float parameters.")
{
  std::string command = "test.exe --option 10.0 15.0 20.0";
  cppli::command_line_config config;
  cppli::command_line cli(config);
  std::vector<float> values;
  cli.add_option(
    "o",
    "option",
    "this is my first option. Isn't it neat?",
    false,
    [&values](const std::vector<float>& args)
    {
      values = args;
      return true;
    }
  );

  REQUIRE(cli.execute(command));
  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == 10.0);
  REQUIRE(values[1] == 15.0);
  REQUIRE(values[2] == 20.0);
}

TEST_CASE("Command line vector of string parameters.")
{
  std::string command = "test.exe --option project/assets project/source project/plugins";
  cppli::command_line_config config;
  cppli::command_line cli(config);
  std::vector<std::string> values;
  cli.add_option(
    "o",
    "option",
    "this is my first option. Isn't it neat?",
    false,
    [&values](const std::vector<std::string>& args)
    {
      values = args;
      return true;
    }
  );

  REQUIRE(cli.execute(command));
  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == "project/assets");
  REQUIRE(values[1] == "project/source");
  REQUIRE(values[2] == "project/plugins");
}

TEST_CASE("Command line type mismatch for vector of parameters.")
{
  std::string command = "test.exe --option 10 15.0 false";
  cppli::command_line_config config;
  cppli::command_line cli(config);
  cli.add_option(
    "o",
    "option",
    "this is my first option. Isn't it neat?",
    false,
    [](const std::vector<int>& args)
    {
      return true;
    }
  );

  REQUIRE(!cli.execute(command));
}

TEST_CASE("raw command line empty")
{
    cppli::raw_command_line cmd1("");
    REQUIRE(cmd1.is_empty());

    cppli::raw_command_line cmd2(nullptr);
    REQUIRE(cmd2.is_empty());
}

TEST_CASE("raw command line with no arguments or options.")
{
  std::string command = "test.command";
  cppli::raw_command_line cmd(command);
  REQUIRE(!cmd.is_empty());
  std::string command_string = cmd.get_command();
  REQUIRE(command == command_string);
}

TEST_CASE("raw command line full path command parsing.")
{
  std::string command = "C:/scripts/test/program.exe -o";
  cppli::raw_command_line cmd(command);
  REQUIRE(!cmd.is_empty());
  std::string command_string = cmd.get_command();
  REQUIRE(command_string == "C:/scripts/test/program.exe");
  REQUIRE(cmd.has_option("o"));
}

TEST_CASE("raw command line single boolean option true.")
{
  std::string command = "\"test.exe\" --option1 true";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<bool>(arguments[0]));
  REQUIRE(std::get<bool>(arguments[0]) == true);
}

TEST_CASE("raw command line single boolean option false.")
{
  std::string command = "\"test.exe\" --option1 false";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<bool>(arguments[0]));
  REQUIRE(std::get<bool>(arguments[0]) == false);
}

TEST_CASE("raw command line single integer option.")
{
  std::string command = "\"test.exe\" --option1 10";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<int>(arguments[0]));
  REQUIRE(std::get<int>(arguments[0]) == 10);
}

TEST_CASE("raw command line single float option.")
{
  std::string command = "\"test.exe\" --option1 10.0";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<float>(arguments[0]));
  REQUIRE(std::get<float>(arguments[0]) == 10.0f);
}

TEST_CASE("raw command line single float option (with f specifier).")
{
  std::string command = "\"test.exe\" --option1 10.0f";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<float>(arguments[0]));
  REQUIRE(std::get<float>(arguments[0]) == 10.0f);
}

TEST_CASE("raw command line single string option.")
{
  std::string command = "\"test.exe\" --option1 \"This is a string\"";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<std::string>(arguments[0]));
  REQUIRE(std::get<std::string>(arguments[0]) == "This is a string");
}

TEST_CASE("raw command line single string identifier option.")
{
  std::string command = "\"test.exe\" --option1 ThisIsAString";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<std::string>(arguments[0]));
  REQUIRE(std::get<std::string>(arguments[0]) == "ThisIsAString");
}

TEST_CASE("raw command line single path option")
{
  std::string command = "\"test.exe\" --option1 Relative/Path";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<std::string>(arguments[0]));
  REQUIRE(std::get<std::string>(arguments[0]) == "Relative/Path");
}

TEST_CASE("raw command line single char option")
{
  std::string command = "\"test.exe\" --option1 'c'";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<std::string>(arguments[0]));
  REQUIRE(std::get<std::string>(arguments[0]) == "c");
}

TEST_CASE("raw command line with 1 option and argument with optional = sign")
{
    std::string command = "\"test.exe\" --option1=true";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<bool>(arguments[0]));
  REQUIRE(std::get<bool>(arguments[0]) == true);
}

TEST_CASE("raw command line derive command from identifiers.")
{
  std::string command = "test.exe --option1 false";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.get_command() == "test.exe");
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<bool>(arguments[0]));
  REQUIRE(std::get<bool>(arguments[0]) == false);
}

TEST_CASE("raw command line multiple options.")
{
  std::string command = "test.exe -d project/plugins -s 10.0f -a true";
  cppli::raw_command_line cmd(command);
  REQUIRE(!cmd.is_empty());
  REQUIRE(cmd.get_command() == "test.exe");
  std::vector<cppli::variant_literal> arguments;

  REQUIRE(cmd.get_option_arguments("d", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<std::string>(arguments[0]));
  REQUIRE(std::get<std::string>(arguments[0]) == "project/plugins");
  arguments.clear();

  REQUIRE(cmd.get_option_arguments("s", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<float>(arguments[0]));
  REQUIRE(std::get<float>(arguments[0]) == 10.0f);
  arguments.clear();
  
  REQUIRE(cmd.get_option_arguments("a", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<bool>(arguments[0]));
  REQUIRE(std::get<bool>(arguments[0]) == true);
  arguments.clear();
}

TEST_CASE("raw command line with options that have multiple arguments.")
{
  std::string command = "test.exe -d project/plugins project/assets project/shaders -o true";
  cppli::raw_command_line cmd(command);
  REQUIRE(!cmd.is_empty());
  std::string command_string = cmd.get_command();
  REQUIRE(command_string == "test.exe");
  REQUIRE(cmd.has_option("d"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("d", arguments));
  REQUIRE(arguments.size() == 3);

  REQUIRE(std::holds_alternative<std::string>(arguments[0]));
  REQUIRE(std::get<std::string>(arguments[0]) == "project/plugins");

  REQUIRE(std::holds_alternative<std::string>(arguments[1]));
  REQUIRE(std::get<std::string>(arguments[1]) == "project/assets");

  REQUIRE(std::holds_alternative<std::string>(arguments[2]));
  REQUIRE(std::get<std::string>(arguments[2]) == "project/shaders");

  arguments.clear();

  REQUIRE(cmd.get_option_arguments("o", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<bool>(arguments[0]));
  REQUIRE(std::get<bool>(arguments[0]) == true);
}

TEST_CASE("raw runtime console command with parameters but no options.")
{
  std::string command = "SetShowFPS true";
  cppli::raw_command_line cmd(command);
  REQUIRE(!cmd.is_empty());
  REQUIRE(cmd.get_command() == "SetShowFPS");
  std::vector<cppli::variant_literal> arguments;
  cmd.get_command_line_arguments(arguments);
  REQUIRE(arguments.size() == 1);
  REQUIRE(std::holds_alternative<bool>(arguments[0]));
  REQUIRE(std::get<bool>(arguments[0]) == true);
}

TEST_CASE("raw command line or console command with logging enabled.")
{
  // we expect this to exhibit a parsing error because the option name is missing.
  // It should emmit an error and fail, calling our custom logger callback.
  std::string command = "SomeInvalidCommand --";
  cppli::raw_command_line cmd(command, logger_callback_test);
  REQUIRE(cmd.is_empty());
  REQUIRE(logger_count == 1);
}

TEST_CASE("Handler for directory strings with - characters.")
{
  std::string command1 = "test.exe -d=C:/relative-path/foo";
  std::string command2 = "test.exe -d=\"C:/relative-path/foo\"";

  cppli::raw_command_line cmd1(command1);
  cppli::raw_command_line cmd2(command2);

  std::vector<cppli::variant_literal> cmd1_args;
  std::vector<cppli::variant_literal> cmd2_args;

  REQUIRE(cmd1.get_option_arguments("d", cmd1_args));
  REQUIRE(cmd1_args.size() == 1);
  REQUIRE(std::holds_alternative<std::string>(cmd1_args[0]));
  REQUIRE(std::get<std::string>(cmd1_args[0]) == "C:/relative-path/foo");


  REQUIRE(cmd2.get_option_arguments("d", cmd2_args));
  REQUIRE(cmd2_args.size() == 1);
  REQUIRE(std::holds_alternative<std::string>(cmd2_args[0]));
  REQUIRE(std::get<std::string>(cmd2_args[0]) == "C:/relative-path/foo");
}