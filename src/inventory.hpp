// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

#include <sdbusplus/bus.hpp>

#include <map>
#include <string>
#include <variant>
#include <vector>

/**
 * struct InventoryItem
 * @brief Inventory item.
 */
struct InventoryItem
{
    using PropName = std::string;
    using PropValue = std::variant<int64_t, uint64_t, uint32_t, uint16_t,
                                   uint8_t, std::string, bool>;
    using Properties = std::map<PropName, PropValue>;

    /** @brief Name of the item. */
    std::string name;

    /** @brief Item's properties. */
    Properties properties;

    /**
     * @brief Get item present flag.
     *
     * @return true if item is present
     */
    bool isPresent() const;

    /**
     * @brief Get pretty name of the item.
     *
     * @return pretty name, can be empty
     */
    std::string prettyName() const;

    /**
     * @brief Merge properties into the internal container.
     *
     * @param props - Properties map.
     */
    void merge(Properties& props);
};

/**
 * @brief Get all inventory items.
 *
 * @param[in] bus D-Bus instance to read inventory
 *
 * @return array with inventory items
 */
std::vector<InventoryItem> getInventory(sdbusplus::bus::bus& bus);
