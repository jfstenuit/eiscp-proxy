/*
 * This file is part of eISCP Proxy, which is licensed under the
 * GNU General Public License v3.0. You can find the full license text
 * in the LICENSE file at the root of the source tree or at
 * https://www.gnu.org/licenses/gpl-3.0.txt.
 */
#ifndef UTILITIES_H
#define UTILITIES_H

#include "types.h"

void dump_device_list(struct DiscoveredDevice *);
void hexDump(const char *, const void *, const int);
void daemonize();
void logger(const Environment *,const char *, int);


#endif