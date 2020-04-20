// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

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
    using propval_t = std::variant<int64_t, std::string, bool>;

    /** @brief Name of the item. */
    std::string name;

    /** @brief Item's properties. */
    std::map<std::string, propval_t> properties;

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
};

/**
 * @brief Get all inventory items.
 *
 * @return array with inventory items
 */
std::vector<InventoryItem> getInventory();
