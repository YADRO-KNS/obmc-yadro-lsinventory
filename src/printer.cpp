// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "printer.hpp"

#include <json.h>

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
    json_object* json = json_object_new_object();

    for (const InventoryItem& item : items)
    {
        if (!checkFilter(item))
        {
            continue;
        }

        json_object* jsonItem = json_object_new_object();

        // print properties
        for (const auto& property : item.properties)
        {
            json_object* jsonProp = nullptr;

            // get value from variant
            std::visit(
                [&jsonProp](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, bool>)
                        jsonProp = json_object_new_boolean(arg);
                    else if constexpr (std::is_arithmetic<T>::value)
                        jsonProp =
                            json_object_new_int64(static_cast<int64_t>(arg));
                    else if constexpr (std::is_same_v<T, std::string>)
                    {
                        if (!arg.empty())
                            jsonProp = json_object_new_string(arg.c_str());
                    }
                    else
                        static_assert(T::value, "Unhandled value type");
                },
                property.second);

            // filter out empty properties
            if (jsonProp || printEmptyProperties)
            {
                json_object_object_add(jsonItem, property.first.c_str(),
                                       jsonProp);
            }
        }

        json_object_object_add(json, item.name.c_str(), jsonItem);
    }

    printf("%s\n",
           json_object_to_json_string_ext(json, JSON_C_TO_STRING_PRETTY |
                                                    JSON_C_TO_STRING_SPACED));

    json_object_put(json);
}

bool Printer::checkFilter(const InventoryItem& item) const
{
    return
        // filter out by name
        (nameFilter.empty() || nameFilter == item.name) &&
        // filter out non-present items
        (printNonPresent || item.isPresent());
}
