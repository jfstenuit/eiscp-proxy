/*
 * This file is part of eISCP Proxy, which is licensed under the
 * GNU General Public License v3.0. You can find the full license text
 * in the LICENSE file at the root of the source tree or at
 * https://www.gnu.org/licenses/gpl-3.0.txt.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "types.h"
#include "cmdline.h"

void print_help(const char* progName) {
    printf("Usage: %s [OPTIONS]\n", progName);
    printf("Options:\n");
    printf("  -i <interfaces>  Comma-separated list of interfaces (mandatory)\n");
    printf("  -d               Enable debug mode\n");
    printf("  -t <timeout>     Set timeout interval in seconds (other than 5 seconds)\n");
    printf("  -h               Display this help and exit\n");
}

void handle_command_line(int argc, char *argv[], Environment *args) {
    int opt;
    args->debugging_enabled = 0;
    args->timeout_interval = 5;
    args->interfaces = NULL;
	args->devices = NULL;

    while ((opt = getopt(argc, argv, "i:dt:h")) != -1) {
        switch (opt) {
            case 'i':
                // Split the optarg by commas and populate args->interfaces
                 // Split the optarg by commas to extract interfaces
                char *token = strtok(optarg, ",");
                while (token != NULL) {
                    args->interfaces = addInterface(args->interfaces,token);
                    token = strtok(NULL, ",");
                }
                break;
            case 'd':
                args->debugging_enabled = 1;
                break;
            case 't':
                args->timeout_interval = atoi(optarg);
                break;
			case 'h':
				print_help(argv[0]);
				exit(EXIT_SUCCESS);
            default:
                print_help(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

int isValidInterfaceName(const char* name) {
    while (*name) {
        if (!islower(*name) && !isdigit(*name) && *name != '.') {
            return 0; // Invalid character found
        }
        name++;
    }
    return 1; // Name is valid
}

InterfaceNode* addInterface(InterfaceNode* node, const char* name) {
    if (!isValidInterfaceName(name)) {
        fprintf(stderr, "Invalid interface name: %s\n", name);
        return node;
    }

    InterfaceNode* newNode = malloc(sizeof(InterfaceNode));
    if (!newNode) {
        perror("Failed to allocate memory for new interface node");
        exit(EXIT_FAILURE);
    }
    newNode->name = strdup(name);
    newNode->next = node;
    return newNode;
}

