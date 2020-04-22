// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "inventory.hpp"

#include "config.hpp"

#include <sdbusplus/bus.hpp>

/**
 * @brief Construct item name from its path.
 *
 * @param[in] path - D-Bus path of the item
 *
 * @return inventory item name
 */
static std::string nameFromPath(const std::string& path)
{
    size_t lastSlash = path.find_last_of('/');

    // some objects (such as CPU's cores) have non-unique name,
    // for these items we should include parent node as prefix
    if (lastSlash && lastSlash != std::string::npos)
    {
        // check for core item
        static const std::string coreName = "core";
        if (path.compare(lastSlash + 1, coreName.length(), coreName) == 0)
        {
            // include CPU name (parent node)
            lastSlash = path.find_last_of('/', lastSlash - 1);
        }
    }

    if (lastSlash == std::string::npos)
        return path;
    else
        return path.substr(lastSlash + 1);
}

/**
 * @brief Comparator for human sorting of inventory items.
 *
 * Comparsion based on digits inside the name:
 * cpu1/core2 is less than cpu1/core10, but grater than cpu0/core10.
 *
 * @param[in] a - first item to compare
 * @param[in] b - second item to compare
 *
 * @return comparsion result, true if a < b
 */
static bool humanCompare(const InventoryItem& a, const InventoryItem& b)
{
    static const char* digits = "0123456789";

    size_t lastNonDigit = 0;
    while (lastNonDigit < a.name.length())
    {
        // searching for start of digits inside the name
        const size_t start = a.name.find_first_of(digits, lastNonDigit);
        if (start == std::string::npos ||
            start != b.name.find_first_of(digits, lastNonDigit))
        {
            break;
        }

        // check for equal of names before digits
        if (a.name.compare(0, start - 1, b.name, 0, start - 1) != 0)
        {
            break;
        }

        // searching for end of digits in this sequence
        const size_t end = a.name.find_first_not_of(digits, start);
        if (end != b.name.find_first_not_of(digits, start))
        {
            break;
        }

        // get and compare numbers
        const unsigned long valueA =
            std::stoul(a.name.substr(start, end - start));
        const unsigned long valueB =
            std::stoul(b.name.substr(start, end - start));
        if (valueA < valueB)
            return true;
        if (valueA > valueB)
            return false;

        lastNonDigit = end + 1;
    }

    // use default string comparer
    return a.name < b.name;
}

bool InventoryItem::isPresent() const
{
    const auto& it = properties.find("Present");
    return it == properties.end() || std::get<bool>(it->second);
}

std::string InventoryItem::prettyName() const
{
    const auto& it = properties.find("PrettyName");
    if (it != properties.end())
        return std::get<std::string>(it->second);
    return std::string();
}

std::vector<InventoryItem> getInventory()
{
    std::vector<InventoryItem> items;

    sdbusplus::bus::bus bus = sdbusplus::bus::new_default();

    // get all inventory items
    auto subTree = bus.new_method_call(MAPPER_SERVICE, MAPPER_PATH,
                                       MAPPER_IFACE, "GetSubTree");
    const std::vector<std::string> ifaces = {INVENTORY_IFACE};
    subTree.append(INVENTORY_PATH, 0, ifaces);
    std::map<std::string, std::map<std::string, std::vector<std::string>>>
        subTreeObjects;
    bus.call(subTree).read(subTreeObjects);

    for (const auto& obj : subTreeObjects)
    {
        InventoryItem item;
        item.name = nameFromPath(obj.first);

        // get all item's properties
        auto getProps =
            bus.new_method_call(INVENTORY_SERVICE, obj.first.c_str(),
                                "org.freedesktop.DBus.Properties", "GetAll");
        getProps.append("");
        bus.call(getProps).read(item.properties);

        // some properties have spaces at the end, strip them
        for (auto& property : item.properties)
        {
            if (std::holds_alternative<std::string>(property.second))
            {
                std::string& v = std::get<std::string>(property.second);
                v.erase(std::find_if(v.rbegin(), v.rend(),
                                     [](int ch) { return !std::isspace(ch); })
                            .base(),
                        v.end());
            }
        }

        items.emplace_back(item);
    }

    // sort in human readable order
    std::sort(items.begin(), items.end(), humanCompare);

    return items;
}
