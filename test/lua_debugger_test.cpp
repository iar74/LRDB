
#include <iostream>

#include "lrdb/debugger.hpp"

#include "gtest/gtest.h"

namespace {

class DebuggerTest : public ::testing::Test {
 protected:
  DebuggerTest() {}

  virtual ~DebuggerTest() {}

  virtual void SetUp() {
    L = luaL_newstate();
    luaL_openlibs(L);
    debugger.reset(L);
  }

  virtual void TearDown() {
    debugger.reset();
    lua_close(L);
    L = 0;
  }
  lua_State* L;
  lrdb::debugger debugger;
};
}
TEST_F(DebuggerTest, BreakPointTest1) {
  const char* TEST_LUA_SCRIPT = "../test/lua/test1.lua";

  debugger.add_breakpoint(TEST_LUA_SCRIPT, 3);

  bool breaked = false;
  debugger.set_pause_handler([&](lrdb::debugger& debugger) {
    auto* breakpoint = debugger.current_breakpoint();
    ASSERT_TRUE(breakpoint);
    ASSERT_EQ(TEST_LUA_SCRIPT, breakpoint->file);
    ASSERT_EQ(3, breakpoint->line);
    ASSERT_EQ(1, breakpoint->break_count);
    ASSERT_TRUE(breakpoint->enabled);

    ASSERT_EQ(3, debugger.current_debug_info().currentline());

    auto callstack = debugger.get_call_stack();
    ASSERT_TRUE(callstack.size() > 0);

    breaked = true;
  });

  luaL_dofile(L, TEST_LUA_SCRIPT);

  ASSERT_TRUE(breaked);
}
TEST_F(DebuggerTest, BreakPointTestCoroutine) {
  const char* TEST_LUA_SCRIPT = "../test/lua/break_coroutine_test1.lua";

  debugger.add_breakpoint(TEST_LUA_SCRIPT, 3);

  bool breaked = false;
  debugger.set_pause_handler([&](lrdb::debugger& debugger) {
    auto* breakpoint = debugger.current_breakpoint();
    ASSERT_TRUE(breakpoint);
    ASSERT_EQ(TEST_LUA_SCRIPT, breakpoint->file);
    ASSERT_EQ(3, breakpoint->line);
    ASSERT_EQ(1, breakpoint->break_count);
    ASSERT_TRUE(breakpoint->enabled);

    ASSERT_EQ(3, debugger.current_debug_info().currentline());

    auto callstack = debugger.get_call_stack();
    ASSERT_TRUE(callstack.size() > 0);

    breaked = true;
  });

  luaL_dofile(L, TEST_LUA_SCRIPT);

  ASSERT_TRUE(breaked);
}
TEST_F(DebuggerTest, StepInTestCoroutine) {
  const char* TEST_LUA_SCRIPT = "../test/lua/break_coroutine_test1.lua";
  std::vector<int> break_line_numbers;
  debugger.step_in();
  debugger.set_pause_handler([&](lrdb::debugger& debugger) {
    break_line_numbers.push_back(debugger.current_debug_info().currentline());
    auto* breakpoint = debugger.current_breakpoint();
    ASSERT_TRUE(!breakpoint);
    debugger.step_in();
  });

  luaL_dofile(L, TEST_LUA_SCRIPT);
  std::vector<int> require_line_number = {1, 7, 2, 9, 3, 4, 5, 6, 7};
  ASSERT_EQ(require_line_number, break_line_numbers);
}
TEST_F(DebuggerTest, StepOverTest) {
  const char* TEST_LUA_SCRIPT = "../test/lua/step_over_test1.lua";
  debugger.step_in();

  std::vector<int> break_line_numbers;

  debugger.set_pause_handler([&](lrdb::debugger& debugger) {
    break_line_numbers.push_back(debugger.current_debug_info().currentline());
    auto* breakpoint = debugger.current_breakpoint();
    ASSERT_TRUE(!breakpoint);
    debugger.step_over();
  });

  luaL_dofile(L, TEST_LUA_SCRIPT);
  std::vector<int> require_line_number = {1, 7, 2, 9};
  ASSERT_EQ(require_line_number, break_line_numbers);
}
TEST_F(DebuggerTest, StepInTest) {
  const char* TEST_LUA_SCRIPT = "../test/lua/step_in_test1.lua";
  std::vector<int> break_line_numbers;
  debugger.step_in();
  debugger.set_pause_handler([&](lrdb::debugger& debugger) {
    break_line_numbers.push_back(debugger.current_debug_info().currentline());
    auto* breakpoint = debugger.current_breakpoint();
    ASSERT_TRUE(!breakpoint);
    debugger.step_in();
  });

  luaL_dofile(L, TEST_LUA_SCRIPT);
  std::vector<int> require_line_number = {1, 7, 2, 9, 3, 4, 5, 6, 7};
  ASSERT_EQ(require_line_number, break_line_numbers);
}
TEST_F(DebuggerTest, StepOutTest) {
  const char* TEST_LUA_SCRIPT = "../test/lua/step_out_test1.lua";
  debugger.add_breakpoint(TEST_LUA_SCRIPT, 3);

  std::vector<int> break_line_numbers;
  debugger.set_pause_handler([&](lrdb::debugger& debugger) {
    break_line_numbers.push_back(debugger.current_debug_info().currentline());
    debugger.step_out();
  });

  luaL_dofile(L, TEST_LUA_SCRIPT);
  std::vector<int> require_line_number = {3, 11};
  ASSERT_EQ(require_line_number, break_line_numbers);
}
TEST_F(DebuggerTest, TickTest1) {
  const char* TEST_LUA_SCRIPT = "../test/lua/test1.lua";

  int tick_count = 0;
  debugger.set_tick_handler([&](lrdb::debugger& debugger) { tick_count++; });

  luaL_dofile(L, TEST_LUA_SCRIPT);

  ASSERT_TRUE(tick_count > 0);
}

