// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "inventory.hpp"

#include "config.hpp"

#include <unordered_set>

/**
 * @brief Construct item name from its path.
 *
 * @param[in] path D-Bus path of the item
 *
 * @return inventory item name
 */
static std::string nameFromPath(const std::string& path)
{
    size_t lastSlash = path.rfind('/');

    // some objects (such as CPU's cores) have non-unique name,
    // for these items we should include parent node as prefix
    if (lastSlash && lastSlash != std::string::npos)
    {
        // check for core item
        static const std::string coreName = "core";
        if (path.find(coreName, lastSlash + 1) != std::string::npos)
        {
            // include CPU name (parent node)
            lastSlash = path.rfind('/', lastSlash - 1);
        }
    }

    if (lastSlash == std::string::npos)
    {
        return path;
    }
    else
    {
        return path.substr(lastSlash + 1);
    }
}

/**
 * @brief Comparator for human sorting of inventory items.
 *
 * Comparsion based on digits inside the name:
 * cpu1/core2 is less than cpu1/core10, but grater than cpu0/core10.
 *
 * @param[in] a first item to compare
 * @param[in] b second item to compare
 *
 * @return comparsion result, true if a < b
 */
static bool humanCompare(const InventoryItem& a, const InventoryItem& b)
{
    const char* strA = a.name.c_str();
    const char* strB = b.name.c_str();
    while (true)
    {
        const char& chrA = *strA;
        const char& chrB = *strB;

        // check for end of name
        if (!chrA || !chrB)
        {
            return chrB;
        }

        const bool isNumA = isdigit(chrA);
        const bool isNumB = isdigit(chrB);

        if (isNumA && isNumB)
        {
            // both names have numbers at the same position
            char* endA = nullptr;
            char* endB = nullptr;
            const unsigned long valA = strtoul(strA, &endA, 10);
            const unsigned long valB = strtoul(strB, &endB, 10);
            if (valA != valB)
                return valA < valB;
            strA = endA;
            strB = endB;
        }
        else if (isNumA || isNumB)
        {
            // only one of names has a number
            return isNumA;
        }
        else
        {
            // no digits at position
            if (chrA != chrB)
            {
                return chrA < chrB;
            }
            ++strA;
            ++strB;
        }
    }
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
    {
        return std::get<std::string>(it->second);
    }
    return std::string();
}

void InventoryItem::merge(InventoryItem::Properties& props)
{
    for (auto& [name, value] : props)
    {
        // some properties have spaces at the end, strip them
        if (std::holds_alternative<std::string>(value))
        {
            std::string& v = std::get<std::string>(value);
            v.erase(std::find_if(v.rbegin(), v.rend(),
                                 [](int ch) { return !std::isspace(ch); })
                        .base(),
                    v.end());
        }

        properties[name] = std::move(value);
    }
}

std::vector<InventoryItem> getInventory(sdbusplus::bus::bus& bus)
{
    std::vector<InventoryItem> items;

#ifndef USE_VEGMAN_HACK
    // get all inventory items
    auto subTree = bus.new_method_call(MAPPER_SERVICE, MAPPER_PATH,
                                       MAPPER_IFACE, "GetSubTree");
    const std::vector<std::string> ifaces = {INVENTORY_IFACE};
    subTree.append(INVENTORY_PATH, 0, ifaces);
    std::map<std::string, std::map<std::string, std::vector<std::string>>>
        subTreeObjects;
    bus.call(subTree).read(subTreeObjects);

    for (const auto& [path, objects] : subTreeObjects)
    {
        InventoryItem item;
        item.name = nameFromPath(path);

        for (const auto& [service, _] : objects)
        {
            // get all item's properties
            auto getProps = bus.new_method_call(
                service.c_str(), path.c_str(),
                "org.freedesktop.DBus.Properties", "GetAll");
            getProps.append("");
            InventoryItem::Properties properties;
            bus.call(getProps).read(properties);
            item.merge(properties);
        }

        items.emplace_back(item);
    }
#else
    using IfaceName = std::string;
    using Ifaces = std::map<IfaceName, InventoryItem::Properties>;
    using Objects = std::map<sdbusplus::message::object_path, Ifaces>;

    static const std::vector<std::pair<std::string, std::string>>
        inventoryServices{
            {EM_SERVICE, EM_ROOT_PATH},
            {SMBIOS_SERVICE, SMBIOS_ROOT_PATH},
            {PCIE_SERVICE, PCIE_ROOT_PATH},
            {STORAGE_SERVICE, STORAGE_ROOT_PATH},
        };

    static const std::unordered_set<IfaceName> wantedIfaces{
        "xyz.openbmc_project.Inventory.Decorator.Asset",
        "xyz.openbmc_project.Inventory.Decorator.AssetTag",
        "xyz.openbmc_project.Inventory.Decorator.Revision",
        "xyz.openbmc_project.Inventory.Item",
        "xyz.openbmc_project.Inventory.Item.Chassis",
        "xyz.openbmc_project.Inventory.Item.Cpu",
        "xyz.openbmc_project.Inventory.Item.Dimm",
        "xyz.openbmc_project.Inventory.Item.Drive",
        "xyz.openbmc_project.PCIe.Device",
        "xyz.openbmc_project.State.Decorator.OperationalStatus",
    };

    for (const auto& [service, rootpath] : inventoryServices)
    {
        auto method = bus.new_method_call(service.c_str(), rootpath.c_str(),
                                          "org.freedesktop.DBus.ObjectManager",
                                          "GetManagedObjects");
        Objects objects;
        bus.call(method).read(objects);

        for (auto& [path, object] : objects)
        {
            InventoryItem item;

            for (auto& [iface, props] : object)
            {
                if (wantedIfaces.find(iface) != wantedIfaces.end())
                {
                    item.merge(props);
                }
            }

            if (!item.properties.empty())
            {
                item.name = nameFromPath(path.str);
                items.emplace_back(item);
            }
        }
    }
#endif

    // sort in human readable order
    std::sort(items.begin(), items.end(), humanCompare);

    return items;
}
