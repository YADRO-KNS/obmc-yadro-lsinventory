// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "inventory.hpp"

#include <sdbusplus/test/sdbus_mock.hpp>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::StrEq;

/**
 * class InventoryTest
 * @brief Inventory tests.
 */
class InventoryTest : public ::testing::Test
{
  protected:
    InventoryTest() : bus(sdbusplus::get_mocked_new(&mock))
    {}

    testing::NiceMock<sdbusplus::SdBusMock> mock;
    sdbusplus::bus::bus bus;
};

TEST_F(InventoryTest, EmptyList)
{
    EXPECT_CALL(mock, sd_bus_message_at_end).WillRepeatedly(Return(1));
    std::vector<InventoryItem> inventory = getInventory(bus);
    EXPECT_TRUE(inventory.empty());
}

TEST_F(InventoryTest, FullList)
{
    // clang-format off
    // ordered list of inventory items
    const char* orderedNames[] = {
        "cpu0",
        "cpu0/core0",
        "cpu0/core1",
        "cpu0/core5",
        "cpu0/core10",
        "cpu5",
        "cpu10",
    };
    // unordered list of D-Bus objects
    const char* dbusPaths[] = {
        "/xyz/openbmc_project/inventory/system/chassis/cpu0/core1",
        "/xyz/openbmc_project/inventory/system/chassis/cpu10",
        "/xyz/openbmc_project/inventory/system/chassis/cpu0",
        "/xyz/openbmc_project/inventory/system/chassis/cpu0/core5",
        "/xyz/openbmc_project/inventory/system/chassis/cpu5",
        "/xyz/openbmc_project/inventory/system/chassis/cpu0/core10",
        "/xyz/openbmc_project/inventory/system/chassis/cpu0/core0",
    };
    // clang-format on

    const size_t itemsCount = sizeof(orderedNames) / sizeof(orderedNames[0]);
    static_assert(itemsCount == sizeof(dbusPaths) / sizeof(dbusPaths[0]));

    // returns "end-of-array" flag, callback for "GetSubTree",
    // we need only object's path, so set "end-of-array" for every second call,
    // because GetSubTree wants map<string, map<string, vector<string>>>
    size_t counter = 0;
    EXPECT_CALL(mock, sd_bus_message_at_end)
        .WillRepeatedly(Invoke([&](sd_bus_message*, int) {
            const size_t index = counter / 2;
            const bool hasData = counter % 2 == 0 && index < itemsCount;
            ++counter;
            return hasData ? 0 : 1;
        }));

    // returns D-Bus object path, callback for "GetSubTree"
    size_t index = 0;
    EXPECT_CALL(mock, sd_bus_message_read_basic(_, 's', _))
        .WillRepeatedly(Invoke([&](sd_bus_message*, char, void* p) {
            const char** s = static_cast<const char**>(p);
            *s = index < itemsCount ? dbusPaths[index] : "<ERR>";
            ++index;
            return 0;
        }));

    std::vector<InventoryItem> inventory = getInventory(bus);
    ASSERT_EQ(inventory.size(), itemsCount);

    for (size_t i = 0; i < itemsCount; ++i)
    {
        const InventoryItem& item = inventory[i];
        EXPECT_EQ(item.name, orderedNames[i]);
        EXPECT_TRUE(item.properties.empty());
        EXPECT_TRUE(item.prettyName().empty());
        EXPECT_TRUE(item.isPresent());
    }
}

