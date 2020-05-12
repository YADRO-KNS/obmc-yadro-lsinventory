// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

#include "inventory.hpp"

/**
 * @class Printer
 * @brief Inventory item printer.
 */
class Printer
{
  public:
    /**
     * @brief Set output filter by item name.
     *
     * @param[in] name - name of the item to filter
     */
    void setNameFilter(const char* name);

    /**
     * @brief Allow printing nonexistent items.
     */
    void allowNonexitent();

    /**
     * @brief Allow printing empty properties.
     */
    void allowEmptyProperties();

    /**
     * @brief Print list of inventory items as formatted text.
     *
     * @param[in] items - array of items to print
     */
    void printText(const std::vector<InventoryItem>& items) const;

    /**
     * @brief Print list of inventory items as JSON text.
     *
     * @param[in] items - array of items to print
     */
    void printJson(const std::vector<InventoryItem>& items) const;

  private:
    /** @brief Filter for item name. */
    std::string nameFilter;
    /** @brief Allow printing of nonexistent items. */
    bool printNonexitent = false;
    /** @brief Allow printing of empty properties. */
    bool printEmptyProperties = false;
};
