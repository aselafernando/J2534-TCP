#include "stdafx.h"
#include <stdlib.h>
#include <cstring>
#include <sstream>
#include <cstdint>
#include <sys/types.h>

#define SOCKET_TIMEOUT_SECONDS 5

static SOCKET sock = INVALID_SOCKET;

static void printPassThruMsg(PASSTHRU_MSG* msg) {
    outfile << "DataSize: " << to_string(msg->DataSize) << endl;
    outfile << "ProtocolID: " << to_string(msg->protocolID) << endl;
    outfile << "RXStatus: " << to_string(msg->RxStatus) << endl;
    outfile << "TXFlags: " << to_string(msg->TxFlags) << endl;
    outfile << "Timestamp: " << to_string(msg->Timestamp) << endl;
    outfile << "ExtraDataIndex: " << to_string(msg->ExtraDataIndex) << endl;
    outfile << "Data[0]: " << to_string(msg->Data[0]) << endl;
    outfile.flush();
}

static void print_SCONFIG_LIST(SCONFIG_LIST* sl) {
    uint32_t i = 0;
    outfile << "NumOfParams: " << to_string(sl->NumOfParams) << endl;
    for (i = 0; i < sl->NumOfParams; i++) {
        outfile << "[" << to_string(i) << "].Parameter = " << to_string(sl->ConfigPtr[i].Parameter) << endl;
        outfile << "[" << to_string(i) << "].Value = " << to_string(sl->ConfigPtr[i].Value) << endl;
    }
    outfile.flush();
}

static void disconnectServer() {
    if (doLog) {
        outfile << "disconnectServer()" << endl;
        outfile.flush();
    }
    #ifdef _WIN32
        WSACleanup();
        closesocket(sock);
    #else //_WIN32
        close(sock);
    #endif //_WIN32
    sock = INVALID_SOCKET;
}

static bool connectToServer() {
    struct sockaddr_in ipOfServer;
    #ifdef _WIN32
        DWORD timeout = SOCKET_TIMEOUT_SECONDS * 1000;
        WSADATA wsaData;
        int res = WSAStartup(0x202, &wsaData);

        if (doLog) {
            outfile << "WSAStartup " << res << endl;
            outfile.flush();
        }

        if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
            if (doLog) {
                outfile << "WSA Error: " << WSAGetLastError() << endl;
                outfile.flush();
            }
            return false;
        }

        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        InetPtonA(AF_INET, serverAddr.c_str(), &ipOfServer.sin_addr.s_addr);
    #else //_WIN32
        struct timeval timeout;
        timeout.tv_sec = SOCKET_TIMEOUT_SECONDS;
        timeout.tv_usec = 0;

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            if (doLog) {
                outfile << "Cannot make socket!" << endl;
                outfile.flush();
            }
            return false;
        }

        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        ipOfServer.sin_addr.s_addr = inet_addr(serverAddr.c_str());
    #endif //_WIN32
    
    ipOfServer.sin_family = AF_INET;
    ipOfServer.sin_port = htons(serverPort);

    if (doLog) {
        outfile << "making connection to " << serverAddr << " on port "  << to_string(serverPort) << endl;
        outfile.flush();
    }
    if(connect(sock, (struct sockaddr *)&ipOfServer, sizeof(ipOfServer)) < 0) {
        if (doLog) {
            outfile << "connection error" << endl;
        }
        disconnectServer();
        return false;
    }
    if (doLog) {
        outfile << "connection made" << endl;
        outfile.flush();
    }
    return true;
}

static void sendData(uint8_t* pData, uint32_t len) {
    uint32_t i;
    int sentLen;

    for (i = 0; i < len; i += sentLen) {
        sentLen = send(sock, (char*)pData + i, len - i, 0);
        if (doLog) {
            outfile << "sentLen " << sentLen << endl;
            outfile.flush();
        }
        if (sentLen <= 0) {
            disconnectServer();
            break;
        }
    }
}

