#include "test_framework.hpp"

extern int LibMain()
{
    // TestApproxOnPrecise(ApproxAlgo::kMcts, PreciseAlgo::kBruteForce);
    // TestApproxOnApprox(ApproxAlgo::kMcts, ApproxAlgo::kMcts);
    TestPreciseOnPrecise(PreciseAlgo::kAStart, PreciseAlgo::kBruteForce);
    return 0;
}
