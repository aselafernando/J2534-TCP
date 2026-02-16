#include <stdlib.h>
#include <stdio.h>
#include <cstring>

#include "J2534_Server.h"
#include "J2534.h"

extern bool verbose;

void printSCONFIG_LIST(SCONFIG_LIST* list) {
    uint32_t i;
    if (list != NULL) {
        fprintf(stdout, "\tNumOfParams: %d\n", list->NumOfParams);
        for (i = 0; i < list->NumOfParams; i++) {
            fprintf(stdout, "\t\tConfigPtr[%d].Parameter: %d\n", i, list->ConfigPtr[i].Parameter);
            fprintf(stdout, "\t\tConfigPtr[%d].Value: %d\n", i, list->ConfigPtr[i].Value);
        }
    }
    else {
        fprintf(stdout, "\tNULL\n");
    }
}

void printSBYTE_ARRAY(SBYTE_ARRAY* arr) {
    uint32_t i;

    if (arr != NULL) {
        fprintf(stdout, "\tNumOfBytes: %d\n", arr->NumOfBytes);
        for (i = 0; i < arr->NumOfBytes; i++) {
            fprintf(stdout, "\t\tBytePtr[%d] = %d\n", i, arr->BytePtr[i]);
        }
    }
    else {
        fprintf(stdout, "\tNULL\n");
    }
}

void printPassThruMsg(PASSTHRU_MSG* msg) {
    uint32_t i;
    if (msg != NULL) {
        fprintf(stdout, "\tProtocolID: %d\n", msg->protocolID);
        fprintf(stdout, "\tRxStatus: %d\n", msg->RxStatus);
        fprintf(stdout, "\tTxFlags: %d\n", msg->TxFlags);
        fprintf(stdout, "\tTimestamp: %d\n", msg->Timestamp);
        fprintf(stdout, "\tDataSize: %d\n", msg->DataSize);
        fprintf(stdout, "\tExtraDataIndex: %d\n", msg->ExtraDataIndex);

        for (i = 0; i < msg->DataSize; i++) {
            fprintf(stdout, "\t\tData[%d] = %d\n", i, msg->Data[i]);
        }
    }
    else {
        fprintf(stdout, "\tNULL\n");
    }
}

ReplyPacket* Server_PassThruOpen(char* name) {
    ReplyPacket* pReply = NULL;

    if (verbose)
        fprintf(stdout, "PassThruOpen()\n");

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket) + sizeof(PassThruOpenReply));
    if (pReply != NULL) {
        memset(pReply->data, 0, sizeof(PassThruOpenReply));
        pReply->cmd = J2534_Command::OPEN;
        pReply->len = sizeof(ReplyPacket) + sizeof(PassThruOpenReply);

        pReply->result = PassThruOpen(NULL, (uint32_t*)(pReply->data));

        if (pReply->result == RETURN_STATUS::STATUS_NOERROR) {
            if (verbose)
                fprintf(stdout, "deviceID = %d\n", *(uint32_t*)(pReply->data));
        }
    }
    return pReply;
}

ReplyPacket* Server_PassThruClose(uint32_t* deviceID) {
    ReplyPacket* pReply = NULL;
    if (verbose)
        fprintf(stdout, "PassThruClose(%d)\n", *deviceID);

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket));
    if (pReply != NULL) {
        pReply->cmd = J2534_Command::CLOSE;
        pReply->len = sizeof(ReplyPacket);
        pReply->result = PassThruClose(*deviceID);
    }
    return pReply;
}