static ReplyPacket* getReply() {
    uint32_t i;
    int readLen;
    uint32_t replyLen;
    ReplyPacket* pReply = NULL;
    
    for(i = 0; i < sizeof(replyLen); i += readLen) {
        readLen = recv(sock, (char*)&replyLen + i, sizeof(replyLen) - i, 0);
        if (doLog) {
            outfile << "readLen " << readLen << endl;
            outfile.flush();
        }
        if(readLen <= 0) {
            //Disconnect Server if there is a read error
            disconnectServer();
            return NULL;
        }
    }

    if (doLog) {
        outfile << "replyLen " << replyLen << endl;
        outfile.flush();
    }

    pReply = (ReplyPacket*)malloc(replyLen);
    if(pReply != NULL) {
        pReply->len = replyLen;
        replyLen -= sizeof(replyLen);
        for(i = 0; i < replyLen; i += readLen) {
            readLen = recv(sock, (char*)pReply + sizeof(replyLen) + i, replyLen - i, 0);
            if (doLog) {
                outfile << "readLen " << readLen << endl;
                outfile.flush();
            }
            if(readLen <= 0) {
                //Disconnect Server if there is a read error
                disconnectServer();
                return pReply;
            }
        }
    }

    return pReply;
}

static ReplyPacket* sendAndReply(CommandPacket* pCmd) {
    if (doLog) {
        outfile << "Sending CmdLen: " << pCmd->len << endl;
        outfile << "Sending Cmd: " << std::to_string(pCmd->cmd) << endl;
        outfile.flush();
    }

    sendData((uint8_t*)pCmd, pCmd->len);
    return getReply();
}

RETURN_STATUS J2534_API PassThruOpen(char* pName, uint32_t* pDeviceID) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = ERR_FAILED;

    #ifndef _WIN32
    readINI();
    #endif

    if (doLog) {
        outfile << "PassThruOpen()" << endl;
        outfile.flush();
    }

    if (pName == NULL) {
        pCmd = (CommandPacket*)malloc(sizeof(CommandPacket));
        if (pCmd != NULL) {
            pCmd->len = sizeof(CommandPacket);
            pCmd->cmd = J2534_Command::OPEN;
        }
    }
    else {
        size_t nameLen = strlen(pName);
        pCmd = (CommandPacket*)malloc(sizeof(CommandPacket) + nameLen + 1);
        if (pCmd != NULL) {
            pCmd->len = sizeof(CommandPacket) + nameLen + 1;
            memcpy(pCmd->data, pName, nameLen);
            pCmd->data[nameLen] = '\0';
            pCmd->cmd = J2534_Command::OPEN;
        }
    }

    if (pCmd == NULL) return result;

    if (sock == INVALID_SOCKET) {
        if (!connectToServer()) {
            free(pCmd);
            return result;
        }
    }

    pReply = sendAndReply(pCmd);
    free(pCmd);

    if (pReply != NULL) {
        result = pReply->result;
        if (doLog) {
            outfile << " = " << std::to_string(pReply->result) << endl;
            outfile.flush();
        }
        *pDeviceID = *(uint32_t*)(pReply->data);
        free(pReply);

        //if (result != RETURN_STATUS::STATUS_NOERROR) {
        //    disconnectServer();
        //}
    }

    return result;
}

