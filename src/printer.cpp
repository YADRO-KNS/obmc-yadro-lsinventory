// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "printer.hpp"

#include <json.h>

#include <iostream>

void Printer::setNameFilter(const char* name)
{
    nameFilter_ = name;
}

void Printer::allowNonexitent()
{
    allowNonexitent_ = true;
}

void Printer::allowEmptyProperties()
{
    allowEmptyProperties_ = true;
}

void Printer::printText(const std::vector<InventoryItem>& items) const
{
    // Number of characters used for printing property name
    static const size_t OutputPropertyWidth = 20;

    for (const InventoryItem& item : items)
    {
        // filter out by name
        if (!nameFilter_.empty() && nameFilter_ != item.name)
            continue;

        // filter out nonexistent items
        if (!allowNonexitent_ && !item.isPresent())
            continue;

        // print title
        std::cout << item.name << ": " << item.prettyName() << std::endl;

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
            if (!allowEmptyProperties_ && val.empty())
                continue;

            std::cout << "  " << property.first << ":";
            std::cout.width(OutputPropertyWidth - property.first.length());
            std::cout << " " << val << std::endl;
        }
    }
}

void Printer::printJson(const std::vector<InventoryItem>& items) const
{
    json_object* json = json_object_new_object();

    for (const InventoryItem& item : items)
    {
        // filter out by name
        if (!nameFilter_.empty() && nameFilter_ != item.name)
            continue;

        // filter out nonexistent items
        if (!allowNonexitent_ && !item.isPresent())
            continue;

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
            if (jsonProp || allowEmptyProperties_)
            {
                json_object_object_add(jsonItem, property.first.c_str(),
                                       jsonProp);
            }
        }

        json_object_object_add(json, item.name.c_str(), jsonItem);
    }

    std::cout << json_object_to_json_string_ext(
                     json, JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED)
              << std::endl;

    json_object_put(json);
}