ReplyPacket* Server_PassThruReadMsgs(PassThruReadMsgsCmd* readMsgsCmd) {
    ReplyPacket* pReply = NULL;
    uint32_t i = 0;

    if (verbose)
        fprintf(stdout, "PassThruReadMsgs(%d, %d, %d)\n", readMsgsCmd->channelID, readMsgsCmd->numMsgs, readMsgsCmd->timeout);

    size_t pReplyDataLen = sizeof(PassThruReadMsgsReply) + (readMsgsCmd->numMsgs * sizeof(PASSTHRU_MSG));
    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket) + pReplyDataLen);

    if (pReply != NULL) {
        pReply->cmd = J2534_Command::READ_MSGS;
        PassThruReadMsgsReply* readMsgsReply = (PassThruReadMsgsReply*)(pReply->data);
        readMsgsReply->numMsgs = readMsgsCmd->numMsgs;

        memcpy((uint8_t*)(readMsgsReply->msgs), (uint8_t*)(readMsgsCmd->msgs),
            readMsgsCmd->numMsgs * sizeof(PASSTHRU_MSG));

        //pReply->result = RETURN_STATUS::ERR_TIMEOUT;
        //readMsgsReply->numMsgs = 0;

        pReply->result = PassThruReadMsgs(readMsgsCmd->channelID, readMsgsReply->msgs,
            &(readMsgsReply->numMsgs), readMsgsCmd->timeout);

        if (verbose) {
            fprintf(stdout, "numMsgs = %d\n", readMsgsReply->numMsgs);

            for (i = 0; i < readMsgsReply->numMsgs; i++) {
                printPassThruMsg((PASSTHRU_MSG*)((uint8_t*)(readMsgsReply->msgs) + (sizeof(PASSTHRU_MSG) * i)));
            }
        }

        pReply->len = sizeof(ReplyPacket) + sizeof(PassThruReadMsgsReply) +
            (readMsgsReply->numMsgs * sizeof(PASSTHRU_MSG));
    }
    return pReply;
}

ReplyPacket* Server_PassThruWriteMsgs(PassThruWriteMsgsCmd* msgsCmd) {
    ReplyPacket* pReply = NULL;
    uint32_t i = 0;

    if (verbose) {
        fprintf(stdout, "PassThruWriteMsgs(%d, %d, %d)\n", msgsCmd->channelID, msgsCmd->timeout, msgsCmd->numMsgs);
        for (i = 0; i < msgsCmd->numMsgs; i++) {
            printPassThruMsg((PASSTHRU_MSG*)((uint8_t*)(msgsCmd->msgs) + (sizeof(PASSTHRU_MSG) * i)));
        }
    }

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket) + sizeof(PassThruWriteMsgsReply));

    if (pReply != NULL) {
        pReply->cmd = J2534_Command::WRITE_MSGS;
        pReply->len = sizeof(ReplyPacket) + sizeof(PassThruWriteMsgsReply);
        // pReply->result = RETURN_STATUS::ERR_TIMEOUT;
        // *(uint32_t*)(pReply->data) = 0;
        memcpy(pReply->data, &(msgsCmd->numMsgs), sizeof(uint32_t));
        pReply->result = PassThruWriteMsgs(msgsCmd->channelID, msgsCmd->msgs,
            (uint32_t*)(pReply->data), msgsCmd->timeout);
        if (verbose)
            fprintf(stdout, "numMsgs = %d\n", *(uint32_t*)(pReply->data));
    }
    return pReply;
}

ReplyPacket* Server_PassThruConnect(PassThruConnectCmd* connectCmd) {
    ReplyPacket* pReply = NULL;

    if (verbose)
        fprintf(stdout, "PassThruConnect(%d, %d, %d, %d)\n", connectCmd->deviceID, connectCmd->protocolID, connectCmd->flags, connectCmd->baudRate);

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket) + sizeof(PassThruConnectReply));
    if (pReply != NULL) {
        memset(pReply->data, 0, sizeof(PassThruConnectReply));
        pReply->cmd = J2534_Command::CONNECT;
        pReply->len = sizeof(ReplyPacket) + sizeof(PassThruConnectReply);
        pReply->result = PassThruConnect(connectCmd->deviceID, connectCmd->protocolID, connectCmd->flags, connectCmd->baudRate, (uint32_t*)(pReply->data));
        if (verbose)
            fprintf(stdout, "channelID = %d\n", *(uint32_t*)(pReply->data));
    }

    return pReply;
}

ReplyPacket* Server_PassThruDisconnect(uint32_t* channelID) {
    ReplyPacket* pReply = NULL;

    if (verbose)
        fprintf(stdout, "PassThruDisconnect(%d)\n", *channelID);

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket));
    if (pReply != NULL) {
        pReply->cmd = J2534_Command::DISCONNECT;
        pReply->len = sizeof(ReplyPacket);
        pReply->result = PassThruDisconnect(*channelID);
    }

    return pReply;
}

