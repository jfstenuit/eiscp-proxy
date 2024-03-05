/*
 * This file is part of eISCP Proxy, which is licensed under the
 * GNU General Public License v3.0. You can find the full license text
 * in the LICENSE file at the root of the source tree or at
 * https://www.gnu.org/licenses/gpl-3.0.txt.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "types.h"
#include "interface.h"

void setInterfaceIPAddress(InterfaceNode* node) {
    int fd;
    struct ifreq ifr;
    char ip[INET_ADDRSTRLEN];

	// Open a dummy socket to perform the ioctl
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return;
    }

	// Prepare the interface request structure
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = AF_INET; // IPv4 address
    strncpy(ifr.ifr_name, node->name, IFNAMSIZ-1);

	// Retrieve the interface address
    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        perror(node->name);
        close(fd);
		fprintf(stderr, "Error: Interface %s does not exist on this system or does not have an IP address assigned.\n", node->name);
        exit(EXIT_FAILURE);
    }

    close(fd);

	// Copy the address to the node's sockaddr_in structure
    memcpy(&node->address, (struct sockaddr_in *)&ifr.ifr_addr, sizeof(struct sockaddr_in));

    // Convert the IP address to string and store it in the node
    inet_ntop(AF_INET, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, ip, INET_ADDRSTRLEN);
    node->ipAddress = strdup(ip);
}

void enrichInterfaceNodes(InterfaceNode* node) {
    for (InterfaceNode* current = node; current != NULL; current = current->next) {
        setInterfaceIPAddress(current);
    }
}

void freeInterfaceList(InterfaceNode* node) {
    InterfaceNode* current = node;
    while (current) {
        InterfaceNode* next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
}

void dumpInterfaceList(InterfaceNode* node) {
	for (InterfaceNode* current = node; current != NULL; current = current->next) {
        fprintf(stderr, "Interface %s (%s)\n", current->name, current->ipAddress);
    }
}
