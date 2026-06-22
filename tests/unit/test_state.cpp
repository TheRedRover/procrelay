#include <gtest/gtest.h>
#include <procrelay/process.hpp>

using namespace procrelay;

TEST(StateFromChar, KnownCodes)
{
    EXPECT_EQ(state_from_char('R'), ProcessState::RUNNING);
    EXPECT_EQ(state_from_char('S'), ProcessState::SLEEPING);
    EXPECT_EQ(state_from_char('D'), ProcessState::DISK_SLEEP);
    EXPECT_EQ(state_from_char('Z'), ProcessState::ZOMBIE);
    EXPECT_EQ(state_from_char('T'), ProcessState::STOPPED);
    EXPECT_EQ(state_from_char('t'), ProcessState::TRACING_STOP);
    EXPECT_EQ(state_from_char('X'), ProcessState::DEAD);
    EXPECT_EQ(state_from_char('x'), ProcessState::DEAD);
    EXPECT_EQ(state_from_char('I'), ProcessState::IDLE);
    EXPECT_EQ(state_from_char('P'), ProcessState::PARKED);
}

TEST(StateFromChar, UnknownCode)
{
    EXPECT_EQ(state_from_char('?'), ProcessState::UNKNOWN);
    EXPECT_EQ(state_from_char('Q'), ProcessState::UNKNOWN);
}

TEST(LabelFromState, AllStates)
{
    EXPECT_EQ(label_from_state(ProcessState::RUNNING), "running");
    EXPECT_EQ(label_from_state(ProcessState::SLEEPING), "sleeping");
    EXPECT_EQ(label_from_state(ProcessState::DISK_SLEEP), "disk-sleep");
    EXPECT_EQ(label_from_state(ProcessState::ZOMBIE), "zombie");
    EXPECT_EQ(label_from_state(ProcessState::STOPPED), "stopped");
    EXPECT_EQ(label_from_state(ProcessState::TRACING_STOP), "tracing-stop");
    EXPECT_EQ(label_from_state(ProcessState::DEAD), "dead");
    EXPECT_EQ(label_from_state(ProcessState::PARKED), "parked");
    EXPECT_EQ(label_from_state(ProcessState::IDLE), "idle");
    EXPECT_EQ(label_from_state(ProcessState::UNKNOWN), "unknown");
}

TEST(StateFromLabel, ValidLabels)
{
    EXPECT_EQ(state_from_label("running"), ProcessState::RUNNING);
    EXPECT_EQ(state_from_label("sleeping"), ProcessState::SLEEPING);
    EXPECT_EQ(state_from_label("disk-sleep"), ProcessState::DISK_SLEEP);
    EXPECT_EQ(state_from_label("zombie"), ProcessState::ZOMBIE);
    EXPECT_EQ(state_from_label("stopped"), ProcessState::STOPPED);
    EXPECT_EQ(state_from_label("tracing-stop"), ProcessState::TRACING_STOP);
    EXPECT_EQ(state_from_label("dead"), ProcessState::DEAD);
    EXPECT_EQ(state_from_label("parked"), ProcessState::PARKED);
    EXPECT_EQ(state_from_label("idle"), ProcessState::IDLE);
    EXPECT_EQ(state_from_label("unknown"), ProcessState::UNKNOWN);
}

TEST(StateFromLabel, CaseInsensitive)
{
    EXPECT_EQ(state_from_label("RUNNING"), ProcessState::RUNNING);
    EXPECT_EQ(state_from_label("Running"), ProcessState::RUNNING);
    EXPECT_EQ(state_from_label("ZOMBIE"), ProcessState::ZOMBIE);
}

TEST(StateFromLabel, UnknownLabel)
{
    EXPECT_FALSE(state_from_label("foo").has_value());
    EXPECT_FALSE(state_from_label("unknown").has_value());
    EXPECT_FALSE(state_from_label("").has_value());
    EXPECT_FALSE(state_from_label(",").has_value());
    EXPECT_FALSE(state_from_label(" ").has_value());
}
