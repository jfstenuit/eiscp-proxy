/*
 * This file is part of eISCP Proxy, which is licensed under the
 * GNU General Public License v3.0. You can find the full license text
 * in the LICENSE file at the root of the source tree or at
 * https://www.gnu.org/licenses/gpl-3.0.txt.
 */
#ifndef PACKET_PROCESSING_H
#define PACKET_PROCESSING_H

int setup_listener();
void process_received_packet(int, Environment *);
void send_discovery_packets(Environment *);
void handle_discovery_response(const struct sockaddr_in *, Environment *, const char *, ssize_t);
void reply_to_discovery(const struct sockaddr_in *, Environment *);
void remove_stale_devices(Environment *);

#endif