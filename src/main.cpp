// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "printer.hpp"

#include <getopt.h>

#include <iostream>

/**
 * @brief Print help usage info.
 *
 * @param[in] app - application's file name
 */
static void printHelp(const char* app)
{
    std::cout << "Print BMC inventory list." << std::endl;
    std::cout << "Copyright (c) 2020 YADRO." << std::endl;
    std::cout << "Usage: " << app << " [OPTION...]" << std::endl;
    std::cout << "  -n, --name=NAME     Print item with specified name only"
              << std::endl;
    std::cout << "  -a, --all           Also print nonexistent units"
              << std::endl;
    std::cout << "  -e, --empty         Also print empty properties"
              << std::endl;
    std::cout << "  -j, --json          Print in JSON format" << std::endl;
    std::cout << "  -h, --help          Print this help and exit" << std::endl;
}

/** @brief Application entry point. */
int main(int argc, char* argv[])
{
    Printer printer;
    bool printJson = false;

    // clang-format off
    const struct option opts[] = {
        {"name",  required_argument, nullptr, 'n'},
        {"all",   no_argument,       nullptr, 'a'},
        {"empty", no_argument,       nullptr, 'e'},
        {"json",  no_argument,       nullptr, 'j'},
        {"help",  no_argument,       nullptr, 'h'},
        {0, 0, nullptr, 0 }
    };
    // clang-format on

    // short options
    char shortOpts[(sizeof(opts) / sizeof(opts[0])) * 2 + 1];
    char* shortOptPtr = shortOpts;
    for (size_t i = 0; i < sizeof(opts) / sizeof(opts[0]); ++i)
    {
        if (!opts[i].flag)
        {
            *shortOptPtr++ = opts[i].val;
            if (opts[i].has_arg != no_argument)
                *shortOptPtr++ = ':';
        }
    }

    // parse arguments
    opterr = 0;
    int optVal;
    while ((optVal = getopt_long(argc, argv, shortOpts, opts, nullptr)) != -1)
    {
        switch (optVal)
        {
            case 'n':
                printer.setNameFilter(optarg);
                break;
            case 'a':
                printer.allowNonexitent();
                break;
            case 'e':
                printer.allowEmptyProperties();
                break;
            case 'j':
                printJson = true;
                break;
            case 'h':
                printHelp(argv[0]);
                return EXIT_SUCCESS;
            default:
                std::cerr << "Invalid option: " << argv[optind - 1]
                          << std::endl;
                return EXIT_FAILURE;
        }
    }
    if (optind < argc)
    {
        std::cerr << "Invalid options: ";
        while (optind < argc)
            std::cerr << argv[optind++] << " ";
        std::cerr << std::endl;
        return EXIT_FAILURE;
    }

    // print inventory list
    try
    {
        const std::vector<InventoryItem> items = getInventory();
        if (printJson)
            printer.printJson(items);
        else
            printer.printText(items);
    }
    catch (std::exception& ex)
    {
        std::cerr << "Error reading inventory: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
