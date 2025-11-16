// #ifndef TEST_FRAMEWORK_HPP
// #define TEST_FRAMEWORK_HPP
//
// #include "algos.hpp"
//
// #include <array>
// #include <tuple>
//
// enum class ApproxAlgo { kHungarian = 0, kMcts, kLast };
//
// enum class PreciseAlgo {
//     kBruteForce = 0,
//     kAStart,
//     kLast,
// };
//
// using SigT = Mapping (*)(const Graph &, const Graph &);
//
// static constexpr std::array kPreciseAlgos{
//     std::make_tuple(AccurateBruteForce, "Brute force Algorithm"),
//     std::make_tuple(AccurateAStar, "A Start algorithm"),
// };
//
// static constexpr std::array kApproxAlgos{
//     std::make_tuple(HungarianLike, "Hungarian like"),
//     std::make_tuple(Mcts, "Mcts algorithm"),
// };
//
// void TestApproxOnPrecise(ApproxAlgo approx_algo, PreciseAlgo precise_algo);
// void TestPreciseOnPrecise(PreciseAlgo precise_algo, PreciseAlgo precise_algo1);
// void TestApproxOnApprox(ApproxAlgo approx_algo, ApproxAlgo approx_algo1);
// void TestPreciseOnPrecise(PreciseAlgo precise_algo, PreciseAlgo precise_algo1);
//
// #endif  // TEST_FRAMEWORK_HPP
