#include <gtest/gtest.h>
#include <procrelay/filters.hpp>

using namespace procrelay;

TEST(ParseFilters, NoParams)
{
    auto result = parse_filters({});
    ASSERT_TRUE(std::holds_alternative<FilterChain>(result));
    EXPECT_TRUE(std::get<FilterChain>(result).empty());
}

TEST(ParseFilters, ValidState)
{
    auto result = parse_filters({.state = "running"});
    ASSERT_TRUE(std::holds_alternative<FilterChain>(result));
    auto &chain = std::get<FilterChain>(result);
    ASSERT_EQ(chain.size(), 1u);
    auto *sf = std::get_if<StateFilter>(&chain[0]);
    ASSERT_NE(sf, nullptr);
    EXPECT_EQ(sf->m_state, ProcessState::RUNNING);
}

TEST(ParseFilters, InvalidState)
{
    auto result = parse_filters({.state = "foo"});
    ASSERT_TRUE(std::holds_alternative<FilterError>(result));
    EXPECT_EQ(std::get<FilterError>(result).m_code, "invalid_state");
}

TEST(ParseFilters, ValidTimeRange)
{
    auto result = parse_filters({.started_after = "1000", .started_before = "2000"});
    ASSERT_TRUE(std::holds_alternative<FilterChain>(result));
    auto &chain = std::get<FilterChain>(result);
    ASSERT_EQ(chain.size(), 1u);
    auto *tf = std::get_if<TimeRangeFilter>(&chain[0]);
    ASSERT_NE(tf, nullptr);
    ASSERT_TRUE(tf->m_after.has_value());
    ASSERT_TRUE(tf->m_before.has_value());
    EXPECT_EQ(*tf->m_after, 1000);
    EXPECT_EQ(*tf->m_before, 2000);
}

TEST(ParseFilters, MalformedEpoch)
{
    auto result = parse_filters({.started_after = "abc"});
    ASSERT_TRUE(std::holds_alternative<FilterError>(result));
    EXPECT_EQ(std::get<FilterError>(result).m_code, "invalid_time");
}

TEST(ParseFilters, ContradictoryRange)
{
    auto result = parse_filters({.started_after = "2000", .started_before = "1000"});
    ASSERT_TRUE(std::holds_alternative<FilterError>(result));
    EXPECT_EQ(std::get<FilterError>(result).m_code, "invalid_range");
}

TEST(ParseFilters, EqualBoundsAllowed)
{
    auto result = parse_filters({.started_after = "1000", .started_before = "1000"});
    EXPECT_TRUE(std::holds_alternative<FilterChain>(result));
}