ReplyPacket* Server_PassThruReset() {
    ReplyPacket* pReply = NULL;

    if (verbose)
        fprintf(stdout, "PassThruReset()\n");

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket));
    if (pReply != NULL) {
        pReply->cmd = J2534_Command::RESET;
        pReply->len = sizeof(ReplyPacket);
        pReply->result = PassThruReset();
    }

    return pReply;
}

ReplyPacket* Server_PassThruReadVersion(uint32_t* deviceID) {
    ReplyPacket* pReply = NULL;

    if (verbose)
        fprintf(stdout, "PassThruReadVersion(%d)\n", *deviceID);

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket) + sizeof(PassThruReadVersionReply));

    if (pReply != NULL) {
        memset(pReply->data, 0, sizeof(PassThruReadVersionReply));
        pReply->cmd = J2534_Command::READ_VERSION;
        pReply->len = sizeof(ReplyPacket) + sizeof(PassThruReadVersionReply);

        PassThruReadVersionReply* readVersionReply = (PassThruReadVersionReply*)(pReply->data);

        pReply->result = PassThruReadVersion(*deviceID,
            readVersionReply->firmwareVersion,
            readVersionReply->dllVersion,
            readVersionReply->apiVersion);


        if (pReply->result == RETURN_STATUS::STATUS_NOERROR) {
            if (verbose) {
                fprintf(stdout, "Firmware Version: %s\nDLL Version: %s\nAPI Version: %s\n",
                    readVersionReply->firmwareVersion,
                    readVersionReply->dllVersion,
                    readVersionReply->apiVersion);
            }
        }
        else {
            memset(readVersionReply->firmwareVersion, 0, 80);
            memset(readVersionReply->dllVersion, 0, 80);
            memset(readVersionReply->apiVersion, 0, 80);
        }

    }
    return pReply;
}

ReplyPacket* Server_PassThruStartMsgFilter(PassThruStartMsgFilterCmd* msgFilterCmd) {
    ReplyPacket* pReply = NULL;

    if (verbose)
        fprintf(stdout, "PassThruStartMsgFilter(channelID = %d, filterType = %d)\n", msgFilterCmd->channelID, msgFilterCmd->filterType);

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket) + sizeof(PassThruStartMsgFilterReply));

    if (pReply != NULL) {
        memset(pReply->data, 0, sizeof(PassThruStartMsgFilterReply));
        pReply->cmd = J2534_Command::START_MSG_FILTER;
        pReply->len = sizeof(ReplyPacket) + sizeof(PassThruStartMsgFilterReply);
        if (msgFilterCmd->filterType == Filter::FLOW_CONTROL_FILTER) {

            if (verbose) {
                fprintf(stdout, "maskMsg\n");
                printPassThruMsg(&(msgFilterCmd->maskMsg));
                fprintf(stdout, "patternMsg\n");
                printPassThruMsg(&(msgFilterCmd->patternMsg));
                fprintf(stdout, "flowControlMsg\n");
                printPassThruMsg(&(msgFilterCmd->flowControlMsg));
            }

            pReply->result = PassThruStartMsgFilter(msgFilterCmd->channelID, msgFilterCmd->filterType,
                &(msgFilterCmd->maskMsg), &(msgFilterCmd->patternMsg), &(msgFilterCmd->flowControlMsg),
                (uint32_t*)(pReply->data));
        }
        else {
            if (verbose) {
                fprintf(stdout, "maskMsg\n");
                printPassThruMsg(&(msgFilterCmd->maskMsg));
                fprintf(stdout, "patternMsg\n");
                printPassThruMsg(&(msgFilterCmd->patternMsg));
            }
            pReply->result = PassThruStartMsgFilter(msgFilterCmd->channelID, msgFilterCmd->filterType,
                &(msgFilterCmd->maskMsg), &(msgFilterCmd->patternMsg), NULL,
                (uint32_t*)(pReply->data));
        }

    }
    return pReply;
}

