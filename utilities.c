/*
 * This file is part of eISCP Proxy, which is licensed under the
 * GNU General Public License v3.0. You can find the full license text
 * in the LICENSE file at the root of the source tree or at
 * https://www.gnu.org/licenses/gpl-3.0.txt.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>

#include "types.h"
#include "utilities.h"

void dump_device_list(struct DiscoveredDevice *list) {
    DiscoveredDevice* current = list;
    while (current != NULL) {
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(current->source.sin_addr), ipStr, INET_ADDRSTRLEN);
        printf("Device IP: %s, Port: %d, Last Updated: %s",
               ipStr, ntohs(current->source.sin_port), ctime(&(current->timestamp)));

        printf("Payload (%zu bytes):\n", current->payloadSize);
        hexDump(NULL, current->payload, current->payloadSize); // Assuming hexDump is implemented

        current = current->next;
    }
}

void hexDump(const char *desc, const void *addr, const int len) {
	// desc: A description of the dump, which can be NULL if you don't want to print a description.
    // addr: The starting address of the memory block to dump.
    // len: The length of the memory block to dump in bytes.

    int i;
    unsigned char buff[17];
    const unsigned char *pc = (const unsigned char *)addr;

    // Description of the dump
    if (desc != NULL)
        printf("%s:\n", desc);

    // Process every byte in the data
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset)
        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset
            printf("  %04x ", i);
        }

        // Print the hex code for the current byte
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later
        if (isprint(pc[i]))
            buff[i % 16] = pc[i];
        else
            buff[i % 16] = '.';

        // Null-terminate the ASCII buffer
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out the last line if not exactly 16 characters
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    // And print the final ASCII buffer
    printf("  %s\n", buff);
}

void daemonize() {
    pid_t pid;

    // Fork off the parent process
    pid = fork();

    // An error occurred
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    // Success: Let the parent terminate
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // On success: The child process becomes session leader
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    // Ignore signal sent from child to parent process
    signal(SIGCHLD, SIG_IGN);

    // Fork off for the second time
    pid = fork();

    // An error occurred
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    // Success: Let the second parent terminate
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Set new file permissions
    umask(0);

    // Change the working directory to the root directory
    // or another appropriated directory
    chdir("/");

    // Write the PID to /var/run/eiscp-proxy.pid
    int pidFile = open("/var/run/eiscp-proxy.pid", O_RDWR|O_CREAT, 0600);
    if (pidFile < 0) {
        perror("Failed to open PID file /var/run/eiscp-proxy.pid");
        exit(EXIT_FAILURE);
    }

    // Ensure only one instance of the daemon is running
    if (lockf(pidFile, F_TLOCK, 0) < 0) {
        // Couldn't obtain lock on PID file, another instance may be running
        perror("An instance of this daemon is already running");
        exit(EXIT_FAILURE);
    }

    // Write PID to PID file
    char str[256];
    sprintf(str, "%d\n", getpid());
    write(pidFile, str, strlen(str));
	
    // Close all open file descriptors
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
        close (x);
    }

    // Initialize syslog
	openlog("eiscp-proxy", LOG_PID | LOG_CONS, LOG_USER);
}

void logger(const Environment *env,const char *msg, int errnum) {
	if (env->debugging_enabled) {
		// If debugging is enabled, print to stderr
		if (errnum) {
			perror(msg);
		} else {
			fprintf(stderr, "%s\n", msg);
		}
	} else {
		// If debugging is not enables, we are daemonized --> log to syslog
		if (errnum) {
			syslog(LOG_ERR, "%s: %s", msg, strerror(errnum));
		} else {
			syslog(LOG_INFO, "%s", msg);
		}
	}
}	