RETURN_STATUS J2534_API PassThruStartMsgFilter(uint32_t ChannelID, Filter FilterType, PASSTHRU_MSG *pMaskMsg, PASSTHRU_MSG *pPatternMsg, PASSTHRU_MSG *pFlowControlMsg, uint32_t *pMsgID) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    if (doLog) {
        outfile << "PassThruStartMsgFilter(ChannelID = " << to_string(ChannelID) << ", FilterType = " << to_string(FilterType) << ")" << endl;
        outfile.flush();
    }

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket) + sizeof(PassThruStartMsgFilterCmd));

    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket) + sizeof(PassThruStartMsgFilterCmd);
        pCmd->cmd = J2534_Command::START_MSG_FILTER;
        PassThruStartMsgFilterCmd* pCmdData = (PassThruStartMsgFilterCmd*)(pCmd->data);
        pCmdData->channelID = ChannelID;
        pCmdData->filterType = FilterType;
        if (pMaskMsg != NULL) {
            memcpy(&(pCmdData->maskMsg), pMaskMsg, sizeof(PASSTHRU_MSG));
            if (doLog) {
                outfile << "maskMsg: " << endl;
                printPassThruMsg(&(pCmdData->maskMsg));
            }
            //pCmdData->maskMsg = *pMaskMsg;
        }
        else {
            memset(&(pCmdData->maskMsg), 0, sizeof(PASSTHRU_MSG));
        }
        if (pPatternMsg != NULL) {
            //pCmdData->patternMsg = *pPatternMsg;
            memcpy(&(pCmdData->patternMsg), pPatternMsg, sizeof(PASSTHRU_MSG));
            if (doLog) {
                outfile << "patternMsg: " << endl;
                printPassThruMsg(&(pCmdData->patternMsg));
            }
        }
        else {
            memset(&(pCmdData->patternMsg), 0, sizeof(PASSTHRU_MSG));
        }
        if (pFlowControlMsg != NULL) {
            memcpy(&(pCmdData->flowControlMsg), pFlowControlMsg, sizeof(PASSTHRU_MSG));
            if (doLog) {
                outfile << "flowControlMsg: " << endl;
                printPassThruMsg(&(pCmdData->flowControlMsg));
            }
            //pCmdData->flowControlMsg = *pFlowControlMsg;
        }
        else {
            memset(&(pCmdData->flowControlMsg), 0, sizeof(PASSTHRU_MSG));
        }
        pReply = sendAndReply(pCmd);
        free(pCmd);
    }

    if (pReply != NULL) {
        result = pReply->result;
        *pMsgID = *(uint32_t*)(pReply->data);
        if (doLog) {
            outfile << "MsgID: " << to_string(*pMsgID) << endl;
            outfile.flush();
        }
        free(pReply);
    }

    return result;
}

RETURN_STATUS J2534_API PassThruStopMsgFilter(uint32_t ChannelID, uint32_t MsgID) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    if (doLog) {
        outfile << "PassThruStopMsgFilter(" << to_string(ChannelID) << "," << to_string(MsgID) << ")" << endl;
        outfile.flush();
    }

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket) + sizeof(PassThruStopMsgFilterCmd));

    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket) + sizeof(PassThruStopMsgFilterCmd);
        pCmd->cmd = J2534_Command::STOP_MSG_FILTER;
        PassThruStopMsgFilterCmd* pCmdData = (PassThruStopMsgFilterCmd*)(pCmd->data);
        pCmdData->channelID = ChannelID;
        pCmdData->msgID = MsgID;
        pReply = sendAndReply(pCmd);
        free(pCmd);
    }

    if (pReply != NULL) {
        result = pReply->result;
        free(pReply);
    }

    return result;
}

RETURN_STATUS J2534_API PassThruSetProgrammingVoltage(uint32_t DeviceID, uint32_t PinNumber, Voltage Voltage) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket) + sizeof(PassThruSetProgrammingVoltageCmd));

    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket) + sizeof(PassThruSetProgrammingVoltageCmd);
        pCmd->cmd = J2534_Command::SET_PROGRAMMING_VOLTAGE;
        PassThruSetProgrammingVoltageCmd* pCmdData = (PassThruSetProgrammingVoltageCmd*)(pCmd->data);
        pCmdData->deviceID = DeviceID;
        pCmdData->pinNumber = PinNumber;
        pCmdData->voltage = Voltage;
        pReply = sendAndReply(pCmd);
        free(pCmd);
    }

    if (pReply != NULL) {
        result = pReply->result;
        free(pReply);
    }

    return result;
}

RETURN_STATUS J2534_API PassThruReadVersion(uint32_t DeviceID, char *pFirmwareVersion, char *pDllVersion, char *pApiVersion) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket) + sizeof(PassThruReadVersionCmd));

    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket) + sizeof(PassThruReadVersionCmd);
        pCmd->cmd = J2534_Command::READ_VERSION;
        *(uint32_t*)(pCmd->data) = DeviceID;
        pReply = sendAndReply(pCmd);
        free(pCmd);
    }

    if (pReply != NULL) {
        result = pReply->result;
        PassThruReadVersionReply* pReplyData = (PassThruReadVersionReply*)(pReply->data);
        memcpy(pFirmwareVersion, pReplyData->firmwareVersion, 80);
        memcpy(pDllVersion, pReplyData->dllVersion, 80);
        memcpy(pApiVersion, pReplyData->apiVersion, 80);
        free(pReply);
    }

    return result;
}