TEST_F(InventoryTest, BooleanProperty)
{
    const char* key = "BooleanProperty";
    const bool val = true;

    EXPECT_CALL(mock, sd_bus_message_at_end)
        .WillOnce(Return(0))
        .WillOnce(Return(1))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_read_basic(_, 's', _))
        .WillOnce(Invoke([&](sd_bus_message*, char, void* p) {
            // callback for GetSubTree
            const char** s = static_cast<const char**>(p);
            *s = "dummy";
            return 0;
        }))
        .WillOnce(Invoke([&](sd_bus_message*, char, void* p) {
            // callback for GetAll (property name)
            const char** s = static_cast<const char**>(p);
            *s = key;
            return 0;
        }));

    EXPECT_CALL(mock, sd_bus_message_verify_type(_, 'v', StrEq("s")))
        .WillOnce(Return(0));
    EXPECT_CALL(mock, sd_bus_message_verify_type(_, 'v', StrEq("x")))
        .WillOnce(Return(0));
    EXPECT_CALL(mock, sd_bus_message_verify_type(_, 'v', StrEq("b")))
        .WillOnce(Return(1));
    EXPECT_CALL(mock, sd_bus_message_read_basic(_, 'b', _))
        .WillOnce(Invoke([&](sd_bus_message*, char, void* p) {
            *static_cast<char*>(p) = val;
            return 0;
        }));

    std::vector<InventoryItem> inventory = getInventory(bus);
    ASSERT_EQ(inventory.size(), 1);
    const InventoryItem& item = inventory[0];
    EXPECT_EQ(item.properties.size(), 1);
    ASSERT_NE(item.properties.find(key), item.properties.end());
    const auto& chkVal = item.properties.find(key)->second;
    ASSERT_TRUE(std::holds_alternative<bool>(chkVal));
    EXPECT_EQ(std::get<bool>(chkVal), val);
}

TEST_F(InventoryTest, NumericProperty)
{
    const char* key = "NumericProperty";
    const int64_t val = 42;

    EXPECT_CALL(mock, sd_bus_message_at_end)
        .WillOnce(Return(0))
        .WillOnce(Return(1))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_read_basic(_, 's', _))
        .WillOnce(Invoke([&](sd_bus_message*, char, void* p) {
            // callback for GetSubTree
            const char** s = static_cast<const char**>(p);
            *s = "dummy";
            return 0;
        }))
        .WillOnce(Invoke([&](sd_bus_message*, char, void* p) {
            // callback for GetAll (property name)
            const char** s = static_cast<const char**>(p);
            *s = key;
            return 0;
        }));

    EXPECT_CALL(mock, sd_bus_message_verify_type(_, 'v', StrEq("x")))
        .WillOnce(Return(1));
    EXPECT_CALL(mock, sd_bus_message_read_basic(_, 'x', _))
        .WillOnce(Invoke([&](sd_bus_message*, char, void* p) {
            *static_cast<int64_t*>(p) = val;
            return 0;
        }));

    std::vector<InventoryItem> inventory = getInventory(bus);
    ASSERT_EQ(inventory.size(), 1);
    const InventoryItem& item = inventory[0];
    EXPECT_EQ(item.properties.size(), 1);
    ASSERT_NE(item.properties.find(key), item.properties.end());
    const auto& chkVal = item.properties.find(key)->second;
    ASSERT_TRUE(std::holds_alternative<int64_t>(chkVal));
    EXPECT_EQ(std::get<int64_t>(chkVal), val);
}

TEST_F(InventoryTest, StringProperty)
{
    const char* key = "PrettyName";
    const char* val = "Object pretty name \t \r\n  ";
    const char* valResult = "Object pretty name";

    EXPECT_CALL(mock, sd_bus_message_at_end)
        .WillOnce(Return(0))
        .WillOnce(Return(1))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_verify_type(_, 'v', StrEq("x")))
        .WillOnce(Return(0));
    EXPECT_CALL(mock, sd_bus_message_verify_type(_, 'v', StrEq("s")))
        .WillOnce(Return(1));

    EXPECT_CALL(mock, sd_bus_message_read_basic(_, 's', _))
        .WillOnce(Invoke([&](sd_bus_message*, char, void* p) {
            // callback for GetSubTree
            const char** s = static_cast<const char**>(p);
            *s = "dummy";
            return 0;
        }))
        .WillOnce(Invoke([&](sd_bus_message*, char, void* p) {
            // callback for GetAll (property name)
            const char** s = static_cast<const char**>(p);
            *s = key;
            return 0;
        }))
        .WillOnce(Invoke([&](sd_bus_message*, char, void* p) {
            // callback for GetAll (property value)
            const char** s = static_cast<const char**>(p);
            *s = val;
            return 0;
        }));

    std::vector<InventoryItem> inventory = getInventory(bus);
    ASSERT_EQ(inventory.size(), 1);
    const InventoryItem& item = inventory[0];
    EXPECT_EQ(item.properties.size(), 1);
    EXPECT_EQ(item.prettyName(), valResult);
}
