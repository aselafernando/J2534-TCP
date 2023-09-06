#ifndef J2534_TCPIP_H
#define J2534_TCPIP_H

#pragma pack(1)

#include <stdint.h>
#include "J2534_V0404.h"

enum J2534_Command : uint8_t {
	OPEN = 0, //PassThruOpen
	CONNECT, //PassThruConnect
	READ_MSGS, //PassThruReadMsgs
	WRITE_MSGS, //PassThruWriteMsgs
	IOCTL, //PassThruIoctl
	START_PERIODIC_MSG, //PassThruStartPeriodicMsg
	STOP_PERIODIC_MSG, //PassThruStopPeriodicMsg
	START_MSG_FILTER, //PassThruStartMsgFilter
	STOP_MSG_FILTER, //PassThruStopMsgFilter
	SET_PROGRAMMING_VOLTAGE, //PassThruSetProgrammingVoltage
	READ_VERSION, //PassThruReadVersion
	DISCONNECT, //PassThruDisconnect
	CLOSE, //PassThruClose
	RESET, //PassThruReset
	GET_LAST_ERROR, //PassThruGetLastError
	GET_LAST_SOCKET_ERROR //PassThruGetLastSocketError
};

typedef struct {
	uint32_t len;
	J2534_Command cmd;
	uint8_t data[];
} CommandPacket;

typedef struct {
	uint32_t len;
	J2534_Command cmd;
	RETURN_STATUS result;
	uint8_t data[];
} ReplyPacket;

//PassThruOpen
typedef char PassThruOpenCmd;
typedef uint32_t PassThruOpenReply;

//PassThruClose
typedef uint32_t PassThruCloseCmd;

//PassThruDisconnect
typedef uint32_t PassThruDisconnectCmd;

//PassThruIoctl
typedef struct {
	uint32_t channelID;
	IOCTL_ID ioctlID;
	uint8_t input[];
} PassThruIoctlCmd;

typedef uint8_t PassThruIoctlReply;

//PassThruReadVersion
typedef uint32_t PassThruReadVersionCmd;

typedef struct {
	char firmwareVersion[80];
	char dllVersion[80];
	char apiVersion[80];
} PassThruReadVersionReply;

//PassThruReadMsgs
typedef struct {
	uint32_t channelID;
	uint32_t numMsgs;
	uint32_t timeout;
	PASSTHRU_MSG msgs[];
} PassThruReadMsgsCmd;

typedef struct {
	uint32_t numMsgs;
	PASSTHRU_MSG msgs[];
} PassThruReadMsgsReply;

//PassThruWriteMsgs
typedef PassThruReadMsgsCmd PassThruWriteMsgsCmd;
typedef uint32_t PassThruWriteMsgsReply;

//PassThruConnect
typedef struct {
	uint32_t deviceID;
	ProtocolID protocolID;
	uint32_t flags;
	uint32_t baudRate;
} PassThruConnectCmd;

typedef uint32_t PassThruConnectReply;

//PassThruStartMsgFilter
typedef struct {
	uint32_t channelID;
	Filter filterType;
	PASSTHRU_MSG maskMsg;
	PASSTHRU_MSG patternMsg;
	PASSTHRU_MSG flowControlMsg;
} PassThruStartMsgFilterCmd;

typedef uint32_t PassThruStartMsgFilterReply;

//PassThruStopMsgFilter
typedef struct {
	uint32_t channelID;
	uint32_t msgID;
} PassThruStopMsgFilterCmd;

//PassThruSetProgrammingVoltage
typedef struct {
	uint32_t deviceID;
	uint32_t pinNumber;
	Voltage voltage;
} PassThruSetProgrammingVoltageCmd;

//PassThruGetLastError
typedef struct {
	char error[80];
} PassThruGetLastErrorReply;

//PassThruGetLastSocketError
typedef PassThruGetLastErrorReply PassThruGetLastSocketErrorReply;

//PassThruStartPeriodicMsg

typedef struct {
	uint32_t channelID;
	PASSTHRU_MSG msg;
	uint32_t timeInterval;
} PassThruStartPeriodicMsgCmd;

typedef uint32_t PassThruStartPeriodicMsgReply;

//PassThruStopPeriodicMsg

typedef struct {
	uint32_t channelID;
	uint32_t msgID;
} PassThruStopPeriodicMsgCmd;

#endif
