#ifndef APP_HPP
#define APP_HPP

#include "random_gen.hpp"

void ParseArgs(int argc, const char *const argv[]);
void Run();
void OnFail();
bool IsDebug();

struct AppState {
    const char *file{};
    bool run_approx{};
    bool debug{};
    bool generate_graph{};
    bool run_internal_tests{};
    GraphSpec spec{};
};

extern AppState g_AppState;

#endif  // APP_HPP
