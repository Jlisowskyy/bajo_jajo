#include "app.hpp"

#include "algos.hpp"
#include "curated_gen.hpp"
#include "io.hpp"
#include "random_gen.hpp"
#include "test_framework.hpp"
#include "trace.hpp"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

// ------------------------------
// State
// ------------------------------

AppState g_AppState{};

// ------------------------------
// Statics
// ------------------------------

static void Help_()
{
    std::cout << "Usage: app <input> <output> [options]\n"
              << "\nOptions:\n"
              << "  --help                 Display this help message and exit.\n"
              << "  --approx               Run the approximate algorithm instead of the precise algorithm.\n"
              << "  --bruteforce           Run the bruteforce accurate algorithm.\n"
              << "  --gen-suite            Generate a curated suite of benchmark graph pairs to 'tests/' directory.\n"
              << "\nArguments:\n"
              << "  input                  Path to the input file with the graphs.\n"
              << "  output                 Path to the output file where the extension of the G2 graph will be saved.\n"
              << std::endl;
}

static void PrintAppState()
{
    const auto &spec = g_AppState.spec;
    TRACE(
        "\n--- Application State ---\n", "Mode:\n", "  - Debug traces:    ", (g_AppState.debug ? "yes" : "no"), "\n",
        "  - Internal tests:    ", (g_AppState.run_internal_tests ? "yes" : "no"), "\n",
        "  - Generate Suite:    ", (g_AppState.generate_suite ? "yes" : "no"), "\n",
        "  - Algorithm:       ", (g_AppState.run_approx ? "Approximate " : "Precise "), "\n",
        "  - K:    ", g_AppState.num_results, "\n",

        (g_AppState.generate_graph ? "Input Source:        Generate Graph" : "Input Source:        File"), "\n",

        (g_AppState.generate_graph ? "  - G1 Size:         " : "  - Filename:        "),
        (g_AppState.generate_graph ? std::to_string(spec.size_g1) : (g_AppState.file ? g_AppState.file : "N/A")), "\n",

        (g_AppState.generate_graph ? "  - G2 Size:         " : ""),
        (g_AppState.generate_graph ? std::to_string(spec.size_g2) : ""), (g_AppState.generate_graph ? "\n" : ""),

        (g_AppState.generate_graph ? "  - G1 Density:      " : ""),
        (g_AppState.generate_graph ? std::to_string(spec.density_g1) : ""), (g_AppState.generate_graph ? "\n" : ""),

        (g_AppState.generate_graph ? "  - G2 Density:      " : ""),
        (g_AppState.generate_graph ? std::to_string(spec.density_g2) : ""), (g_AppState.generate_graph ? "\n" : ""),

        (g_AppState.generate_graph ? "  - G1 based on G2:  " : ""),
        (g_AppState.generate_graph ? (spec.create_g1_based_on_g2 ? "yes" : "no") : ""),
        (g_AppState.generate_graph ? "\n" : ""),

        "-------------------------"
    );
}

// ------------------------------
// Imlemenatations
// ------------------------------

void OnFail() { Help_(); }

bool IsDebug() { return g_AppState.debug; }

