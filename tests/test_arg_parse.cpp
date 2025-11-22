#include <filesystem>
#include <stdexcept>
#include "app.hpp"
#include "gtest/gtest.h"

// Test fixture to reset AppState before each test
class AppTest : public ::testing::Test
{
    protected:
    void SetUp() override
    {
        // Reset the global state before each test to ensure test isolation
        g_AppState = AppState{};
    }
};

// --- Test Cases for Valid Arguments ---

TEST_F(AppTest, ParseArgs_ValidFilename)
{
    const char *const argv[] = {"app", "test.txt"};
    ASSERT_NO_THROW(ParseArgs(2, argv));
    EXPECT_EQ(g_AppState.file, std::filesystem::path("test.txt"));
    EXPECT_FALSE(g_AppState.run_approx);
    EXPECT_FALSE(g_AppState.debug);
    EXPECT_FALSE(g_AppState.generate_graph);
    EXPECT_FALSE(g_AppState.run_internal_tests);
}

TEST_F(AppTest, ParseArgs_ApproxFlag)
{
    const char *const argv[] = {"app", "--approx", "test.txt"};
    ASSERT_NO_THROW(ParseArgs(3, argv));
    EXPECT_EQ(g_AppState.file, std::filesystem::path("test.txt"));
    EXPECT_TRUE(g_AppState.run_approx);
}

TEST_F(AppTest, ParseArgs_DebugFlag)
{
    const char *const argv[] = {"app", "--debug", "test.txt"};
    ASSERT_NO_THROW(ParseArgs(3, argv));
    EXPECT_EQ(g_AppState.file, std::filesystem::path("test.txt"));
    EXPECT_TRUE(g_AppState.debug);
}

TEST_F(AppTest, ParseArgs_InternalTestsFlag)
{
    const char *const argv[] = {"app", "--run_internal_tests"};
    ASSERT_NO_THROW(ParseArgs(2, argv));
    EXPECT_TRUE(g_AppState.run_internal_tests);
    EXPECT_TRUE(g_AppState.file.empty());  // No file is needed
}

TEST_F(AppTest, ParseArgs_CombinedFlags)
{
    const char *const argv[] = {"app", "--debug", "--approx", "test.txt"};
    ASSERT_NO_THROW(ParseArgs(4, argv));
    EXPECT_EQ(g_AppState.file, std::filesystem::path("test.txt"));
    EXPECT_TRUE(g_AppState.debug);
    EXPECT_TRUE(g_AppState.run_approx);
}

TEST_F(AppTest, ParseArgs_KOption)
{
    const char *const argv[] = {"app", "--k", "5", "test.txt"};
    ASSERT_NO_THROW(ParseArgs(4, argv));
    EXPECT_EQ(g_AppState.file, std::filesystem::path("test.txt"));
    EXPECT_EQ(g_AppState.num_results, 5);
}

TEST_F(AppTest, ParseArgs_KOption_DefaultValue)
{
    const char *const argv[] = {"app", "test.txt"};
    ASSERT_NO_THROW(ParseArgs(2, argv));
    EXPECT_EQ(g_AppState.file, std::filesystem::path("test.txt"));
    EXPECT_EQ(g_AppState.num_results, 1);  // Default value
}

TEST_F(AppTest, ParseArgs_GenFlag_ValidTrue)
{
    const char *const argv[] = {"app", "--gen", "10", "20", "0.5", "0.8", "true", "output.txt"};
    ASSERT_NO_THROW(ParseArgs(8, argv));

    EXPECT_TRUE(g_AppState.generate_graph);
    EXPECT_EQ(g_AppState.file, std::filesystem::path("output.txt"));
    EXPECT_EQ(g_AppState.spec.size_g1, 10);
    EXPECT_EQ(g_AppState.spec.size_g2, 20);
    EXPECT_DOUBLE_EQ(g_AppState.spec.density_g1, 0.5);
    EXPECT_DOUBLE_EQ(g_AppState.spec.density_g2, 0.8);
    EXPECT_TRUE(g_AppState.spec.create_g1_based_on_g2);
}

TEST_F(AppTest, ParseArgs_GenFlag_ValidFalse)
{
    const char *const argv[] = {"app", "--gen", "15", "25", "0.6", "0.9", "false", "output.txt"};
    ASSERT_NO_THROW(ParseArgs(8, argv));

    EXPECT_TRUE(g_AppState.generate_graph);
    EXPECT_EQ(g_AppState.file, std::filesystem::path("output.txt"));
    EXPECT_EQ(g_AppState.spec.size_g1, 15);
    EXPECT_EQ(g_AppState.spec.size_g2, 25);
    EXPECT_DOUBLE_EQ(g_AppState.spec.density_g1, 0.6);
    EXPECT_DOUBLE_EQ(g_AppState.spec.density_g2, 0.9);
    EXPECT_FALSE(g_AppState.spec.create_g1_based_on_g2);
}

// --- Test Cases for Error Conditions (throwing exceptions) ---

TEST_F(AppTest, ParseArgs_NoFileOrGenOrInternalTest_Throws)
{
    const char *const argv[] = {"app", "--debug"};
    EXPECT_THROW(
        {
            try {
                ParseArgs(2, argv);
            } catch (const std::runtime_error &e) {
                EXPECT_STREQ(e.what(), "A filename is required if --run_internal_tests is not used.");
                throw;
            }
        },
        std::runtime_error
    );
}

TEST_F(AppTest, ParseArgs_MultipleFiles_Throws)
{
    const char *const argv[] = {"app", "file1.txt", "file2.txt"};
    // CLI11 will throw a ParseError for too many positional arguments
    EXPECT_THROW(ParseArgs(3, argv), std::runtime_error);
}

TEST_F(AppTest, ParseArgs_UnknownOption_Throws)
{
    const char *const argv[] = {"app", "--unknown", "file.txt"};
    // CLI11 will throw a ParseError for unknown options
    EXPECT_THROW(ParseArgs(3, argv), std::runtime_error);
}

TEST_F(AppTest, ParseArgs_GenTooFewArgs_Throws)
{
    const char *const argv[] = {"app", "--gen", "1", "2", "0.1", "0.2", "file.txt"};
    // CLI11 will throw a ParseError for insufficient arguments to --gen
    EXPECT_THROW(ParseArgs(7, argv), std::runtime_error);
}

TEST_F(AppTest, ParseArgs_GenInvalidBool_Throws)
{
    const char *const argv[] = {"app", "--gen", "1", "2", "0.1", "0.2", "yes", "file.txt"};
    EXPECT_THROW(
        {
            try {
                ParseArgs(8, argv);
            } catch (const std::runtime_error &e) {
                EXPECT_STREQ(
                    e.what(),
                    "Error parsing --gen arguments: Invalid boolean value for 'base', must be 'true' or 'false'"
                );
                throw;
            }
        },
        std::runtime_error
    );
}