RETURN_STATUS J2534_API PassThruGetLastError(char* pErrorDescription) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    if (doLog) {
        outfile << "PassThruGetLastError()" << endl;
        outfile.flush();
    }

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket));
    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket);
        pCmd->cmd = J2534_Command::GET_LAST_ERROR;
        pReply = sendAndReply(pCmd);
        free(pCmd);
    }

    if (pReply != NULL) {
        result = pReply->result;

        if(pErrorDescription != NULL)
            memcpy(pErrorDescription, (char*)(pReply->data), sizeof(PassThruGetLastErrorReply));

        if (doLog) {
            outfile << "PassThruGetLastError(" << pErrorDescription << ") = " << std::to_string(result) << endl;
            outfile.flush();
        }

        free(pReply);
    }

    return result;
}

RETURN_STATUS J2534_API PassThruGetLastSocketError(char* pErrorDescription) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket));
    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket);
        pCmd->cmd = J2534_Command::GET_LAST_SOCKET_ERROR;
        pReply = sendAndReply(pCmd);
        free(pCmd);
    }

    if (pReply != NULL) {
        result = pReply->result;
        if (pErrorDescription != NULL)
            memcpy(pErrorDescription, pReply->data, sizeof(PassThruGetLastSocketErrorReply));
        free(pReply);
    }

    return result;
}

RETURN_STATUS J2534_API PassThruReset() {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket));
    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket);
        pCmd->cmd = J2534_Command::RESET;

        if (sock < 0) {
            if (!connectToServer()) {
                free(pCmd);
                return result;
            }
        }

        pReply = sendAndReply(pCmd);
        free(pCmd);
    }

    if (pReply != NULL) {
        result = pReply->result;
        free(pReply);
    }

    return result;
}

RETURN_STATUS J2534_API PassThruConnect(uint32_t DeviceID, ProtocolID ProtocolID, uint32_t Flags, uint32_t BaudRate, uint32_t *pChannelID) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    if (doLog) {
        outfile << "PassThruConnect(" << std::to_string(DeviceID) << "," << std::to_string(ProtocolID) << "," << std::to_string(Flags) << "," << std::to_string(BaudRate) << ")" << endl;
        outfile.flush();
    }

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket) + sizeof(PassThruConnectCmd));
    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket) + sizeof(PassThruConnectCmd);
        pCmd->cmd = J2534_Command::CONNECT;
        PassThruConnectCmd* pCmdData = (PassThruConnectCmd*)(pCmd->data);
        pCmdData->deviceID = DeviceID;
        pCmdData->protocolID = ProtocolID;
        pCmdData->flags = Flags;
        pCmdData->baudRate = BaudRate;
        pReply = sendAndReply(pCmd);
        free(pCmd);
    }

    if (pReply != NULL) {
        result = pReply->result;
        if (doLog) {
            outfile << " = " << std::to_string(result) << endl;
            outfile.flush();
        }
        *pChannelID = *(uint32_t*)(pReply->data);
        free(pReply);
    }

    return result;
}

RETURN_STATUS J2534_API PassThruDisconnect(uint32_t ChannelID) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket) + sizeof(PassThruDisconnectCmd));

    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket) + sizeof(PassThruDisconnectCmd);
        pCmd->cmd = J2534_Command::DISCONNECT;
        PassThruDisconnectCmd* pCmdData = (PassThruDisconnectCmd*)(pCmd->data);
        *pCmdData = ChannelID;
        pReply = sendAndReply(pCmd);
        free(pCmd);
    }

    if (pReply != NULL) {
        result = pReply->result;
        free(pReply);
    }

    return result;
}

