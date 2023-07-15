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

/*
TEST_CASE("single parameter option test.")
{
  std::string command = "test.exe --option1 1";
  cppli::command_line_config config;
  cppli::command_line cli(config);
  //cli.add_option("o", "option", "this is my first option. Isn't it neat?", test_fn);
  cli.add_option("o", "option", "this is my first option. Isn't it neat?", 
  [](int value) 
  {
    printf("test! %d", value);
    return true;
  });

  REQUIRE(cli.execute(command) == true);
}*/

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
  REQUIRE(arguments[0].get_type() == cppli::variant_type::boolean);
  REQUIRE(arguments[0].get_bool() == true);
}

TEST_CASE("raw command line single boolean option false.")
{
  std::string command = "\"test.exe\" --option1 false";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::boolean);
  REQUIRE(arguments[0].get_bool() == false);
}

TEST_CASE("raw command line single integer option.")
{
  std::string command = "\"test.exe\" --option1 10";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::integer);
  REQUIRE(arguments[0].get_int() == 10);
}

TEST_CASE("raw command line single float option.")
{
  std::string command = "\"test.exe\" --option1 10.0";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::floating_point);
  REQUIRE(arguments[0].get_float() == 10.0f);
}

TEST_CASE("raw command line single float option (with f specifier).")
{
  std::string command = "\"test.exe\" --option1 10.0f";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::floating_point);
  REQUIRE(arguments[0].get_float() == 10.0f);
}

TEST_CASE("raw command line single string option.")
{
  std::string command = "\"test.exe\" --option1 \"This is a string\"";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::string);
  REQUIRE(arguments[0].get_string() == "This is a string");
}

TEST_CASE("raw command line single string identifier option.")
{
  std::string command = "\"test.exe\" --option1 ThisIsAString";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::string);
  REQUIRE(arguments[0].get_string() == "ThisIsAString");
}

TEST_CASE("raw command line single path option")
{
  std::string command = "\"test.exe\" --option1 Relative/Path";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::string);
  REQUIRE(arguments[0].get_string() == "Relative/Path");
}

TEST_CASE("raw command line single char option")
{
  std::string command = "\"test.exe\" --option1 'c'";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::string);
  REQUIRE(arguments[0].get_string() == "c");
}

TEST_CASE("raw command line with 1 option and argument with optional = sign")
{
    std::string command = "\"test.exe\" --option1=true";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::boolean);
  REQUIRE(arguments[0].get_bool() == true);
}

TEST_CASE("raw command line derive command from identifiers.")
{
  std::string command = "test.exe --option1 false";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.get_command() == "test.exe");
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::boolean);
  REQUIRE(arguments[0].get_bool() == false);
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
  REQUIRE(arguments[0].get_type() == cppli::variant_type::string);
  REQUIRE(arguments[0].get_string() == "project/plugins");
  arguments.clear();

  REQUIRE(cmd.get_option_arguments("s", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::floating_point);
  REQUIRE(arguments[0].get_float() == 10.0f);
  arguments.clear();
  
  REQUIRE(cmd.get_option_arguments("a", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::boolean);
  REQUIRE(arguments[0].get_bool() == true);
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

  REQUIRE(arguments[0].get_type() == cppli::variant_type::string);
  REQUIRE(arguments[0].get_string() == "project/plugins");

  REQUIRE(arguments[1].get_type() == cppli::variant_type::string);
  REQUIRE(arguments[1].get_string() == "project/assets");

  REQUIRE(arguments[2].get_type() == cppli::variant_type::string);
  REQUIRE(arguments[2].get_string() == "project/shaders");

  arguments.clear();

  REQUIRE(cmd.get_option_arguments("o", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::boolean);
  REQUIRE(arguments[0].get_bool() == true);
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
  REQUIRE(arguments[0].get_type() == cppli::variant_type::boolean);
  REQUIRE(arguments[0].get_bool() == true);
}

TEST_CASE("raw command line or consoel command with logging enabled.")
{
  // we expect this to exhibit a parsing error because the option name is missing.
  // It should emmit an error and fail, calling our custom logger callback.
  std::string command = "SomeInvalidCommand --";
  cppli::raw_command_line cmd(command, logger_callback_test);
  REQUIRE(cmd.is_empty());
  REQUIRE(logger_count == 1);
}