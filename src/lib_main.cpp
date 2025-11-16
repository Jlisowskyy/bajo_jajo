#include <iostream>

#include <bits/ostream.tcc>

#include "app.hpp"

extern int LibMain(const int argc, const char *const argv[])
{
    try {
        ParseArgs(argc, argv);
        Run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        OnFail();
        return 1;
    }

    return 0;
}