RETURN_STATUS J2534_API PassThruClose(uint32_t deviceID) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    if (doLog) {
        outfile << "PassThruClose(" << to_string(deviceID) << ")" << endl;
        outfile.flush();
    }

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket) + sizeof(PassThruCloseCmd));

    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket) + sizeof(PassThruCloseCmd);
        pCmd->cmd = J2534_Command::CLOSE;
        PassThruCloseCmd* pCmdData = (PassThruCloseCmd*)(pCmd->data);
        *pCmdData = deviceID;
        pReply = sendAndReply(pCmd);
        free(pCmd);
    }

    if (pReply != NULL) {
        result = pReply->result;
        free(pReply);
        if (result == RETURN_STATUS::STATUS_NOERROR) {
            disconnectServer();
        }
    }

    return result;
}


RETURN_STATUS J2534_API PassThruReadMsgs(uint32_t ChannelID, PASSTHRU_MSG *pMsg, uint32_t *pNumMsgs, uint32_t Timeout) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    if (doLog) {
        outfile << "PassThruReadMsgs(" << to_string(ChannelID) << ")" << endl;
        outfile.flush();
    }

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket) + sizeof(PassThruReadMsgsCmd));

    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket) + sizeof(PassThruReadMsgsCmd) + (*pNumMsgs * sizeof(PASSTHRU_MSG));
        pCmd->cmd = J2534_Command::READ_MSGS;
        PassThruReadMsgsCmd* pCmdData = (PassThruReadMsgsCmd*)(pCmd->data);
        pCmdData->channelID = ChannelID;
        pCmdData->numMsgs = *pNumMsgs;
        pCmdData->timeout = Timeout;
        sendData((uint8_t*)pCmd, sizeof(CommandPacket) + sizeof(PassThruReadMsgsCmd));
        sendData((uint8_t*)pMsg, *pNumMsgs * sizeof(PASSTHRU_MSG));
        free(pCmd);
        pReply = getReply();
    }

    if (pReply != NULL) {
        result = pReply->result;
        PassThruReadMsgsReply* pReplyData = (PassThruReadMsgsReply*)(pReply->data);
        if (pReplyData->numMsgs > 0)
            memcpy((uint8_t*)pMsg, (uint8_t*)(pReplyData->msgs), pReplyData->numMsgs * sizeof(PASSTHRU_MSG));
        free(pReply);
    }

    return result;
}

RETURN_STATUS J2534_API PassThruWriteMsgs(uint32_t ChannelID, PASSTHRU_MSG *pMsg, uint32_t *pNumMsgs, uint32_t Timeout) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    if (doLog) {
        outfile << "PassThruWriteMsgs(" << to_string(ChannelID) << "," << to_string(*pNumMsgs) << "," << to_string(Timeout) << ")" << endl;
        printPassThruMsg(pMsg);
        outfile.flush();
    }

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket) + sizeof(PassThruWriteMsgsCmd));

    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket) + sizeof(PassThruWriteMsgsCmd) + (*pNumMsgs * sizeof(PASSTHRU_MSG));
        pCmd->cmd = J2534_Command::WRITE_MSGS;
        PassThruWriteMsgsCmd* pCmdData = (PassThruWriteMsgsCmd*)(pCmd->data);
        pCmdData->channelID = ChannelID;
        pCmdData->numMsgs = *pNumMsgs;
        pCmdData->timeout = Timeout;
        sendData((uint8_t*)pCmd, sizeof(CommandPacket) + sizeof(PassThruWriteMsgsCmd));
        sendData((uint8_t*)pMsg, *pNumMsgs * sizeof(PASSTHRU_MSG));
        free(pCmd);
        pReply = getReply();
    }

    if (pReply != NULL) {
        result = pReply->result;
        //PassThruWriteMsgsReply* pReplyData = (PassThruWriteMsgsReply*)(pReply->data);
        *pNumMsgs = *(uint32_t*)(pReply->data);
        if (doLog) {
            outfile << "= " << to_string(result) << endl;
            outfile << "NumMsgs: " << to_string(*pNumMsgs) << endl;
            outfile.flush();
        }
        free(pReply);
    }

    return result;
}

