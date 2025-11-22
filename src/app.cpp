#include "app.hpp"

#include <CLI/CLI.hpp>
#include <algos.hpp>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <random_gen.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "io.hpp"
#include "test_framework.hpp"
#include "trace.hpp"

// ------------------------------
// State
// ------------------------------

AppState g_AppState{};

// ------------------------------
// Statics
// ------------------------------

static void PrintAppState()
{
    const auto &spec = g_AppState.spec;
    TRACE(
        "\n--- Application State ---\n", "Mode:\n", "  - Debug traces:    ", (g_AppState.debug ? "yes" : "no"), "\n",
        "  - Internal tests:    ", (g_AppState.run_internal_tests ? "yes" : "no"), "\n",
        "  - Algorithm:       ", (g_AppState.run_approx ? "Approximate " : "Precise "), "\n",
        "  - K:    ", g_AppState.num_results, "\n",

        (g_AppState.generate_graph ? "Input Source:        Generate Graph" : "Input Source:        File"), "\n",

        (g_AppState.generate_graph ? "  - G1 Size:         " : "  - Filename:        "),
        (g_AppState.generate_graph ? std::to_string(spec.size_g1)
                                   : (g_AppState.file.empty() ? "N/A" : g_AppState.file.string())),
        "\n",

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

void OnFail() { /* CLI11 handles help automatically */ }

bool IsDebug() { return g_AppState.debug; }

void ParseArgs(int argc, const char *const argv[])
{
    CLI::App app{"Graph Application", "app"};

    // Boolean flags
    app.add_flag("--approx", g_AppState.run_approx, "Run the approximate algorithm instead of the precise algorithm");
    app.add_flag("--debug", g_AppState.debug, "Run debug traces");
    app.add_flag("--run_internal_tests", g_AppState.run_internal_tests, "Run internal tests");

    // --k option with default value
    app.add_option("--k", g_AppState.num_results, "Return num_results result mappings")->default_val(1);

    // Filename as optional positional argument
    std::filesystem::path filename;
    app.add_option("filename", filename, "Path to the input file describing the graphs or output")->expected(0, 1);

    // --gen option with 5 expected arguments (allow 5+ to handle filename after)
    std::vector<std::string> gen_args;
    app.add_option("--gen", gen_args, "Generate random graphs instead of reading from a file")
        ->expected(5, -1)  // At least 5, no maximum
        ->description(
            "s1: size of graph 1 (integer)\n"
            "s2: size of graph 2 (integer)\n"
            "d1: density of graph 1 (float)\n"
            "d2: density of graph 2 (float)\n"
            "base: create g1 based on g2 (true/false)"
        );

    // Parse arguments
    try {
        app.parse(argc, argv);
    } catch (const CLI::CallForHelp &e) {
        std::cout << app.help() << std::endl;
        std::exit(0);
    } catch (const CLI::CallForVersion &e) {
        std::exit(0);
    } catch (const CLI::ParseError &e) {
        throw std::runtime_error(e.what());
    }

    // Parse --gen arguments if provided
    if (!gen_args.empty()) {
        if (gen_args.size() < 5) {
            throw std::runtime_error("--gen requires exactly 5 arguments.");
        }

        // Extract filename from 6th argument if present and not already set
        std::filesystem::path gen_filename;
        if (gen_args.size() > 5 && filename.empty()) {
            gen_filename = gen_args[5];
            gen_args.resize(5);  // Keep only the 5 gen arguments
        }

        g_AppState.generate_graph = true;
        try {
            g_AppState.spec.size_g1     = std::stoul(gen_args[0]);
            g_AppState.spec.size_g2     = std::stoul(gen_args[1]);
            g_AppState.spec.density_g1  = std::stod(gen_args[2]);
            g_AppState.spec.density_g2  = std::stod(gen_args[3]);
            const std::string &base_str = gen_args[4];
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

        // Set filename from gen if it was extracted
        if (!gen_filename.empty()) {
            filename = gen_filename;
        }
    }

    // Copy filename to AppState if provided
    if (!filename.empty()) {
        g_AppState.file = filename;
    }

    // Validation: require filename unless --run_internal_tests is set
    if (!g_AppState.run_internal_tests && g_AppState.file.empty()) {
        throw std::runtime_error("A filename is required if --run_internal_tests is not used.");
    }
}

void Run()
{
    PrintAppState();

    if (g_AppState.run_internal_tests) {
        TRACE("Running internal tests...");
        TestApproxOnPrecise(ApproxAlgo::kApproxAStar, PreciseAlgo::kAStar);
        TestApproxOnApprox(ApproxAlgo::kApproxAStar, ApproxAlgo::kApproxAStar5);
        TestApproxOnPrecise(ApproxAlgo::kApproxAStar, PreciseAlgo::kBruteForce);
        TestPreciseOnPrecise(PreciseAlgo::kBruteForce, PreciseAlgo::kBruteForce);
        TestPreciseOnPrecise(PreciseAlgo::kBruteForce, PreciseAlgo::kAStar);
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

    const auto t0 = std::chrono::high_resolution_clock::now();
    const auto mappings =
        g_AppState.run_approx ? Approximate(g1, g2, g_AppState.num_results) : Accurate(g1, g2, g_AppState.num_results);
    const auto t1 = std::chrono::high_resolution_clock::now();

    const std::uint64_t time_spent = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    Write(g1, g2, mappings, time_spent);
}
