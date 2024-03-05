/*
 * This file is part of eISCP Proxy, which is licensed under the
 * GNU General Public License v3.0. You can find the full license text
 * in the LICENSE file at the root of the source tree or at
 * https://www.gnu.org/licenses/gpl-3.0.txt.
 */
#ifndef RAWPACKET_H
#define RAWPACKET_H

unsigned short checksum(void *, int);
ssize_t send_raw_udp_packet(const struct sockaddr_in *, const struct sockaddr_in *, const char *, size_t);

#endif