RETURN_STATUS J2534_API PassThruStartPeriodicMsg(uint32_t ChannelID, PASSTHRU_MSG *pMsg, uint32_t *pMsgID, uint32_t TimeInterval) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    if (doLog) {
        outfile << "PassThruStartPeriodicMsg(" << to_string(ChannelID) << ")" << endl;
        outfile.flush();
    }

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket) + sizeof(PassThruStartPeriodicMsgCmd));

    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket) + sizeof(PassThruStartPeriodicMsgCmd);
        pCmd->cmd = J2534_Command::START_PERIODIC_MSG;
        PassThruStartPeriodicMsgCmd* pCmdData = (PassThruStartPeriodicMsgCmd*)(pCmd->data);
        pCmdData->channelID = ChannelID;
        pCmdData->timeInterval = TimeInterval;
        memcpy((uint8_t*)&(pCmdData->msg), (uint8_t*)pMsg, sizeof(PASSTHRU_MSG));
        pReply = sendAndReply(pCmd);
        free(pCmd);
    }

    if (pReply != NULL) {
        result = pReply->result;
        *pMsgID = *(uint8_t*)(pReply->data);
        free(pReply);
    }

    return result;
}

RETURN_STATUS J2534_API PassThruStopPeriodicMsg(uint32_t ChannelID, uint32_t MsgID) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;

    if (doLog) {
        outfile << "PassThruStopPeriodicMsg(" << to_string(ChannelID) << "," << to_string(MsgID) << ")" << endl;
        outfile.flush();
    }

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket) + sizeof(PassThruStopPeriodicMsgCmd));

    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket) + sizeof(PassThruStopPeriodicMsgCmd);
        pCmd->cmd = J2534_Command::STOP_PERIODIC_MSG;
        PassThruStopPeriodicMsgCmd* pCmdData = (PassThruStopPeriodicMsgCmd*)(pCmd->data);
        pCmdData->channelID = ChannelID;
        pCmdData->msgID = MsgID;
        pReply = sendAndReply(pCmd);
        free(pCmd);
    }

    if (pReply != NULL) {
        result = pReply->result;
        free(pReply);
    }

    return result;
}

