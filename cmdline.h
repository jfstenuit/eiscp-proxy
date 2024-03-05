/*
 * This file is part of eISCP Proxy, which is licensed under the
 * GNU General Public License v3.0. You can find the full license text
 * in the LICENSE file at the root of the source tree or at
 * https://www.gnu.org/licenses/gpl-3.0.txt.
 */
#ifndef CMDLINE_H
#define CMDLINE_H

#include "types.h"

void handle_command_line(int argc, char *argv[], Environment *args);
InterfaceNode* addInterface(InterfaceNode*, const char*);
int isValidInterfaceName(const char*);

#endif
