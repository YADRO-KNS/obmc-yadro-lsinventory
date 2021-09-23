// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "printer.hpp"

#include <nlohmann/json.hpp>

void Printer::setNameFilter(const char* name)
{
    nameFilter = name;
}

void Printer::allowNonPresent()
{
    printNonPresent = true;
}

void Printer::allowEmptyProperties()
{
    printEmptyProperties = true;
}

void Printer::printText(const std::vector<InventoryItem>& items) const
{
    // Size of the column with property name (formatting output)
    static const int PropNmColWidth = 20;

    for (const InventoryItem& item : items)
    {
        if (!checkFilter(item))
        {
            continue;
        }

        // print title
        printf("%s: %s\n", item.name.c_str(), item.prettyName().c_str());

        // print properties
        for (const auto& property : item.properties)
        {
            // get value from variant
            std::string val;
            std::visit(
                [&val](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, bool>)
                        val = arg ? "Yes" : "No";
                    else if constexpr (std::is_arithmetic<T>::value)
                        val = std::to_string(arg);
                    else if constexpr (std::is_same_v<T, std::string>)
                        val = arg;
                    else
                        static_assert(T::value, "Unhandled value type");
                },
                property.second);

            // filter out empty properties
            if (!val.empty() || printEmptyProperties)
            {
                const int nameLen = static_cast<int>(property.first.length());
                printf("  %s: %*s%s\n", property.first.c_str(),
                       nameLen < PropNmColWidth ? PropNmColWidth - nameLen : 0,
                       "", val.c_str());
            }
        }
    }
}

void Printer::printJson(const std::vector<InventoryItem>& items) const
{
    nlohmann::json json = nlohmann::json::object();

    for (const InventoryItem& item : items)
    {
        if (!checkFilter(item))
        {
            continue;
        }

        nlohmann::json jsonItem = nlohmann::json::object();

        // print properties
        for (const auto& property : item.properties)
        {
            nlohmann::json jsonProp;

            // get value from variant
            std::visit([&jsonProp](auto&& arg) { jsonProp = arg; },
                       property.second);

            const bool isEmpty =
                (jsonProp.is_string() ? jsonProp == "" : jsonProp.empty());
            if (!isEmpty || printEmptyProperties)
            {
                jsonItem.emplace(property.first, jsonProp);
            }
        }

        if (!jsonItem.empty())
        {
            json.emplace(item.name, jsonItem);
        }
    }

    constexpr auto JsonPrettyLookOffset = 2;
    printf("%s\n", json.dump(JsonPrettyLookOffset).c_str());
}

bool Printer::checkFilter(const InventoryItem& item) const
{
    return
        // filter out by name
        (nameFilter.empty() || nameFilter == item.name) &&
        // filter out non-present items
        (printNonPresent || item.isPresent());
}