ReplyPacket* Server_PassThruStopMsgFilter(PassThruStopMsgFilterCmd* msgFilterCmd) {
    ReplyPacket* pReply = NULL;
    fprintf(stdout, "PassThruStopMsgFilter(%d, %d)\n", msgFilterCmd->channelID, msgFilterCmd->msgID);

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket));

    if (pReply != NULL) {
        pReply->cmd = J2534_Command::STOP_MSG_FILTER;
        pReply->len = sizeof(ReplyPacket);
        pReply->result = PassThruStopMsgFilter(msgFilterCmd->channelID, msgFilterCmd->msgID);
    }
    return pReply;
}

ReplyPacket* Server_PassThruSetProgrammingVoltage(PassThruSetProgrammingVoltageCmd* prgCmd) {
    ReplyPacket* pReply = NULL;

    if (verbose)
        fprintf(stdout, "PassThruSetProgrammingVoltage(%d, %d, %d)\n", prgCmd->deviceID, prgCmd->pinNumber, prgCmd->voltage);

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket));

    if (pReply != NULL) {
        pReply->cmd = J2534_Command::SET_PROGRAMMING_VOLTAGE;
        pReply->len = sizeof(ReplyPacket);
        pReply->result = PassThruSetProgrammingVoltage(prgCmd->deviceID, prgCmd->pinNumber, prgCmd->voltage);
    }
    return pReply;
}

ReplyPacket* Server_PassThruGetLastError() {
    ReplyPacket* pReply = NULL;

    if (verbose)
        fprintf(stdout, "PassThruGetLastError()\n");

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket) + sizeof(PassThruGetLastErrorReply));

    if (pReply != NULL) {
        memset(pReply->data, 0, sizeof(PassThruGetLastErrorReply));
        pReply->cmd = J2534_Command::GET_LAST_ERROR;
        pReply->len = sizeof(ReplyPacket) + sizeof(PassThruGetLastErrorReply);
        pReply->result = PassThruGetLastError((char*)(pReply->data));
        if (verbose)
            fprintf(stdout, "error = %s\n", (char*)(pReply->data));
    }
    return pReply;
}

ReplyPacket* Server_PassThruGetLastSocketError() {
    ReplyPacket* pReply = NULL;

    if (verbose)
        fprintf(stdout, "PassThruGetLastSocketError()\n");

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket) + sizeof(PassThruGetLastSocketErrorReply));

    if (pReply != NULL) {
        memset(pReply->data, 0, sizeof(PassThruGetLastSocketErrorReply));
        pReply->cmd = J2534_Command::GET_LAST_SOCKET_ERROR;
        pReply->len = sizeof(ReplyPacket) + sizeof(PassThruGetLastSocketErrorReply);
        pReply->result = PassThruGetLastSocketError((char*)(pReply->data));
        if (verbose)
            fprintf(stdout, "error = %s\n", (char*)(pReply->data));
    }
    return pReply;
}

ReplyPacket* Server_PassThruStartPeriodicMsg(PassThruStartPeriodicMsgCmd* msgCmd) {
    ReplyPacket* pReply = NULL;

    if (verbose)
        fprintf(stdout, "PassThruStartPeriodicMsg(%d, %d)\n", msgCmd->channelID, msgCmd->timeInterval);

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket) +
        sizeof(PassThruStartPeriodicMsgReply));

    if (pReply != NULL) {
        memset(pReply->data, 0, sizeof(PassThruStartPeriodicMsgReply));
        pReply->cmd = J2534_Command::START_PERIODIC_MSG;
        pReply->len = sizeof(ReplyPacket) + sizeof(PassThruStartPeriodicMsgReply);
        pReply->result = PassThruStartPeriodicMsg(msgCmd->channelID, &(msgCmd->msg), (uint32_t*)(pReply->data), msgCmd->timeInterval);
        if (verbose)
            fprintf(stdout, "msgID = %d\n", *(uint32_t*)(pReply->data));
    }
    return pReply;
}

ReplyPacket* Server_PassThruStopPeriodicMsg(PassThruStopPeriodicMsgCmd* msgCmd) {
    ReplyPacket* pReply = NULL;

    if (verbose)
        fprintf(stdout, "PassThruStopPeriodicMsg(%d, %d)\n", msgCmd->channelID, msgCmd->msgID);

    pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket));

    if (pReply != NULL) {
        pReply->cmd = J2534_Command::STOP_PERIODIC_MSG;
        pReply->len = sizeof(ReplyPacket);
        pReply->result = PassThruStopPeriodicMsg(msgCmd->channelID, msgCmd->msgID);
    }
    return pReply;
}