void ParseArgs(int argc, const char *const argv[])
{
    std::vector<std::string_view> args(argv + 1, argv + argc);

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg = args[i];

        if (arg == "--help") {
            Help_();
            std::exit(0);
        }

        if (arg == "--approx") {
            g_AppState.run_approx = true;
        } else if (arg == "--bruteforce") {
            g_AppState.run_bruteforce = true;
        } else if (arg == "--debug") {
            g_AppState.debug = true;
        } else if (arg == "--run_internal_tests") {
            g_AppState.run_internal_tests = true;
        } else if (arg == "--gen-suite") {
            g_AppState.generate_suite = true;
        } else if (arg == "--gen") {
            if (i + 5 >= args.size()) {
                throw std::runtime_error("--gen requires 5 arguments.");
            }
            g_AppState.generate_graph = true;
            try {
                g_AppState.spec.size_g1    = std::stoul(std::string(args[i + 1]));
                g_AppState.spec.size_g2    = std::stoul(std::string(args[i + 2]));
                g_AppState.spec.density_g1 = std::stod(std::string(args[i + 3]));
                g_AppState.spec.density_g2 = std::stod(std::string(args[i + 4]));
                std::string base_str(args[i + 5]);
                if (base_str == "true") {
                    g_AppState.spec.create_g1_based_on_g2 = true;
                } else if (base_str == "false") {
                    g_AppState.spec.create_g1_based_on_g2 = false;
                } else {
                    throw std::invalid_argument("Invalid boolean value for 'base', must be 'true' or 'false'");
                }
            } catch (const std::exception &e) {
                throw std::runtime_error("Error parsing --gen arguments: " + std::string(e.what()));
            }
            i += 5;
        } else if (arg.rfind("--", 0) == 0) {
            throw std::runtime_error("Unknown option " + std::string(arg));
        } else {
            if (g_AppState.file != nullptr || g_AppState.output != nullptr) {
                throw std::runtime_error("Only input and output filepaths should be provided.");
            }
            g_AppState.file   = argv[i + 1];
            g_AppState.output = argv[i + 2];
            ++i;
        }
    }

    // Logic validation
    const bool is_special_mode =
        g_AppState.run_internal_tests || g_AppState.generate_graph || g_AppState.generate_suite;

    if (!is_special_mode && g_AppState.file == nullptr) {
        throw std::runtime_error(
            "A filename is required if not running in a special mode (--gen-suite, --run_internal_tests, --gen)."
        );
    }

    if (!is_special_mode && g_AppState.output == nullptr) {
        throw std::runtime_error(
            "Output filename is required if not running in a special mode (--gen-suite, --run_internal_tests, --gen)."
        );
    }
}

void Run()
{
    PrintAppState();

    if (g_AppState.run_internal_tests) {
        TRACE("Running internal tests...");
        TestApproxOnApprox(ApproxAlgo::kApproxAStar, ApproxAlgo::kApproxAStar5);
        return;
    }

    if (g_AppState.generate_suite) {
        TRACE("Generating curated test suite...");
        const auto suite = GenerateAllCurated();
        std::cout << "Generating " << suite.size() << " curated test cases...\n";

        for (const auto &test_case : suite) {
            std::string filename = test_case.name + ".txt";
            std::cout << "  - Writing: " << filename << std::endl;
            auto [g1, g2] = test_case.generator();
            Write(filename.c_str(), std::make_tuple(std::ref(g1), std::ref(g2)));
        }
        std::cout << "Done.\n";
        return;
    }

    if (g_AppState.generate_graph) {
        TRACE("Generating random graph...");
        const auto graphs = GenerateExample(g_AppState.spec);
        Write(g_AppState.file, graphs);
        return;
    }

    TRACE("Running base application flow...");
    auto [g1, g2] = Read(g_AppState.file);
    TRACE("Got g1 with size: ", g1.GetVertices(), " and g2 with size: ", g2.GetVertices());

    const auto t0                 = std::chrono::high_resolution_clock::now();
    std::vector<Mapping> mappings = {};
    if (g_AppState.run_approx) {
        mappings = Approximate(g1, g2, g_AppState.num_results);
    } else if (g_AppState.run_bruteforce) {
        mappings = AccurateBruteForce(g1, g2, g_AppState.num_results);
    } else {
        mappings = Accurate(g1, g2, g_AppState.num_results);
    }
    const auto t1                  = std::chrono::high_resolution_clock::now();
    const std::uint64_t time_spent = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

    Write(g1, g2, mappings, time_spent);
    if (!mappings.empty()) {
        WriteResult(g_AppState.output, g1, g2, mappings[0], time_spent);
    } else {
        Mapping empty_map(g1.GetVertices(), g2.GetVertices());
        WriteResult(g_AppState.output, g1, g2, empty_map, time_spent);
    }
}
