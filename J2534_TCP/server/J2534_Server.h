#ifndef J2534_SERVER_H
#define J2534_SERVER_H

#include "J2534_TCPIP.h"

extern ReplyPacket* processCommand(CommandPacket* cmd);

#endif
