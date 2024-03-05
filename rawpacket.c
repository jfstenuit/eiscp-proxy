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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>  // IP header
#include <netinet/udp.h> // UDP header
#include <arpa/inet.h>

// Function to calculate IP header checksum
unsigned short checksum(void *b, int len) {    
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// Function to send a raw UDP packet
ssize_t send_raw_udp_packet(const struct sockaddr_in *src, const struct sockaddr_in *dst, const char *payload, size_t payload_len) {
    int sockfd;
    struct iphdr *iph;
    struct udphdr *udph;
    char packet[4096];
    char *data;
    int packet_len;
	ssize_t bytes_sent;

    // Create a raw socket
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        perror("socket failed");
        return -1;
    }

    // Pointer to the IP header within 'packet'
    iph = (struct iphdr *)packet;

    // Pointer to the UDP header within 'packet'
    udph = (struct udphdr *)(packet + sizeof(struct iphdr));

    // Pointer to the data/payload within 'packet'
    data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);

    // Fill in the data/payload
    memcpy(data, payload, payload_len);

    // Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + payload_len;
    iph->id = htonl(54321);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0; // Set to 0 before calculating checksum
    iph->saddr = src->sin_addr.s_addr;
    iph->daddr = dst->sin_addr.s_addr;
    iph->check = checksum((unsigned short *)packet, iph->tot_len);

    // Fill in the UDP Header
    udph->source = src->sin_port;
    udph->dest = dst->sin_port;
    udph->len = htons(sizeof(struct udphdr) + payload_len);
    udph->check = 0; // UDP checksum is optional, set to 0

    // Calculate total packet length
    packet_len = iph->tot_len;

    // Inform the kernel do not fill up the packet structure, we will build our own...
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &(int){1}, sizeof(int)) < 0) {
        perror("setsockopt");
        return -1;
    }

    // Send the packet
	bytes_sent=sendto(sockfd, packet, packet_len, 0, (struct sockaddr *)dst, sizeof(*dst));
    if (bytes_sent < 0) {
        perror("sendto failed");
    }

    // Close the socket
    close(sockfd);
	
	return bytes_sent;
}
