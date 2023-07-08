#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>
#include <cppli/cppli.hpp>

bool test_fn(int testing)
{
  return false;
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

TEST_CASE("raw command line single boolean option.")
{
  std::string command = "\"test.exe\" --option1 true";
  cppli::raw_command_line cmd(command);
  REQUIRE(cmd.has_option("option1"));
  std::vector<cppli::variant_literal> arguments;
  REQUIRE(cmd.get_option_arguments("option1", arguments));
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments[0].get_type() == cppli::variant_type::boolean);
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