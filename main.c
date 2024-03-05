/*
Proxy for the eISCP protocol
https://tascam.com/downloads/tascam/790/pa-r100_200_protocol.pdf
*/
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
#include <syslog.h>

#include "types.h"
#include "interface.h"
#include "packet_processing.h"
#include "utilities.h"
#include "cmdline.h"

int main(int argc, char *argv[]) {
	Environment env;
    int sockfd;

    if (geteuid() != 0) {
        fprintf(stderr, "This application must be run as root. Exiting.\n");
        exit(EXIT_FAILURE);
    }

    handle_command_line(argc, argv, &env);

    // Check if interfaces were specified
    if (env.interfaces == NULL) {
        fprintf(stderr, "Error: No interfaces specified. The -i option is mandatory.\n");
        fprintf(stderr, "Usage: %s -i interface1,interface2\n", argv[0]);
        exit(EXIT_FAILURE);
    }

	enrichInterfaceNodes(env.interfaces);
	
	if (env.debugging_enabled) {
		fprintf(stderr, "Selected interfaces:\n");
		dumpInterfaceList(env.interfaces);
	}

	if (!env.debugging_enabled) {
		daemonize();
		logger(&env,"Daemon started successfully.",0);
	}

	sockfd=setup_listener();

	// Start by sending a first batch of discovery packets
	send_discovery_packets(&env);

    struct timeval tv;
    fd_set readfds;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        // Set timeout for select
        tv.tv_sec = env.timeout_interval;
        tv.tv_usec = 0;

        // Wait for a packet or a timeout
        int ret = select(sockfd + 1, &readfds, NULL, NULL, &tv);
        if (ret < 0) {
			logger(&env, "select error", errno);
            exit(EXIT_FAILURE);
        } else if (ret == 0) {
            // Timeout occurred, time to send discovery packets
			remove_stale_devices(&env);
            send_discovery_packets(&env);
        } else {
            // Packet received
            if (FD_ISSET(sockfd, &readfds)) {
                process_received_packet(sockfd,&env);
            }
        }
    }

    close(sockfd);
    closelog();
    return 0;
}
