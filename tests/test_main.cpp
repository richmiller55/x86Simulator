#include "gtest/gtest.h"
#include "ncurses_environment.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new NCursesEnvironment);
  return RUN_ALL_TESTS();
}