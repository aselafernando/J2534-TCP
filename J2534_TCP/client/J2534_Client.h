#ifndef J2534_CLIENT_H
#define J2534_CLIENT_H

#pragma pack(1)

#include <stdint.h>
#include "J2534_V0404.h"

#define SERVER "127.0.0.1"
#define PORT 2534

#if defined(_WIN32)
	//  Microsoft 
	#define EXPORTED __declspec(dllexport) RETURN_STATUS J2534_API
	#define IMPORTED __declspec(dllimport) RETURN_STATUS J2534_API
#elif defined(__GNUC__)
	//  GCC
	#define EXPORTED extern "C" __attribute__((visibility("default"))) RETURN_STATUS
	#define IMPORTED extern "C" RETURN_STATUS
#else
	//  do nothing and hope for the best?
	#define EXPORTED
	#define IMPORTED
	#error "Unknown dynamic link import/export semantics."
#endif

EXPORTED PassThruConnect (
    uint32_t DeviceID,
    ProtocolID ProtocolID,
    uint32_t Flags,
    uint32_t BaudRate,
    uint32_t *pChannelID
);
                                                                                                                            
EXPORTED PassThruDisconnect(
	uint32_t ChannelID
);

EXPORTED PassThruReadMsgs
(
	uint32_t ChannelID,
	PASSTHRU_MSG *pMsg,
	uint32_t *pNumMsgs,
	uint32_t Timeout
);

EXPORTED PassThruWriteMsgs
(
	uint32_t ChannelID,
	PASSTHRU_MSG *pMsg,
	uint32_t *pNumMsgs,
	uint32_t Timeout
);

EXPORTED PassThruStartPeriodicMsg
(
	uint32_t ChannelID,
	PASSTHRU_MSG *pMsg,
	uint32_t *pMsgID,
	uint32_t TimeInterval
);

EXPORTED PassThruStopPeriodicMsg
(
	uint32_t ChannelID,
	uint32_t MsgID
);

EXPORTED PassThruStartMsgFilter
(
	uint32_t ChannelID,
	Filter FilterType,
	PASSTHRU_MSG *pMaskMsg,
	PASSTHRU_MSG *pPatternMsg,
	PASSTHRU_MSG *pFlowControlMsg,
	uint32_t *pMsgID
);

EXPORTED PassThruStopMsgFilter
(
	uint32_t ChannelID,
	uint32_t MsgID
);

EXPORTED PassThruSetProgrammingVoltage
(
	uint32_t DeviceID,
	uint32_t PinNumber,
	Voltage Voltage
);

EXPORTED PassThruReadVersion
(
	uint32_t DeviceID,
	char *pFirmwareVersion,
	char *pDllVersion,
	char *pApiVersion
);

EXPORTED PassThruGetLastError
(
	char *pErrorDescription
);

EXPORTED PassThruIoctl
(
	uint32_t ChannelID,
	IOCTL_ID IoctlID,
	void *pInput,
	void *pOutput
);

EXPORTED PassThruOpen
(
	char *pName,
	uint32_t *pDeviceID
);

EXPORTED PassThruClose
(
	uint32_t DeviceID
);

EXPORTED PassThruReset
(
	void
);

EXPORTED PassThruGetLastSocketError
(
	char *pErrorDescription
);

#endif