ReplyPacket* Server_PassThruIoctl(PassThruIoctlCmd* pData) {
    ReplyPacket* pReply = NULL;
    void* pInput = NULL;
    void* pOutput = NULL;
    SCONFIG_LIST inputSCL;
    SBYTE_ARRAY inputSBA;
    SBYTE_ARRAY outputSBA;

    if (verbose)
        fprintf(stdout, "PassThruIoctl(%d, %d)\n", pData->channelID, pData->ioctlID);

    switch (pData->ioctlID) {
        case IOCTL_ID::GET_CONFIG:
            //pInput = SCONFIG_LIST*
            //pOutput = NULL
            inputSCL.NumOfParams = *(uint32_t*)(pData->input);
            inputSCL.ConfigPtr = (SCONFIG*)(pData->input + sizeof(inputSCL.NumOfParams));
            pInput = &inputSCL;
            pOutput = NULL;
            pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket));
            pReply->len = sizeof(ReplyPacket) + sizeof(inputSCL.NumOfParams) +
                inputSCL.NumOfParams * sizeof(SCONFIG);
            break;
        case IOCTL_ID::SET_CONFIG:
            //pInput = SCONFIG_LIST*
            //pOutput = NULL
            inputSCL.NumOfParams = *(uint32_t*)(pData->input);
            inputSCL.ConfigPtr = (SCONFIG*)(pData->input + sizeof(inputSCL.NumOfParams));
            pInput = &inputSCL;
            pOutput = NULL;
            pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket));
            pReply->len = sizeof(ReplyPacket);
            if (verbose) {
                fprintf(stdout, "pInput:\n");
                printSCONFIG_LIST(&inputSCL);
            }
            break;
        case IOCTL_ID::READ_VBATT:
        case IOCTL_ID::READ_PROG_VOLTAGE:
            //pInput = NULL;
            //pOutput = uint32_t*
            pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket) + sizeof(uint32_t));
            pReply->len = sizeof(ReplyPacket) + sizeof(uint32_t);
            memset(pReply->data, 0, sizeof(uint32_t));
            pInput = NULL;
            pOutput = pReply->data;
            break;
        case IOCTL_ID::FIVE_BAUD_INIT:
            //pInput = SBYTE_ARRAY*
            //pOutput = SBYTE_ARRAY* Output always contains a 2 byte long SBYTE_ARRAY
            inputSBA.NumOfBytes = *(uint32_t*)(pData->input);
            inputSBA.BytePtr = (uint8_t*)(pData->input + sizeof(inputSBA.NumOfBytes));
            pInput = &inputSBA;
            pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket) + sizeof(uint32_t) + 2);
            pReply->len = sizeof(ReplyPacket) + sizeof(uint32_t) + 2;
            memset(pReply->data, 0, sizeof(uint32_t) + 2);
            outputSBA.NumOfBytes = 2;
            outputSBA.BytePtr = (uint8_t*)(pReply->data + sizeof(outputSBA.NumOfBytes));
            pOutput = &outputSBA;
            break;
        case IOCTL_ID::FAST_INIT:
            //pInput = PASSTHRU_MSG*
            //pOutput = PASSTHRU_MSG*
            pInput = pData->input;
            pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket) + sizeof(PASSTHRU_MSG));
            pReply->len = sizeof(ReplyPacket) + sizeof(PASSTHRU_MSG);
            memset(pReply->data, 0, sizeof(PASSTHRU_MSG));
            pOutput = pReply->data;
            break;
        case IOCTL_ID::CLEAR_TX_BUFFER:
        case IOCTL_ID::CLEAR_RX_BUFFER:
        case IOCTL_ID::CLEAR_PERIODIC_MSGS:
        case IOCTL_ID::CLEAR_MSG_FILTERS:
        case IOCTL_ID::CLEAR_FUNCT_MSG_LOOKUP_TABLE:
            //pInput = NULL;
            //pOutput = NULL;
            pInput = NULL;
            pOutput = NULL;
            pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket));
            pReply->len = sizeof(ReplyPacket);
            break;
        case IOCTL_ID::ADD_TO_FUNCT_MSG_LOOKUP_TABLE:
        case IOCTL_ID::DELETE_FROM_FUNCT_MSG_LOOKUP_TABLE:
            //pInput = SBYTE_ARRAY*;
            //pOutput = NULL;
            inputSBA.NumOfBytes = *(uint32_t*)(pData->input);
            inputSBA.BytePtr = (uint8_t*)(pData->input + sizeof(inputSBA.NumOfBytes));
            pInput = &inputSBA;
            pOutput = NULL;
            pReply = (ReplyPacket*)malloc(sizeof(ReplyPacket));
            pReply->len = sizeof(ReplyPacket);
            break;
        default:
            return NULL;
    }
    pReply->cmd = J2534_Command::IOCTL;
    pReply->result = PassThruIoctl(pData->channelID, pData->ioctlID, pInput, pOutput);

    if (pData->ioctlID == IOCTL_ID::GET_CONFIG) {
        //Need to copy the input data to the pReply data
        memcpy(pReply->data, &inputSCL, sizeof(inputSCL.NumOfParams));
        memcpy(pReply->data + sizeof(inputSCL.NumOfParams),
            inputSCL.ConfigPtr, pReply->len - sizeof(inputSCL.NumOfParams) - sizeof(ReplyPacket));
    }
    else if (pData->ioctlID == IOCTL_ID::FIVE_BAUD_INIT) {
        //Need to copy the output data to the pReply data
        memcpy(pReply->data, &outputSBA, sizeof(outputSBA.NumOfBytes));
        memcpy(pReply->data + sizeof(outputSBA.NumOfBytes),
            outputSBA.BytePtr, outputSBA.NumOfBytes);
    }

    return pReply;
}


