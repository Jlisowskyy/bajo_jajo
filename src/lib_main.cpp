#include "test_framework.hpp"

extern int LibMain()
{
    TestCorrectnessApproxOnPrecise(ApproxAlgo::kMcts, PreciseAlgo::kBruteForce);
    TestApproxOnApproxTime(ApproxAlgo::kMcts, ApproxAlgo::kMcts);
    return 0;
}
