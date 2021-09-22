// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "config.hpp"
#include "printer.hpp"
#include "version.hpp"

#include <getopt.h>

/**
 * @brief Print help usage info.
 *
 * @param[in] app application's file name
 */
static void printHelp(const char* app)
{
    printf("Print BMC inventory list.\n");
    printf("Copyright (c) 2020 YADRO.\n");
    printf("Version " VERSION "\n");
    printf("Usage: %s [OPTION...]\n", app);
    printf("  -n, --name=NAME  Print item with specified name only\n");
    printf("  -a, --all        Also print non-present units\n");
    printf("  -e, --empty      Also print empty properties\n");
    printf("  -j, --json       Print in JSON format\n");
    printf("  -h, --help       Print this help and exit\n");
#ifdef REMOTE_HOST_SUPPORT
    printf("  -H, --host       Get data from remote host over SSH\n");
#endif
}

/** @brief Application entry point. */
int main(int argc, char* argv[])
{
    Printer printer;
    bool printJson = false;
#ifdef REMOTE_HOST_SUPPORT
    const char* host = nullptr;
#endif

    // clang-format off
    const struct option longOpts[] = {
        {"name",  required_argument, nullptr, 'n'},
        {"all",   no_argument,       nullptr, 'a'},
        {"empty", no_argument,       nullptr, 'e'},
        {"json",  no_argument,       nullptr, 'j'},
        {"help",  no_argument,       nullptr, 'h'},
#ifdef REMOTE_HOST_SUPPORT
        {"host",  required_argument, nullptr, 'H'},
#endif
        {nullptr, 0,                 nullptr,  0 }
    };
#ifdef REMOTE_HOST_SUPPORT
    const char* shortOpts = "n:aejhH:";
#else
    const char* shortOpts = "n:aejh";
#endif
    // clang-format on

    opterr = 0; // prevent native error messages

    // parse arguments
    int val;
    while ((val = getopt_long(argc, argv, shortOpts, longOpts, nullptr)) != -1)
    {
        switch (val)
        {
            case 'n':
                printer.setNameFilter(optarg);
                break;
            case 'a':
                printer.allowNonPresent();
                break;
            case 'e':
                printer.allowEmptyProperties();
                break;
            case 'j':
                printJson = true;
                break;
#ifdef REMOTE_HOST_SUPPORT
            case 'H':
                host = optarg;
                break;
#endif
            case 'h':
                printHelp(argv[0]);
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "Invalid option: %s\n", argv[optind - 1]);
                return EXIT_FAILURE;
        }
    }
    if (optind < argc)
    {
        fprintf(stderr, "Unexpected option: %s\n", argv[optind]);
        return EXIT_FAILURE;
    }

    // print inventory list
    try
    {
        sdbusplus::bus::bus bus = sdbusplus::bus::new_default();
#ifdef REMOTE_HOST_SUPPORT
        if (host)
        {
            sd_bus* b = nullptr;
            sd_bus_open_system_remote(&b, host);
            bus = sdbusplus::bus::bus(b, std::false_type());
        }
#endif

        const std::vector<InventoryItem> items = getInventory(bus);
        if (printJson)
        {
            printer.printJson(items);
        }
        else
        {
            printer.printText(items);
        }
    }
    catch (std::exception& ex)
    {
        fprintf(stderr, "Error reading inventory: %s\n", ex.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
