#include "test_framework.hpp"

extern int LibMain()
{
    TestApproxOnPrecise(ApproxAlgo::kMcts, PreciseAlgo::kBruteForce);
    TestApproxOnApprox(ApproxAlgo::kMcts, ApproxAlgo::kMcts);
    return 0;
}
