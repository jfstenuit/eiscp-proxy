/*
 * This file is part of eISCP Proxy, which is licensed under the
 * GNU General Public License v3.0. You can find the full license text
 * in the LICENSE file at the root of the source tree or at
 * https://www.gnu.org/licenses/gpl-3.0.txt.
 */
#ifndef TYPES_H
#define TYPES_H

#include <netinet/in.h>

#define PORT 60128
#define BUFFER_SIZE 65507

typedef struct InterfaceNode {
    char* name;
	char* ipAddress; // IP address of the interface (as text)
	struct sockaddr_in address; // Ip address of the interface (as system structure)
    struct InterfaceNode* next;
} InterfaceNode;

typedef struct DiscoveredDevice {
    struct sockaddr_in source; // Source IP and port
    char* payload; // Dynamically allocated to store the payload
	size_t payloadSize;         // Size of the payload
    time_t timestamp; // Time when the packet was received
    struct DiscoveredDevice* next; // Pointer to the next element in the list
} DiscoveredDevice;

typedef struct {
	DiscoveredDevice* devices;
	InterfaceNode* interfaces;
    int debugging_enabled;
    int timeout_interval;
} Environment;

#endif
