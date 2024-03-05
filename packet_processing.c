/*
 * This file is part of eISCP Proxy, which is licensed under the
 * GNU General Public License v3.0. You can find the full license text
 * in the LICENSE file at the root of the source tree or at
 * https://www.gnu.org/licenses/gpl-3.0.txt.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <net/if.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>

#include "types.h"
#include "utilities.h"
#include "rawpacket.h"
#include "packet_processing.h"

int setup_listener() {
    int sockfd;
	int optval = 1;
    struct sockaddr_in addr;

	// Create socket with SO_REUSEPORT
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Enable SO_REUSEPORT
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
        perror("setsockopt SO_REUSEPORT failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to listen for incoming packets
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces for broadcast packets
    addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

	return sockfd;
}

void process_received_packet(int sockfd, Environment *pEnv) {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in senderAddr;
    socklen_t senderAddrLen = sizeof(senderAddr);
    ssize_t receivedLen;
	
    while (1) {
        // Attempt to receive a packet
        receivedLen = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_DONTWAIT, (struct sockaddr *)&senderAddr, &senderAddrLen);

        // If no more packets are available, break the loop
        if (receivedLen == -1) {
            break;
        }

		bool doIgnore = false;
		// Iterate through interfaceList to check if the packet's source IP matches one of our interfaces
		for (InterfaceNode* current = pEnv->interfaces; current != NULL; current = current->next) {
			if (current->address.sin_addr.s_addr == senderAddr.sin_addr.s_addr) {
				doIgnore = true;
				break;
			}
		}

		// Next, check if the packet starts with "ISCP"
		if (receivedLen < 4 || strncmp(buffer, "ISCP", 4) != 0) {
			// Packet does not start with "ISCP", ignore it
			doIgnore = true;
		}
	
		// This packet was sent from one of our interfaces, or does not
		// start with "ISCP" : ignore it
        if (doIgnore == true) {
            continue;
        }

        // Dump the content of the packet
		if (pEnv->debugging_enabled) {
			char senderIP[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(senderAddr.sin_addr), senderIP, INET_ADDRSTRLEN);
			fprintf(stderr,"Received a packet from %s:%d\n", senderIP, ntohs(senderAddr.sin_port));
			hexDump("Packet Content", buffer, receivedLen);
		}

		// Check for specific packet types based on payload starting at byte 16
		if (receivedLen >= 25 && strncmp(buffer + 16, "!xECNQSTN", 9) == 0) {
			// It's a discovery broadcast packet
			reply_to_discovery(&senderAddr,pEnv); 
		} else if (receivedLen >= 20 && strncmp(buffer + 16, "!1ECN", 5) == 0) {
			// It's a discovery response packet
			handle_discovery_response(&senderAddr,pEnv,buffer,receivedLen); 
		}		
    }
}

void send_discovery_packets(Environment *pEnv) {
    int sockfd;
    struct sockaddr_in dest_addr;
    char payload[] = {0x49, 0x53, 0x43, 0x50, 0x00, 0x00, 0x00, 0x10, 
                      0x00, 0x00, 0x00, 0x0a, 0x01, 0x00, 0x00, 0x00, 
                      0x21, 0x78, 0x45, 0x43, 0x4e, 0x51, 0x53, 0x54, 0x4e, 0x0a};

    // Destination address setup
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT); // Destination port
    dest_addr.sin_addr.s_addr = inet_addr("255.255.255.255"); // Broadcast address

    // Iterate through each interface
    for (InterfaceNode* current = pEnv->interfaces; current != NULL; current = current->next) {
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			logger(pEnv,"socket creation failed",errno);
            continue; // Try next interface if socket creation fails
        }

        // Enable SO_REUSEPORT and SO_BROADCAST options
        int optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0 ||
            setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0) {
			logger(pEnv,"setsockopt failed",errno);
            close(sockfd);
            return;
        }

		// Adjust the source address to include the source port
        current->address.sin_port = htons(PORT); // Source port
		
        // Bind to the specific interface's IP address and PORT
        if (bind(sockfd, (struct sockaddr *)&current->address, sizeof(current->address)) < 0) {
            logger(pEnv,"bind failed",errno);
            close(sockfd);
            return;
        }

        // Send the packet
        if (sendto(sockfd, payload, sizeof(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
            logger(pEnv,"sendto failed",errno);
        } else {
			if (pEnv->debugging_enabled) {
				fprintf(stderr,"Discovery packet sent on interface: %s\n", current->name);
			}
        }

        close(sockfd);
    }
}

void handle_discovery_response(const struct sockaddr_in* source, Environment *pEnv, const char* payloadBuffer, ssize_t payloadLength) {
    // Iterate through the list of devices to find if the source already exists
    for (DiscoveredDevice* current = pEnv->devices; current != NULL; current = current->next) {
        if (current->source.sin_port == source->sin_port &&
            current->source.sin_addr.s_addr == source->sin_addr.s_addr) {
            // We found a matching source, update the timestamp and return
            current->timestamp = time(NULL);
			if (pEnv->debugging_enabled) {
				fprintf(stderr,"Updated last seen timestamp for existing device\n");
			}
            return;
        }
    }

    // Create a new DiscoveryResponse node
    DiscoveredDevice* newNode = (DiscoveredDevice*)malloc(sizeof(DiscoveredDevice));
    if (!newNode) {
        logger(pEnv,"Failed to allocate memory for new DiscoveredDevice node",errno);
        return;
    }

    // Copy the source sockaddr_in
    memcpy(&newNode->source, source, sizeof(struct sockaddr_in));

    // Copy the payload into dynamically allocated memory
    newNode->payload = (char*)malloc(payloadLength);
    if (!newNode->payload) {
        logger(pEnv,"Failed to allocate memory for payload",errno);
        free(newNode);
        return;
    }
    memcpy(newNode->payload, payloadBuffer, payloadLength);
    newNode->payloadSize = payloadLength;

    // Set the timestamp
    newNode->timestamp = time(NULL);

    // Insert the new node at the beginning of the list
    newNode->next = pEnv->devices;
	pEnv->devices = newNode;
	
	if (pEnv->debugging_enabled) {
		fprintf(stderr,"Created new device entry\n");
		dump_device_list(pEnv->devices);
	}
	
	return;
}

void reply_to_discovery(const struct sockaddr_in* destAddr, Environment *pEnv) {
	// We need to forge packets with source IP from the discovered devices
	// destAddr contains the IP/Port of requesting party
	// devices->source is a struct sockaddr_in containing the IP/Port we
	//    want to forge in our answers
	// devices->payload and devices->payloadSize represent the payload

    // Iterate through the devices list
    for (DiscoveredDevice* current = pEnv->devices; current != NULL; current = current->next) {
        // Send the payload back to the discoverer
        if (send_raw_udp_packet(&current->source, destAddr, current->payload, current->payloadSize) < 0) {
            logger(pEnv,"sendto failed",errno);
        } else {
			if (pEnv->debugging_enabled) {
				fprintf(stderr,"Discovery reply sent to %s:%d\n", inet_ntoa(destAddr->sin_addr), ntohs(destAddr->sin_port));
			}
        }
    }
}

void remove_stale_devices(Environment *pEnv) {
    DiscoveredDevice *current = pEnv->devices, *prev = NULL;
    time_t now = time(NULL);
    int timeout = 4 * pEnv->timeout_interval; // Timeout threshold

    while (current != NULL) {
        // Calculate time difference
        double diff = difftime(now, current->timestamp);

        if (diff > timeout) {
            DiscoveredDevice* to_delete = current;

			if (pEnv->debugging_enabled) {
				fprintf(stderr,"Removing stale device %s\n",inet_ntoa(current->source.sin_addr));
			}
            // Remove the device from the list
            if (prev == NULL) {
                pEnv->devices = current->next; // Update head if the first device is removed
            } else {
                prev->next = current->next; // Bypass the current device
            }

            current = current->next;

            // Free the removed device's resources
            free(to_delete->payload);
            free(to_delete);
        } else {
            // Move to the next device
            prev = current;
            current = current->next;
        }
    }
}
