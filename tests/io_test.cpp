#include "io.hpp"
#include <cstdio>
#include <fstream>
#include "graph.hpp"
#include "gtest/gtest.h"

void AssertGraphsEqual(const Graph &g1, const Graph &g2)
{
    ASSERT_EQ(g1.GetVertices(), g2.GetVertices());
    ASSERT_EQ(g1.GetEdges(), g2.GetEdges());
    for (std::uint32_t i = 0; i < g1.GetVertices(); ++i) {
        for (std::uint32_t j = 0; j < g1.GetVertices(); ++j) {
            EXPECT_EQ(g1.GetEdges(i, j), g2.GetEdges(i, j));
        }
    }
}

class IoTest : public ::testing::Test
{
    protected:
    void SetUp() override
    {
        g1_ = std::make_unique<Graph>(3);
        g1_->AddEdges(0, 1, 1);
        g1_->AddEdges(1, 2, 2);
        g1_->AddEdges(2, 0, 3);

        g2_ = std::make_unique<Graph>(4);
        g2_->AddEdges(0, 0, 1);
        g2_->AddEdges(1, 1, 2);
        g2_->AddEdges(2, 2, 3);
        g2_->AddEdges(3, 3, 4);
        g2_->AddEdges(0, 3, 5);

        test_filename_ = "test_io_graphs.txt";
    }

    void TearDown() override { std::remove(test_filename_.c_str()); }

    std::unique_ptr<Graph> g1_;
    std::unique_ptr<Graph> g2_;
    std::string test_filename_;
};

TEST_F(IoTest, WriteAndRead_RoundTrip)
{
    ASSERT_NO_THROW(Write(test_filename_.c_str(), std::make_tuple(std::ref(*g1_), std::ref(*g2_))));

    Graph g1_read(0), g2_read(0);
    ASSERT_NO_THROW(std::tie(g1_read, g2_read) = Read(test_filename_.c_str()));

    AssertGraphsEqual(*g1_, g1_read);
    AssertGraphsEqual(*g2_, g2_read);
}

TEST_F(IoTest, Read_ThrowsOnNonExistentFile) { EXPECT_THROW(Read("non_existent_file.txt"), std::runtime_error); }

TEST_F(IoTest, Read_ThrowsOnMalformedFile_IncompleteData)
{
    std::ofstream malformed_file(test_filename_);
    malformed_file << "3\n";
    malformed_file << "0 1 0\n";
    malformed_file.close();

    EXPECT_THROW(Read(test_filename_.c_str()), std::runtime_error);
}

TEST_F(IoTest, Read_ThrowsOnMalformedFile_InvalidSize)
{
    std::ofstream malformed_file(test_filename_);
    malformed_file << "0\n";
    malformed_file.close();

    EXPECT_THROW(Read(test_filename_.c_str()), std::runtime_error);
}
