#include "test_framework.hpp"

extern int LibMain()
{
    TestCorrectnessApproxOnPrecise(ApproxAlgo::kMcts, PreciseAlgo::kBruteForce);
    return 0;
}
