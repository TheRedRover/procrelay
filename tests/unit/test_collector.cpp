#include <algorithm>
#include <filesystem>
#include <gtest/gtest.h>
#include <procrelay/collector.hpp>

using namespace procrelay;
namespace fs = std::filesystem;

static const fs::path PROC = fs::path(FIXTURE_DIR) / "proc";

TEST(Collector, GetNormalProcess)
{
    auto result = get_process(1, PROC);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->get_pid(), 1);
    EXPECT_EQ(result->get_ppid(), 0);
    EXPECT_EQ(result->get_comm(), "systemd");
    EXPECT_EQ(result->get_cmdline(), "/sbin/init");
    EXPECT_EQ(result->get_state_code(), 'S');
    EXPECT_EQ(result->get_state(), ProcessState::SLEEPING);
    EXPECT_GT(result->get_start_time(), 0);
}

TEST(Collector, GetMissingProcess)
{
    auto result = get_process(99999, PROC);
    EXPECT_FALSE(result.has_value());
}

TEST(Collector, KernelThreadCmdline)
{
    auto result = get_process(2, PROC);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->get_cmdline(), "[kthreadd]");
}

TEST(Collector, CommWithParens)
{
    auto result = get_process(3, PROC);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->get_comm(), "my(proc)name");
}

TEST(Collector, ScanIncludesPid1)
{
    auto procs = scan(PROC);
    EXPECT_GE(procs.size(), 1u);
    auto it = std::find_if(procs.begin(), procs.end(),
                           [](const ProcessInfo &p) { return p.get_pid() == 1; });
    EXPECT_NE(it, procs.end());
}
