/*
 * This file is part of eISCP Proxy, which is licensed under the
 * GNU General Public License v3.0. You can find the full license text
 * in the LICENSE file at the root of the source tree or at
 * https://www.gnu.org/licenses/gpl-3.0.txt.
 */
#ifndef INTERFACE_H
#define INTERFACE_H

#include "types.h"

void setInterfaceIPAddress(InterfaceNode* node);
void enrichInterfaceNodes(InterfaceNode* node);
void freeInterfaceList(InterfaceNode* node);
void dumpInterfaceList(InterfaceNode* node);

#endif