ReplyPacket* processCommand(CommandPacket* cmd) {

    switch (cmd->cmd) {
        case J2534_Command::OPEN:
            return Server_PassThruOpen((PassThruOpenCmd*)(cmd->data));
        case J2534_Command::CONNECT:
            return Server_PassThruConnect((PassThruConnectCmd*)(cmd->data));
        case J2534_Command::READ_MSGS:
            return Server_PassThruReadMsgs((PassThruReadMsgsCmd*)(cmd->data));
        case J2534_Command::WRITE_MSGS:
            return Server_PassThruWriteMsgs((PassThruWriteMsgsCmd*)(cmd->data));
        case J2534_Command::IOCTL:
            return Server_PassThruIoctl((PassThruIoctlCmd*)(cmd->data));
        case J2534_Command::START_PERIODIC_MSG:
            return Server_PassThruStartPeriodicMsg((PassThruStartPeriodicMsgCmd*)(cmd->data));
        case J2534_Command::STOP_PERIODIC_MSG:
            return Server_PassThruStopPeriodicMsg((PassThruStopPeriodicMsgCmd*)(cmd->data));
        case J2534_Command::START_MSG_FILTER:
            return Server_PassThruStartMsgFilter((PassThruStartMsgFilterCmd*)(cmd->data));
        case J2534_Command::STOP_MSG_FILTER:
            return Server_PassThruStopMsgFilter((PassThruStopMsgFilterCmd*)(cmd->data));
        case J2534_Command::SET_PROGRAMMING_VOLTAGE:
            return Server_PassThruSetProgrammingVoltage((PassThruSetProgrammingVoltageCmd*)(cmd->data));
        case J2534_Command::READ_VERSION:
            return Server_PassThruReadVersion((PassThruReadVersionCmd*)(cmd->data));
        case J2534_Command::DISCONNECT:
            return Server_PassThruDisconnect((PassThruDisconnectCmd*)(cmd->data));
        case J2534_Command::CLOSE:
            return Server_PassThruClose((PassThruCloseCmd*)(cmd->data));
        case J2534_Command::RESET:
            return Server_PassThruReset();
        case J2534_Command::GET_LAST_ERROR:
            return Server_PassThruGetLastError();
        case J2534_Command::GET_LAST_SOCKET_ERROR:
            return Server_PassThruGetLastSocketError();
        default:
            fprintf(stderr, "Got Unknown Command %d\n", cmd->cmd);
            break;
    }

    return NULL;
}
