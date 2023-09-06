#ifndef J2534_V0404_H
#define J2534_V0404_H

#pragma pack(1)

#include <stdint.h>

#ifdef _WIN32
#define J2534_API __stdcall
#else
#define J2534_API
#endif

enum ProtocolID : uint32_t
{
	J1850VPW = 1, // GM/DaimlerChrylser CLASS2
	J1850PWM, //Ford SCP
	ISO9141, //ISO9141 and ISO9141-2
	ISO14230, //ISO14230-4 Keywrod Protocol 2000
	CAN, //Raw CAN (flow control not automatically handled by interface
	ISO15765, //ISO15765-2 flow control enabled
	SCI_A_ENGINE, //SAE J2610 (Daimler Chrysler SCI) configuration A for engine
	SCI_A_TRANS, //SAE J2610 (Daimler Chrysler SCI) configuration A for transmission
	SCI_B_ENGINE, //SAE J2610 (Daimler Chrysler SCI) configuration B for engine
	SCI_B_TRANS, //SAE J2610 (Daimler Chrysler SCI) configuration B for transmission
	/* 0x0B - 0xFFFF Reserved for SAE use */
	/* 0x10000 - 0xFFFFFFFF - Toold manufacturer specific */
};

enum Filter : uint32_t {
	PASS_FILTER = 1,
	BLOCK_FILTER = 2,
	FLOW_CONTROL_FILTER = 3
};

enum Voltage : uint32_t {
	SHORT_TO_GROUND = 0xFFFFFFFE,
	VOLTAGE_OFF = 0xFFFFFFFF
};

enum IOCTL_ID : uint32_t {
	GET_CONFIG = 1,
	SET_CONFIG,
	READ_VBATT,
	FIVE_BAUD_INIT,
	FAST_INIT,
	CLEAR_TX_BUFFER,
	CLEAR_RX_BUFFER,
	CLEAR_PERIODIC_MSGS,
	CLEAR_MSG_FILTERS,
	CLEAR_FUNCT_MSG_LOOKUP_TABLE,
	ADD_TO_FUNCT_MSG_LOOKUP_TABLE,
	DELETE_FROM_FUNCT_MSG_LOOKUP_TABLE,
	READ_PROG_VOLTAGE
	/* 0x0F - 0xFFFF REserved for SAE use */
	/* 0x1000 - 0xFFFFFFFF Tool manufacturer specific */
};

enum NETWORK_LINE : uint32_t {
	BUS_NORMAL = 0,
	BUS_PLUS,
	BUS_MINUS
};

enum LOOPBACK : uint32_t {
	OFF = 0,
	ON
};

enum PARITY : uint32_t {
	NO_PARITY = 0,
	ODD_PARITY,
	EVEN_PARITY
};

enum RETURN_STATUS : uint32_t {
	STATUS_NOERROR = 0, // Function call successful
	ERR_NOT_SUPPORTED, // Function call not supported
	ERR_INVALID_CHANNEL_ID, // Invalid Channel ID
	ERR_INVALID_PROTOCOL_ID, //Invalid Protocol ID
	ERR_NULLPARAMETER, // NULL pointer supplied where a valid pointer is required
	ERR_INVALID_IOCTL_VALUE, //Invalid value for Ioctl parameter
	ERR_INVALID_FLAGS, //Invalid flag values
	ERR_FAILED, // Undefined Error. Get description with PassThruGetLastError
	ERR_DEVICE_NOT_CONNECTED, //Device not connected to PC
	ERR_TIMEOUT, //Timeout. No message available to read or could not read the specified number of messages. Actual number of messages read is in NumMsgs
	ERR_INVALID_MSG, //Invalid message structure pointed to by pMsg
	ERR_INVALID_TIME_INTERVAL, //Invalid TimerInterval Value
	ERR_EXCEEDED_LIMIT, //Exceeded max number of message IDs or allocated space
	ERR_INVALID_MSG_ID, //Invalid MsgID value
	ERR_INVALID_ERROR_ID, //Invalid ErrorID value
	ERR_INVALID_IOCTL_ID, //Invalid IoctlID value
	ERR_BUFFER_EMPTY, //Protocol message buffer empty
	ERR_BUFFER_FULL, //Protocol message buffer full
	ERR_BUFFER_OVERFLOW, //Protocol message buffer overflow
	ERR_PIN_INVALID, //Invalid PIN
	ERR_CHANNEL_IN_USE, //Channel already in use
	ERR_MSG_PROTOCOL_ID //Protocol type does not match the protocol associated with ChannelID
	/* 0x16 - 0xFFFFFFFF Reserver for SAE use */
};

