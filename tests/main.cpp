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