RETURN_STATUS J2534_API PassThruIoctl(uint32_t ChannelID, IOCTL_ID IoctlID, void *pInput, void *pOutput) {
    CommandPacket *pCmd = NULL;
    ReplyPacket *pReply = NULL;
    size_t cmdDataLen = 0;
    RETURN_STATUS result = RETURN_STATUS::ERR_FAILED;
    //Placeholder pointers
    SCONFIG_LIST* pSConfigList = NULL;
    SBYTE_ARRAY* pSByteArray = NULL;

    if (doLog) {
        outfile << "PassThruIoctl(" << to_string(ChannelID) << "," << to_string(IoctlID) << ")" << endl;
        outfile.flush();
    }

    switch (IoctlID) {
        case IOCTL_ID::GET_CONFIG:
        case IOCTL_ID::SET_CONFIG:
            //pInput = SCONFIG_LIST*
            //pOutput = NULL
            pSConfigList = (SCONFIG_LIST*)pInput;
            cmdDataLen = sizeof(pSConfigList->NumOfParams) + (pSConfigList->NumOfParams * sizeof(SCONFIG));
            if (doLog) {
                outfile << "pInput:" << endl;
                print_SCONFIG_LIST(pSConfigList);
            }
            break;
        case IOCTL_ID::READ_VBATT:
        case IOCTL_ID::READ_PROG_VOLTAGE:
            //pInput = NULL;
            //pOutput = uint32_t*;
            break;
        case IOCTL_ID::FIVE_BAUD_INIT:
            //pInput = SBYTE_ARRAY*
            //pOutput = SBYTE_ARRAY* Output always contains a 2 byte long SBYTE_ARRAY
            pSByteArray = (SBYTE_ARRAY*)pInput;
            cmdDataLen = sizeof(pSByteArray->NumOfBytes) + pSByteArray->NumOfBytes;
            break;
        case IOCTL_ID::FAST_INIT:
            //pInput = PASSTHRU_MSG*
            //pOutput = PASSTHRU_MSG*
            cmdDataLen = sizeof(PASSTHRU_MSG);
            if (doLog) {
                outfile << "pInput:" << endl;
                printPassThruMsg((PASSTHRU_MSG*)&pInput);
            }
            break;
        case IOCTL_ID::CLEAR_TX_BUFFER:
        case IOCTL_ID::CLEAR_RX_BUFFER:
        case IOCTL_ID::CLEAR_PERIODIC_MSGS:
        case IOCTL_ID::CLEAR_MSG_FILTERS:
        case IOCTL_ID::CLEAR_FUNCT_MSG_LOOKUP_TABLE:
            //pInput = NULL;
            //pOutput = NULL;
            break;
        case IOCTL_ID::ADD_TO_FUNCT_MSG_LOOKUP_TABLE:
        case IOCTL_ID::DELETE_FROM_FUNCT_MSG_LOOKUP_TABLE:
            //pInput = SBYTE_ARRAY*;
            //pOutput = NULL;
            pSByteArray = (SBYTE_ARRAY*)pInput;
            cmdDataLen = sizeof(pSByteArray->NumOfBytes) + pSByteArray->NumOfBytes;
            break;
        default:
            return result;
    }

    pCmd = (CommandPacket*)malloc(sizeof(CommandPacket) + sizeof(PassThruIoctlCmd));

    if (pCmd != NULL) {
        pCmd->len = sizeof(CommandPacket) + sizeof(PassThruIoctlCmd) + cmdDataLen;
        pCmd->cmd = J2534_Command::IOCTL;
        PassThruIoctlCmd* pCmdData = (PassThruIoctlCmd*)(pCmd->data);
        pCmdData->channelID = ChannelID;
        pCmdData->ioctlID = IoctlID;
        sendData((uint8_t*)pCmd, sizeof(CommandPacket) + sizeof(PassThruIoctlCmd));
        free(pCmd);

        if (IoctlID == IOCTL_ID::FIVE_BAUD_INIT || IoctlID == IOCTL_ID::DELETE_FROM_FUNCT_MSG_LOOKUP_TABLE) {
            //Send SBYTE ARRAY
            sendData((uint8_t*)&(pSByteArray->NumOfBytes), sizeof(pSByteArray->NumOfBytes));
            sendData((uint8_t*)(pSByteArray->BytePtr), pSByteArray->NumOfBytes);
        }
        else if (IoctlID == IOCTL_ID::GET_CONFIG || IoctlID == IOCTL_ID::SET_CONFIG) {
            //Send SCONFIG List
            sendData((uint8_t*)&(pSConfigList->NumOfParams), sizeof(pSConfigList->NumOfParams));
            sendData((uint8_t*)(pSConfigList->ConfigPtr), pSConfigList->NumOfParams * sizeof(SCONFIG));
        }
        else {
            if (pInput != NULL && cmdDataLen > 0)
                sendData((uint8_t*)pInput, cmdDataLen);
        }

        pReply = getReply();

        if (pReply != NULL) {
            result = pReply->result;
            switch (IoctlID) {
            case IOCTL_ID::GET_CONFIG:
                memcpy((uint8_t*)(pSConfigList->ConfigPtr), pReply->data + sizeof(uint32_t), pSConfigList->NumOfParams * sizeof(SCONFIG));
                break;
            case IOCTL_ID::FIVE_BAUD_INIT:
                //Output is always a 2 byte array within SBYTE_ARRAY
                pSByteArray = (SBYTE_ARRAY*)pOutput;
                memcpy((uint8_t*)&(pSByteArray->NumOfBytes), pReply->data, sizeof(pSByteArray->NumOfBytes));
                memcpy((uint8_t*)(pSByteArray->BytePtr), pReply->data + sizeof(pSByteArray->NumOfBytes), pSByteArray->NumOfBytes);
                break;
            case IOCTL_ID::FAST_INIT:
                memcpy((uint8_t*)pOutput, pReply->data, sizeof(PASSTHRU_MSG));
                if (doLog) {
                    outfile << "pOutput:" << endl;
                    printPassThruMsg((PASSTHRU_MSG*)&(pReply->data));
                }
                break;
            case IOCTL_ID::READ_VBATT:
            case IOCTL_ID::READ_PROG_VOLTAGE:
                memcpy((uint8_t*)pOutput, pReply->data, sizeof(uint32_t));
                break;
            default:
                break;
            }
            free(pReply);
        }
    }

    return result;
}