typedef struct {
	ProtocolID protocolID; /* protocol type */
	uint32_t RxStatus; /* Receieve message status */
	uint32_t TxFlags; /* Transmit message flags */
	uint32_t Timestamp; /* Recieved message timestamp (ms) */
	uint32_t DataSize; /* Data size in bytes */
	uint32_t ExtraDataIndex; /* Start position of extra data in receieved message */
	uint8_t  Data[4128]; /* The Data */
} PASSTHRU_MSG;

typedef struct {
	uint32_t Parameter; /* name of parameter */
	uint32_t Value; /* value of parameter */
} SCONFIG;

typedef struct {
	uint32_t NumOfParams; /* number of SCONFIG elements */
	SCONFIG* ConfigPtr; /* array of SCONFIG */
} SCONFIG_LIST;

typedef struct {
	uint32_t NumOfBytes; /* number of bytes in the array */
	uint8_t* BytePtr; /* array of bytes */
} SBYTE_ARRAY;

//Function Pointer Types
typedef RETURN_STATUS(J2534_API* _PassThruConnect)(
	uint32_t DeviceID,
	ProtocolID ProtocolID,
	uint32_t Flags,
	uint32_t BaudRate,
	uint32_t* pChannelID
);

typedef RETURN_STATUS(J2534_API* _PassThruDisconnect)(
	uint32_t ChannelID
);

typedef RETURN_STATUS(J2534_API* _PassThruReadMsgs)
(
	uint32_t ChannelID,
	PASSTHRU_MSG* pMsg,
	uint32_t* pNumMsgs,
	uint32_t Timeout
);

typedef RETURN_STATUS(J2534_API* _PassThruWriteMsgs)
(
	uint32_t ChannelID,
	PASSTHRU_MSG* pMsg,
	uint32_t* pNumMsgs,
	uint32_t Timeout
);

typedef RETURN_STATUS(J2534_API* _PassThruStartPeriodicMsg)
(
	uint32_t ChannelID,
	PASSTHRU_MSG* pMsg,
	uint32_t* pMsgID,
	uint32_t TimeInterval
);

typedef RETURN_STATUS(J2534_API* _PassThruStopPeriodicMsg)
(
	uint32_t ChannelID,
	uint32_t MsgID
);

typedef RETURN_STATUS(J2534_API* _PassThruStartMsgFilter)
(
	uint32_t ChannelID,
	Filter FilterType,
	PASSTHRU_MSG* pMaskMsg,
	PASSTHRU_MSG* pPatternMsg,
	PASSTHRU_MSG* pFlowControlMsg,
	uint32_t* pMsgID
);

typedef RETURN_STATUS(J2534_API* _PassThruStopMsgFilter)
(
	uint32_t ChannelID,
	uint32_t MsgID
);

typedef RETURN_STATUS(J2534_API* _PassThruSetProgrammingVoltage)
(
	uint32_t DeviceID,
	uint32_t PinNumber,
	Voltage Voltage
);

typedef RETURN_STATUS(J2534_API* _PassThruReadVersion)
(
	uint32_t DeviceID,
	char* pFirmwareVersion,
	char* pDllVersion,
	char* pApiVersion
);

typedef RETURN_STATUS(J2534_API* _PassThruGetLastError)
(
	char* pErrorDescription
);

typedef RETURN_STATUS(J2534_API* _PassThruIoctl)
(
	uint32_t ChannelID,
	IOCTL_ID IoctlID,
	void* pInput,
	void* pOutput
);

typedef RETURN_STATUS(J2534_API* _PassThruOpen)
(
	char* pName,
	uint32_t* pDeviceID
);

typedef RETURN_STATUS(J2534_API* _PassThruClose)
(
	uint32_t DeviceID
);

typedef RETURN_STATUS(J2534_API* _PassThruReset)
(
	void
);

typedef RETURN_STATUS(J2534_API* _PassThruGetLastSocketError)
(
	char* pErrorDescription
);

#endif
