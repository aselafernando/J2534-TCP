#ifndef J2534_H
#define J2534_H

#include "J2534_V0404.h"

//Function Prototypes
extern _PassThruConnect PassThruConnect;
extern _PassThruDisconnect PassThruDisconnect;
extern _PassThruReadMsgs PassThruReadMsgs;
extern _PassThruWriteMsgs PassThruWriteMsgs;
extern _PassThruStartPeriodicMsg PassThruStartPeriodicMsg;
extern _PassThruStopPeriodicMsg PassThruStopPeriodicMsg;
extern _PassThruStartMsgFilter PassThruStartMsgFilter;
extern _PassThruStopMsgFilter PassThruStopMsgFilter;
extern _PassThruSetProgrammingVoltage PassThruSetProgrammingVoltage;
extern _PassThruReadVersion PassThruReadVersion;
extern _PassThruGetLastError PassThruGetLastError;
extern _PassThruIoctl PassThruIoctl;
extern _PassThruOpen PassThruOpen;
extern _PassThruClose PassThruClose;
extern _PassThruReset PassThruReset;
extern _PassThruGetLastSocketError PassThruGetLastSocketError;

extern int loadDLL();
extern bool unloadDLL();

#endif
