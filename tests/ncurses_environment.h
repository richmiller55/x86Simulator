#ifndef NCURSES_ENVIRONMENT_H
#define NCURSES_ENVIRONMENT_H

#include "gtest/gtest.h"
#include "../arm_ui_manager.h"
#include <ncurses.h>

class NCursesEnvironment : public ::testing::Environment {
public:
    ~NCursesEnvironment() override {}

    void TearDown() override {
        if (ncurses_initialized) {
            endwin();
        }
    }
};

#endif // NCURSES_ENVIRONMENT_H