TEST_F(DebuggerTest, EvalTest1) {
  const char* TEST_LUA_SCRIPT = "../test/lua/eval_test1.lua";

  debugger.add_breakpoint(TEST_LUA_SCRIPT, 4);

  debugger.set_pause_handler([&](lrdb::debugger& debugger) {
    std::vector<picojson::value> ret = debugger.current_debug_info().eval(
        "return arg,value,local_value,local_value3");

    ASSERT_EQ(4, ret.size());

    ASSERT_TRUE(ret[0].is<double>());
    ASSERT_EQ(4, ret[0].get<double>());
    ASSERT_TRUE(ret[1].is<double>());
    ASSERT_EQ(1, ret[1].get<double>());
    ASSERT_TRUE(ret[2].is<double>());
    ASSERT_EQ(2, ret[2].get<double>());
    ASSERT_TRUE(ret[3].is<picojson::null>());
  });

  luaL_dofile(L, TEST_LUA_SCRIPT);
}

TEST_F(DebuggerTest, GetLocalTest1) {
  const char* TEST_LUA_SCRIPT = "../test/lua/get_local_var_test1.lua";

  debugger.add_breakpoint(TEST_LUA_SCRIPT, 5);

  debugger.set_pause_handler([&](lrdb::debugger& debugger) {

    lrdb::debug_info::local_vars_type localvars =
        debugger.current_debug_info().get_local_vars();
    ASSERT_EQ(2, localvars.size());
    ASSERT_EQ("local_value1", localvars[0].first);
    ASSERT_EQ(1, localvars[0].second.get<double>());

    bool ret = debugger.current_debug_info().set_local_var(
        "local_value1", picojson::value("ab"));

    ASSERT_TRUE(ret);
    localvars = debugger.current_debug_info().get_local_vars();

    ASSERT_EQ(2, localvars.size());
    ASSERT_EQ("local_value1", localvars[0].first);
    ASSERT_EQ("ab", localvars[0].second.get<std::string>());

    std::vector<picojson::value> eret =
        debugger.current_debug_info().eval("return local_value1");

    ASSERT_EQ(1, eret.size());
    ASSERT_TRUE(eret[0].is<std::string>());
    ASSERT_EQ("ab", eret[0].get<std::string>());

    eret = debugger.current_debug_info().eval("return _G");
    ASSERT_EQ(1, eret.size());
    ASSERT_TRUE(eret[0].is<picojson::object>());

  });

  luaL_dofile(L, TEST_LUA_SCRIPT);
}
TEST_F(DebuggerTest, GetVaArgTest1) {
  const char* TEST_LUA_SCRIPT = "../test/lua/vaarg_test1.lua";

  debugger.add_breakpoint(TEST_LUA_SCRIPT, 5);

  debugger.set_pause_handler([&](lrdb::debugger& debugger) {

    lrdb::debug_info::local_vars_type localvars =
        debugger.current_debug_info().get_local_vars(1);

    ASSERT_EQ(4, localvars.size());
    ASSERT_EQ("v1", localvars[0].first);
    ASSERT_EQ(2, localvars[0].second.get<double>());

    ASSERT_EQ("local_value1", localvars[1].first);
    ASSERT_EQ(1, localvars[1].second.get<double>());
    ASSERT_EQ("local_value2", localvars[2].first);
    ASSERT_EQ("abc", localvars[2].second.get<std::string>());
    ASSERT_EQ("(*vararg)", localvars[3].first);
    ASSERT_TRUE(localvars[3].second.is<picojson::array>());
    auto& vararg = localvars[3].second.get<picojson::array>();

    ASSERT_EQ(2, vararg.size());
    ASSERT_EQ(1, vararg[0].get<double>());
    ASSERT_EQ(3, vararg[1].get<double>());

    std::vector<picojson::value> eret =
        debugger.current_debug_info().eval("return table.unpack(_ENV[\"(*vararg)\"])");
    ASSERT_EQ(2, eret.size());
    ASSERT_EQ(1, eret[0].get<double>());
    ASSERT_EQ(3, eret[1].get<double>());

    bool vaarg = debugger.current_debug_info().is_variadic_arg();
    ASSERT_TRUE(vaarg);
  });

  luaL_dofile(L, TEST_LUA_SCRIPT);
}
int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}