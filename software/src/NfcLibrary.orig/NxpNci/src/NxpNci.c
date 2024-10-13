/*
 *         Copyright (c), NXP Semiconductors Caen / France
 *
 *                     (C)NXP Semiconductors
 *       All rights are reserved. Reproduction in whole or in part is
 *      prohibited without the written consent of the copyright owner.
 *  NXP reserves the right to make changes without notice at any time.
 * NXP makes no warranty, expressed, implied or statutory, including but
 * not limited to any implied warranty of merchantability or fitness for any
 *particular purpose, or that the use will not infringe any third party patent,
 * copyright or trademark. NXP must not be liable for any loss or damage
 *                          arising from its use.
 */

#include <tool.h>
#include <tml.h>
#include <NxpNci.h>
#include <Nfc_settings.h>

#define MAX_NCI_FRAME_SIZE    258

static bool gRfSettingsRestored_flag = false;
static uint8_t gNfcController_generation = 0;
static uint8_t gNfcController_fw_version[3] = {0};
static uint8_t gNextTag_Protocol = PROT_UNDETERMINED;

static uint8_t NCIStartDiscovery[30];
static uint8_t NCIStartDiscovery_length = 0;


static bool NxpNci_CheckDevPres(void)
{
    uint8_t NCICoreReset[] = {0x20, 0x00, 0x01, 0x01};
    uint8_t Answer[6];
    uint16_t NbBytes = 0;

    /* Reset RF settings restore flag */
    gRfSettingsRestored_flag = false;

    tml_Send(NCICoreReset, sizeof(NCICoreReset), &NbBytes);
    NCI_PRINT_BUF("NCI >> ", NCICoreReset, NbBytes);
    if (NbBytes != sizeof(NCICoreReset)) return NXPNCI_ERROR;
    tml_Receive(Answer, sizeof(Answer), &NbBytes, TIMEOUT_100MS);
    NCI_PRINT_BUF("NCI << ", Answer, NbBytes);
    if ((NbBytes == 0) || (Answer[0] != 0x40) || (Answer[1] != 0x00)) return NXPNCI_ERROR;

    /* Catch potential notifications */
    tml_Receive(Answer, sizeof(Answer), &NbBytes, TIMEOUT_100MS);
    if (NbBytes != 0)
    {
        NCI_PRINT_BUF("NCI << ", Answer, NbBytes);
        /* Is CORE_GENERIC_ERROR_NTF ? */
        if ((Answer[0] == 0x60) && (Answer[1] == 0x07))
        {
            /* Is PN7150B0HN/C11004 Anti-tearing recovery procedure triggered ? */
            if ((Answer[3] == 0xE6)) gRfSettingsRestored_flag = true;
        }
        else
        {
            return NXPNCI_ERROR;
        }
    }

    return NXPNCI_SUCCESS;
}

static bool NxpNci_WaitForReception(uint8_t *pRBuff, uint16_t RBuffSize, uint16_t *pBytesread, uint16_t timeout)
{
    tml_Receive(pRBuff, RBuffSize, pBytesread, timeout);
    if (*pBytesread == 0) return NXPNCI_ERROR;
    NCI_PRINT_BUF("NCI << ", pRBuff, *pBytesread);

    return NXPNCI_SUCCESS;
}

static bool NxpNci_HostTransceive(uint8_t *pTBuff, uint16_t TbuffLen, uint8_t *pRBuff, uint16_t RBuffSize, uint16_t *pBytesread)
{
    tml_Send(pTBuff, TbuffLen, pBytesread);
    NCI_PRINT_BUF("NCI >> ", pTBuff, TbuffLen);
    if (*pBytesread != TbuffLen) return NXPNCI_ERROR;
    tml_Receive(pRBuff, RBuffSize, pBytesread, TIMEOUT_1S);
    if (*pBytesread == 0) return NXPNCI_ERROR;
    NCI_PRINT_BUF("NCI << ", pRBuff, *pBytesread);

    return NXPNCI_SUCCESS;
}

static void NxpNci_FillInterfaceInfo(NxpNci_RfIntf_t* pRfIntf, uint8_t* pBuf)
{
    uint8_t i, temp;

    switch(pRfIntf->ModeTech)
    {
    case (MODE_POLL | TECH_PASSIVE_NFCA):
        memcpy(pRfIntf->Info.NFC_APP.SensRes, &pBuf[0], 2);
        temp = 2;
        pRfIntf->Info.NFC_APP.NfcIdLen = pBuf[temp];
        temp++;
        memcpy(pRfIntf->Info.NFC_APP.NfcId, &pBuf[3], pRfIntf->Info.NFC_APP.NfcIdLen);
        temp+=pBuf[2];
        pRfIntf->Info.NFC_APP.SelResLen = pBuf[temp];
        temp++;
        if(pRfIntf->Info.NFC_APP.SelResLen == 1) pRfIntf->Info.NFC_APP.SelRes[0] = pBuf[temp];
        temp+=4;
        if(pBuf[temp] != 0)
        {
            temp++;
            pRfIntf->Info.NFC_APP.RatsLen = pBuf[temp];
            memcpy(pRfIntf->Info.NFC_APP.Rats, &pBuf[temp+1], pBuf[temp]);
        }
        else
        {
            pRfIntf->Info.NFC_APP.RatsLen = 0;
        }
        break;

    case (MODE_POLL | TECH_PASSIVE_NFCB):
        pRfIntf->Info.NFC_BPP.SensResLen = pBuf[0];
        memcpy(pRfIntf->Info.NFC_BPP.SensRes, &pBuf[1], pRfIntf->Info.NFC_BPP.SensResLen);
        temp = pBuf[0] + 4;
        if(pBuf[temp] != 0)
        {
            temp++;
            pRfIntf->Info.NFC_BPP.AttribResLen = pBuf[temp];
            memcpy(pRfIntf->Info.NFC_BPP.AttribRes, &pBuf[temp+1], pBuf[temp]);
        }
        else
        {
            pRfIntf->Info.NFC_BPP.AttribResLen = 0;
        }
        break;

    case (MODE_POLL | TECH_PASSIVE_NFCF):
        pRfIntf->Info.NFC_FPP.BitRate = pBuf[0];
        pRfIntf->Info.NFC_FPP.SensResLen = pBuf[1];
        memcpy(pRfIntf->Info.NFC_FPP.SensRes, &pBuf[2], pRfIntf->Info.NFC_FPP.SensResLen);
        break;

    case (MODE_POLL | TECH_PASSIVE_15693):
        pRfIntf->Info.NFC_VPP.AFI = pBuf[0];
        pRfIntf->Info.NFC_VPP.DSFID = pBuf[1];
        for(i=0; i<8; i++) pRfIntf->Info.NFC_VPP.ID[7-i] = pBuf[2+i];
        break;

    default:
        break;
    }
}

#ifdef CARDEMU_SUPPORT
void NxpNci_ProcessCardMode(NxpNci_RfIntf_t RfIntf)
{
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;
    uint8_t NCIStopDiscovery[] = {0x21, 0x06, 0x01, 0x00};
    bool FirstCmd = true;

    /* Reset Card emulation state */
    T4T_NDEF_EMU_Reset();

    while(NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_2S) == NXPNCI_SUCCESS)
    {
    	/* is RF_DEACTIVATE_NTF ? */
        if((Answer[0] == 0x61) && (Answer[1] == 0x06))
        {
            if(FirstCmd)
            {
            	/* Restart the discovery loop */
				NxpNci_HostTransceive(NCIStopDiscovery, sizeof(NCIStopDiscovery), Answer, sizeof(Answer), &AnswerSize);
				do
				{
					if ((Answer[0] == 0x41) && (Answer[1] == 0x06)) break;
					NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
				} while (AnswerSize != 0);
				NxpNci_HostTransceive(NCIStartDiscovery, NCIStartDiscovery_length, Answer, sizeof(Answer), &AnswerSize);
            }
            /* Come back to discovery state */
            break;
        }
        /* is DATA_PACKET ? */
        else if((Answer[0] == 0x00) && (Answer[1] == 0x00))
        {
            /* DATA_PACKET */
            uint8_t Cmd[MAX_NCI_FRAME_SIZE];
            uint16_t CmdSize;

            T4T_NDEF_EMU_Next(&Answer[3], Answer[2], &Cmd[3], (unsigned short *) &CmdSize);

            Cmd[0] = 0x00;
            Cmd[1] = (CmdSize & 0xFF00) >> 8;
            Cmd[2] = CmdSize & 0x00FF;

            NxpNci_HostTransceive(Cmd, CmdSize+3, Answer, sizeof(Answer), &AnswerSize);
        }
        FirstCmd = false;
    }
}

bool NxpNci_CardModeReceive (unsigned char *pData, unsigned char *pDataSize)
{
    bool status = NXPNCI_ERROR;
    uint8_t Ans[MAX_NCI_FRAME_SIZE];
    uint16_t AnsSize;

    NxpNci_WaitForReception(Ans, sizeof(Ans), &AnsSize, TIMEOUT_2S);

    /* Is data packet ? */
    if ((Ans[0] == 0x00) && (Ans[1] == 0x00))
    {
        *pDataSize = Ans[2];
        memcpy(pData, &Ans[3], *pDataSize);
        status = NXPNCI_SUCCESS;
    }

    return status;
}

bool NxpNci_CardModeSend (unsigned char *pData, unsigned char DataSize)
{
    bool status;
    uint8_t Cmd[MAX_NCI_FRAME_SIZE];
    uint8_t Ans[MAX_NCI_FRAME_SIZE];
    uint16_t AnsSize;

    /* Compute and send DATA_PACKET */
    Cmd[0] = 0x00;
    Cmd[1] = 0x00;
    Cmd[2] = DataSize;
    memcpy(&Cmd[3], pData, DataSize);
    //status = NxpNci_HostSend(Cmd, DataSize+3);
    status = NxpNci_HostTransceive(Cmd, DataSize+3, Ans, sizeof(Ans), &AnsSize);

    return status;
}
#endif

#ifdef P2P_SUPPORT
void NxpNci_ProcessP2pMode(NxpNci_RfIntf_t RfIntf)
{
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;
    bool status;
    bool restart = false;
    uint8_t NCILlcpSymm[] = {0x00, 0x00, 0x02, 0x00, 0x00};
    uint8_t NCIRestartDiscovery[] = {0x21, 0x06, 0x01, 0x03};

    /* Reset P2P_NDEF state */
    P2P_NDEF_Reset();

    /* Is Initiator mode ? */
    if((RfIntf.ModeTech & MODE_LISTEN) != MODE_LISTEN)
    {
        /* Initiate communication (SYMM PDU) */
        NxpNci_HostTransceive(NCILlcpSymm, sizeof(NCILlcpSymm), Answer, sizeof(Answer), &AnswerSize);
        /* Save status for discovery restart */
        restart = true;
    }

    status = NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_2S);

    /* Get frame from remote peer */
    while(status == NXPNCI_SUCCESS)
    {
    	/* is DATA_PACKET ? */
        if((Answer[0] == 0x00) && (Answer[1] == 0x00))
        {
            uint8_t Cmd[MAX_NCI_FRAME_SIZE];
            uint16_t CmdSize;
            /* Handle P2P communication */
            P2P_NDEF_Next(&Answer[3], Answer[2], &Cmd[3], (unsigned short *) &CmdSize);
            /* Compute DATA_PACKET to answer */
            Cmd[0] = 0x00;
            Cmd[1] = (CmdSize & 0xFF00) >> 8;
            Cmd[2] = CmdSize & 0x00FF;
            status = NxpNci_HostTransceive(Cmd, CmdSize+3, Answer, sizeof(Answer), &AnswerSize);
        }
        /* is CORE_INTERFACE_ERROR_NTF ?*/
        else if ((Answer[0] == 0x60) && (Answer[1] == 0x08))
        {
            /* Come back to discovery state */
            break;
        }
        /* is RF_DEACTIVATE_NTF ? */
        else if((Answer[0] == 0x61) && (Answer[1] == 0x06))
        {
            /* Come back to discovery state */
            break;
        }
        /* is RF_DISCOVERY_NTF ? */
        else if((Answer[0] == 0x61) && ((Answer[1] == 0x05) || (Answer[1] == 0x03)))
        {
            do{
                if((Answer[0] == 0x61) && ((Answer[1] == 0x05) || (Answer[1] == 0x03)))
                {
                    if((Answer[6] & MODE_LISTEN) != MODE_LISTEN) restart = true;
                    else restart = false;
                }
                status = NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
            }
            while (AnswerSize != 0);
            /* Come back to discovery state */
            break;
        }

        /* Wait for next frame from remote P2P, or notification event */
        status = NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_2S);
    }

    /* Is Initiator mode ? */
    if(restart)
    {
        /* Communication ended, restart discovery loop */
        NxpNci_HostTransceive(NCIRestartDiscovery, sizeof(NCIRestartDiscovery), Answer, sizeof(Answer), &AnswerSize);
        NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
    }
}
#endif

#ifdef RW_SUPPORT
bool NxpNci_ReaderTagCmd (unsigned char *pCommand, unsigned char CommandSize, unsigned char *pAnswer, unsigned char *pAnswerSize)
{
    bool status = NXPNCI_ERROR;
    uint8_t Cmd[MAX_NCI_FRAME_SIZE];
    uint8_t Ans[MAX_NCI_FRAME_SIZE];
    uint16_t AnsSize;

    /* Compute and send DATA_PACKET */
    Cmd[0] = 0x00;
    Cmd[1] = 0x00;
    Cmd[2] = CommandSize;
    memcpy(&Cmd[3], pCommand, CommandSize);
    NxpNci_HostTransceive(Cmd, CommandSize+3, Ans, sizeof(Ans), &AnsSize);

    /* Wait for Answer */
    NxpNci_WaitForReception(Ans, sizeof(Ans), &AnsSize, TIMEOUT_1S);

    if ((Ans[0] == 0x0) && (Ans[1] == 0x0))
    {
        status = NXPNCI_SUCCESS;
    }

    *pAnswerSize = Ans[2];
    memcpy(pAnswer, &Ans[3], *pAnswerSize);

    return status;
}

#ifndef NO_NDEF_SUPPORT
static void NxpNci_ReadNdef(NxpNci_RfIntf_t RfIntf)
{
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;
    uint8_t Cmd[MAX_NCI_FRAME_SIZE];
    uint16_t CmdSize = 0;

    RW_NDEF_Reset(RfIntf.Protocol);

    while(1)
    {
        RW_NDEF_Read_Next(&Answer[3], Answer[2], &Cmd[3], (unsigned short *) &CmdSize);
        if(CmdSize == 0)
        {
            /* End of the Read operation */
            break;
        }
        else
        {
            /* Compute and send DATA_PACKET */
            Cmd[0] = 0x00;
            Cmd[1] = (CmdSize & 0xFF00) >> 8;
            Cmd[2] = CmdSize & 0x00FF;

            NxpNci_HostTransceive(Cmd, CmdSize+3, Answer, sizeof(Answer), &AnswerSize);
            NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_1S);
            
            /* Manage chaining in case of T4T */
            if((RfIntf.Interface = INTF_ISODEP) && Answer[0] == 0x10)
            {
                uint8_t tmp[MAX_NCI_FRAME_SIZE];
                uint8_t tmpSize = 0;
                while(Answer[0] == 0x10)
                {
                    memcpy(&tmp[tmpSize], &Answer[3], Answer[2]);
                    tmpSize += Answer[2];
                    NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
                }
                memcpy(&tmp[tmpSize], &Answer[3], Answer[2]);
                tmpSize += Answer[2];
                /* Compute all chained frame into one unique answer */
                memcpy(&Answer[3], tmp, tmpSize);
                Answer[2] = tmpSize;
            }
        }
    }
}

static void NxpNci_WriteNdef(NxpNci_RfIntf_t RfIntf)
{
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;
    uint8_t Cmd[MAX_NCI_FRAME_SIZE];
    uint16_t CmdSize = 0;

    RW_NDEF_Reset(RfIntf.Protocol);

    while(1)
    {
        RW_NDEF_Write_Next(&Answer[3], Answer[2], &Cmd[3], (unsigned short *) &CmdSize);
        if(CmdSize == 0)
        {
            /* End of the Write operation */
            break;
        }
        else
        {
            /* Compute and send DATA_PACKET */
            Cmd[0] = 0x00;
            Cmd[1] = (CmdSize & 0xFF00) >> 8;
            Cmd[2] = CmdSize & 0x00FF;

            NxpNci_HostTransceive(Cmd, CmdSize+3, Answer, sizeof(Answer), &AnswerSize);
            NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_2S);
        }
    }
}
#endif

static void NxpNci_PresenceCheck(NxpNci_RfIntf_t RfIntf)
{
    bool status;
    uint8_t i;
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;

    uint8_t NCIPresCheckT1T[] = {0x00, 0x00, 0x07, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t NCIPresCheckT2T[] = {0x00, 0x00, 0x02, 0x30, 0x00};
    uint8_t NCIPresCheckT3T[] = {0x21, 0x08, 0x04, 0xFF, 0xFF, 0x00, 0x01};
    uint8_t NCIPresCheckIsoDep[] = {0x2F, 0x11, 0x00};
    uint8_t NCIPresCheckIso15693[] = {0x00, 0x00, 0x0B, 0x26, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t NCIDeactivate[] = {0x21, 0x06, 0x01, 0x01};
    uint8_t NCISelectMIFARE[] = {0x21, 0x04, 0x03, 0x01, 0x80, 0x80};

    switch (RfIntf.Protocol) {
    case PROT_T1T:
        do
        {
            Sleep(500);
            NxpNci_HostTransceive(NCIPresCheckT1T, sizeof(NCIPresCheckT1T), Answer, sizeof(Answer), &AnswerSize);
            NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
        } while ((Answer[0] == 0x00) && (Answer[1] == 0x00));
        break;

    case PROT_T2T:
        do
        {
            Sleep(500);
            NxpNci_HostTransceive(NCIPresCheckT2T, sizeof(NCIPresCheckT2T), Answer, sizeof(Answer), &AnswerSize);
            NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
        } while ((Answer[0] == 0x00) && (Answer[1] == 0x00) && (Answer[2] == 0x11));
        break;

    case PROT_T3T:
        do
        {
            Sleep(500);
            NxpNci_HostTransceive(NCIPresCheckT3T, sizeof(NCIPresCheckT3T), Answer, sizeof(Answer), &AnswerSize);
            NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
        } while ((Answer[0] == 0x61) && (Answer[1] == 0x08) && ((Answer[3] == 0x00) || (Answer[4] > 0x00)));
        break;

    case PROT_ISODEP:
        do
        {
            Sleep(500);
            NxpNci_HostTransceive(NCIPresCheckIsoDep, sizeof(NCIPresCheckIsoDep), Answer, sizeof(Answer), &AnswerSize);
            NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
        } while ((Answer[0] == 0x6F) && (Answer[1] == 0x11) && (Answer[2] == 0x01) && (Answer[3] == 0x01));
        break;

    case PROT_T5T:
        do
        {
            Sleep(500);
            for(i=0; i<8; i++) NCIPresCheckIso15693[i+6] = RfIntf.Info.NFC_VPP.ID[7-i];
            NxpNci_HostTransceive(NCIPresCheckIso15693, sizeof(NCIPresCheckIso15693), Answer, sizeof(Answer), &AnswerSize);
            status = NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
        } while ((status == NXPNCI_SUCCESS) && (Answer[0] == 0x00) && (Answer[1] == 0x00) && (Answer[AnswerSize-1] == 0x00));
        break;

    case PROT_MIFARE:
        do
        {
            Sleep(500);
            /* Deactivate target */
            NxpNci_HostTransceive(NCIDeactivate, sizeof(NCIDeactivate), Answer, sizeof(Answer), &AnswerSize);
            NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
            /* Reactivate target */
            NxpNci_HostTransceive(NCISelectMIFARE, sizeof(NCISelectMIFARE), Answer, sizeof(Answer), &AnswerSize);
            NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
        } while ((Answer[0] == 0x61) && (Answer[1] == 0x05));
        break;

    default:
        /* Nothing to do */
        break;
    }
}

void NxpNci_ProcessReaderMode(NxpNci_RfIntf_t RfIntf, NxpNci_RW_Operation_t Operation)
{
#ifndef NO_NDEF_SUPPORT
	if(RfIntf.Protocol == PROT_T3T)
	{
		RW_NDEF_T3T_SetIDm(RfIntf.Info.NFC_FPP.SensRes);
	}
#endif

    switch (Operation)
    {
#ifndef NO_NDEF_SUPPORT
    case READ_NDEF:
        NxpNci_ReadNdef(RfIntf);
        break;

    case WRITE_NDEF:
        NxpNci_WriteNdef(RfIntf);
        break;
#endif
    case PRESENCE_CHECK:
        NxpNci_PresenceCheck(RfIntf);
        break;
    default:
        break;
    }
}

bool NxpNci_ReaderActivateNext(NxpNci_RfIntf_t *pRfIntf)
{
    uint8_t NCIStopDiscovery[] = {0x21, 0x06, 0x01, 0x01};
    uint8_t NCIRfDiscoverSelect[] = {0x21, 0x04, 0x03, 0x02, PROT_ISODEP, INTF_ISODEP};
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;
    bool status = NXPNCI_ERROR;

    pRfIntf->MoreTags = false;

    if (gNextTag_Protocol == PROT_UNDETERMINED)
    {
        pRfIntf->Interface = INTF_UNDETERMINED;
        pRfIntf->Protocol = PROT_UNDETERMINED;
        return NXPNCI_ERROR;
    }

    /* First disconnect current tag */
    NxpNci_HostTransceive(NCIStopDiscovery, sizeof(NCIStopDiscovery), Answer, sizeof(Answer), &AnswerSize);
    if((Answer[0] != 0x41) && (Answer[1] != 0x06) && (Answer[3] != 0x00)) return NXPNCI_ERROR;
    NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
    if((Answer[0] != 0x61) && (Answer[1] != 0x06)) return NXPNCI_ERROR;

    NCIRfDiscoverSelect[4] = gNextTag_Protocol;
    if (gNextTag_Protocol == PROT_ISODEP) NCIRfDiscoverSelect[5] = INTF_ISODEP;
    else if (gNextTag_Protocol == PROT_ISODEP) NCIRfDiscoverSelect[5] = INTF_NFCDEP;
    else if (gNextTag_Protocol == PROT_MIFARE) NCIRfDiscoverSelect[5] = INTF_TAGCMD;
    else NCIRfDiscoverSelect[5] = INTF_FRAME;
    NxpNci_HostTransceive(NCIRfDiscoverSelect, sizeof(NCIRfDiscoverSelect), Answer, sizeof(Answer), &AnswerSize);
    if ((Answer[0] == 0x41) && (Answer[1] == 0x04) && (Answer[3] == 0x00))
    {
        NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
        if ((Answer[0] == 0x61) || (Answer[1] == 0x05))
        {
            pRfIntf->Interface = Answer[4];
            pRfIntf->Protocol = Answer[5];
            pRfIntf->ModeTech = Answer[6];
            NxpNci_FillInterfaceInfo(pRfIntf, &Answer[10]);
            status = NXPNCI_SUCCESS;
        }
    }

    return status;
}

bool NxpNci_ReaderReActivate(NxpNci_RfIntf_t *pRfIntf)
{
    uint8_t NCIDeactivate[] = {0x21, 0x06, 0x01, 0x01};
    uint8_t NCIActivate[] = {0x21, 0x04, 0x03, 0x01, 0x00, 0x00};
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;

    /* First de-activate the target */
    NxpNci_HostTransceive(NCIDeactivate, sizeof(NCIDeactivate), Answer, sizeof(Answer), &AnswerSize);
    NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
    
    /* Then re-activate the target */
    NCIActivate[4] = pRfIntf->Protocol;
    NCIActivate[5] = pRfIntf->Interface;
    NxpNci_HostTransceive(NCIActivate, sizeof(NCIActivate), Answer, sizeof(Answer), &AnswerSize);
    NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
    if((Answer[0] != 0x61) || (Answer[1] != 0x05)) return NXPNCI_ERROR;
    return NXPNCI_SUCCESS;
}
#endif

bool NxpNci_Connect(void)
{
    uint8_t i = 2;
    uint8_t NCICoreInit[] = {0x20, 0x01, 0x00};
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;

    /* Open connection to NXPNCI */
    tml_Connect ();

    /* Loop until NXPNCI answers */
    while(NxpNci_CheckDevPres() != NXPNCI_SUCCESS)
    {
        if(i-- == 0) return NXPNCI_ERROR;
        Sleep(500);
    }

    NxpNci_HostTransceive(NCICoreInit, sizeof(NCICoreInit), Answer, sizeof(Answer), &AnswerSize);
    if ((Answer[0] != 0x40) || (Answer[1] != 0x01) || (Answer[3] != 0x00)) return NXPNCI_ERROR;

    /* Retrieve NXP-NCI NFC Controller generation */
    if (Answer[17+Answer[8]] == 0x08) gNfcController_generation = 1;
    else if (Answer[17+Answer[8]] == 0x10) gNfcController_generation = 2;

    /* Retrieve NXP-NCI NFC Controller FW version */
    gNfcController_fw_version[0] = Answer[17+Answer[8]];
    gNfcController_fw_version[1] = Answer[18+Answer[8]];
    gNfcController_fw_version[2] = Answer[19+Answer[8]];

    return NXPNCI_SUCCESS;
}

void NxpNci_GetFwVersion(unsigned char fw[3])
{
    fw[0] = gNfcController_fw_version[0];
    fw[1] = gNfcController_fw_version[1];
    fw[2] = gNfcController_fw_version[2];
}

bool NxpNci_Disconnect(void)
{
    /* Close connection to NXPNCI */
    tml_Disconnect ();

    return NXPNCI_SUCCESS;
}

bool NxpNci_ConfigureSettings(void)
{
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;
    uint8_t NCICoreReset[] = {0x20, 0x00, 0x01, 0x00};
    uint8_t NCICoreInit[] = {0x20, 0x01, 0x00};
#if (NXP_TVDD_CONF | NXP_RF_CONF)
    uint8_t *NxpNci_CONF;
    uint16_t NxpNci_CONF_size = 0;
#endif
#if (NXP_CORE_CONF_EXTN | NXP_CLK_CONF | NXP_TVDD_CONF | NXP_RF_CONF)
    uint8_t NCIReadTS[] = {0x20, 0x03, 0x03, 0x01, 0xA0, 0x14};
    uint8_t NCIWriteTS[7+32] = {0x20, 0x02, 0x24, 0x01, 0xA0, 0x14, 0x20};
#endif
    bool isResetRequired = false;

    /* Apply settings according definition of Nfc_settings.h header file */
#if NXP_CORE_CONF
    if (sizeof(NxpNci_CORE_CONF) != 0)
    {
        isResetRequired = true;
        NxpNci_HostTransceive(NxpNci_CORE_CONF, sizeof(NxpNci_CORE_CONF), Answer, sizeof(Answer), &AnswerSize);
        if ((Answer[0] != 0x40) || (Answer[1] != 0x02) || (Answer[3] != 0x00) || (Answer[4] != 0x00)) return NXPNCI_ERROR;
    }
#endif

#if NXP_CORE_STANDBY
    if (sizeof(NxpNci_CORE_STANDBY) != 0)
    {
        isResetRequired = true;
        NxpNci_HostTransceive(NxpNci_CORE_STANDBY, sizeof(NxpNci_CORE_STANDBY), Answer, sizeof(Answer), &AnswerSize);
        if ((Answer[0] != 0x4F) || (Answer[1] != 0x00) || (Answer[3] != 0x00)) return NXPNCI_ERROR;
    }
#endif

    /* All further settings are not versatile, so configuration only applied if there are changes (application build timestamp) 
       or in case of PN7150B0HN/C11004 Anti-tearing recovery procedure inducing RF setings were restored to their default value */
#if (NXP_CORE_CONF_EXTN | NXP_CLK_CONF | NXP_TVDD_CONF | NXP_RF_CONF)
    /* First read timestamp stored in NFC Controller */
    if(gNfcController_generation == 1) NCIReadTS[5] = 0x0F;
    NxpNci_HostTransceive(NCIReadTS, sizeof(NCIReadTS), Answer, sizeof(Answer), &AnswerSize);
    if ((Answer[0] != 0x40) || (Answer[1] != 0x03) || (Answer[3] != 0x00)) return NXPNCI_ERROR;
    /* Then compare with current build timestamp, and check RF settings restore flag */
    if(!memcmp(&Answer[8], NxpNci_SettingCurrentTS, sizeof(NxpNci_SettingCurrentTS)) && (gRfSettingsRestored_flag == false))
    {
        /* No change, nothing to do */
    }
    else
    {
        /* Apply settings */
#if NXP_CORE_CONF_EXTN
        if (sizeof(NxpNci_CORE_CONF_EXTN) != 0)
        {
            isResetRequired = true;
            NxpNci_HostTransceive(NxpNci_CORE_CONF_EXTN, sizeof(NxpNci_CORE_CONF_EXTN), Answer, sizeof(Answer), &AnswerSize);
            if ((Answer[0] != 0x40) || (Answer[1] != 0x02) || (Answer[3] != 0x00) || (Answer[4] != 0x00)) return NXPNCI_ERROR;
        }
#endif

#if NXP_CLK_CONF
        if (sizeof(NxpNci_CLK_CONF) != 0)
        {
            isResetRequired = true;
            NxpNci_HostTransceive(NxpNci_CLK_CONF, sizeof(NxpNci_CLK_CONF), Answer, sizeof(Answer), &AnswerSize);
            if ((Answer[0] != 0x40) || (Answer[1] != 0x02) || (Answer[3] != 0x00) || (Answer[4] != 0x00)) return NXPNCI_ERROR;
        }
#endif

#if NXP_TVDD_CONF
        NxpNci_CONF = NULL;
        if(gNfcController_generation == 1)
        {
            NxpNci_CONF = NxpNci_TVDD_CONF_1stGen;
            NxpNci_CONF_size = sizeof(NxpNci_TVDD_CONF_1stGen);
        }
        else if(gNfcController_generation == 2)
        {
            NxpNci_CONF = NxpNci_TVDD_CONF_2ndGen;
            NxpNci_CONF_size = sizeof(NxpNci_TVDD_CONF_2ndGen);
        }
        if (NxpNci_CONF != NULL)
        {
            isResetRequired = true;
            NxpNci_HostTransceive(NxpNci_CONF, NxpNci_CONF_size, Answer, sizeof(Answer), &AnswerSize);
            if ((Answer[0] != 0x40) || (Answer[1] != 0x02) || (Answer[3] != 0x00) || (Answer[4] != 0x00)) return NXPNCI_ERROR;
        }
#endif

#if NXP_RF_CONF
        NxpNci_CONF = NULL;
        if(gNfcController_generation == 1)
        {
            NxpNci_CONF = NxpNci_RF_CONF_1stGen;
            NxpNci_CONF_size = sizeof(NxpNci_RF_CONF_1stGen);
        }
        else if(gNfcController_generation == 2)
        {
            NxpNci_CONF = NxpNci_RF_CONF_2ndGen;
            NxpNci_CONF_size = sizeof(NxpNci_RF_CONF_2ndGen);
        }
        if (NxpNci_CONF != NULL)
        {
            isResetRequired = true;
            NxpNci_HostTransceive(NxpNci_CONF, NxpNci_CONF_size, Answer, sizeof(Answer), &AnswerSize);
            if ((Answer[0] != 0x40) || (Answer[1] != 0x02) || (Answer[3] != 0x00) || (Answer[4] != 0x00)) return NXPNCI_ERROR;
        }
#endif
        /* Store curent timestamp to NFC Controller memory for further checks */
        if(gNfcController_generation == 1) NCIWriteTS[5] = 0x0F;
        memcpy(&NCIWriteTS[7], NxpNci_SettingCurrentTS, sizeof(NxpNci_SettingCurrentTS));
        NxpNci_HostTransceive(NCIWriteTS, sizeof(NCIWriteTS), Answer, sizeof(Answer), &AnswerSize);
        if ((Answer[0] != 0x40) || (Answer[1] != 0x02) || (Answer[3] != 0x00) || (Answer[4] != 0x00)) return NXPNCI_ERROR;
    }
#endif

    if(isResetRequired)
    {
        /* Reset the NFC Controller to insure new settings apply */
        NxpNci_HostTransceive(NCICoreReset, sizeof(NCICoreReset), Answer, sizeof(Answer), &AnswerSize);
        if ((Answer[0] != 0x40) || (Answer[1] != 0x00) || (Answer[3] != 0x00)) return NXPNCI_ERROR;
        NxpNci_HostTransceive(NCICoreInit, sizeof(NCICoreInit), Answer, sizeof(Answer), &AnswerSize);
        if ((Answer[0] != 0x40) || (Answer[1] != 0x01) || (Answer[3] != 0x00)) return NXPNCI_ERROR;
    }

    return NXPNCI_SUCCESS;
}

bool NxpNci_ConfigureMode(unsigned char mode)
{
#if defined RW_SUPPORT || defined P2P_SUPPORT || defined CARDEMU_SUPPORT
    uint8_t Command[MAX_NCI_FRAME_SIZE];
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;
    uint8_t Item = 0;
    uint8_t NCIDiscoverMap[] = {0x21, 0x00};

#ifdef CARDEMU_SUPPORT
    const uint8_t DM_CARDEMU[] = {0x4, 0x2, 0x2};
    const uint8_t R_CARDEMU[] = {0x1, 0x3, 0x0, 0x1, 0x4};
#endif
#ifdef P2P_SUPPORT
    const uint8_t DM_P2P[] = {0x5, 0x3, 0x3};
    const uint8_t R_P2P[] = {0x1, 0x3, 0x0, 0x1, 0x5};
    uint8_t NCISetConfig_NFC[] = {0x20, 0x02, 0x1F, 0x02, 0x29, 0x0D, 0x46, 0x66, 0x6D, 0x01, 0x01, 0x11, 0x03, 0x02, 0x00, 0x01, 0x04, 0x01, 0xFA, 0x61, 0x0D, 0x46, 0x66, 0x6D, 0x01, 0x01, 0x11, 0x03, 0x02, 0x00, 0x01, 0x04, 0x01, 0xFA};
#endif
#ifdef RW_SUPPORT
    const uint8_t DM_RW[] = {0x1, 0x1, 0x1, 0x2, 0x1, 0x1, 0x3, 0x1, 0x1, 0x4, 0x1, 0x2, 0x80, 0x01, 0x80};
    uint8_t NCIPropAct[] = {0x2F, 0x02, 0x00};
#endif
#if defined P2P_SUPPORT || defined CARDEMU_SUPPORT
    uint8_t NCIRouting[] = {0x21, 0x01, 0x07, 0x00, 0x01};
    uint8_t NCISetConfig_NFCA_SELRSP[] = {0x20, 0x02, 0x04, 0x01, 0x32, 0x01, 0x00};
    // UID 04 45 76 4A B0 58 80
    // uint8_t NCISetConfig_NFCA_SELRSP[] = {0x20, 0x02, 0x10, 0x03, 0x31, 0x01, 0x44, 0x32, 0x01, 0x00, 0x33, 0x07, 0x04, 0x45, 0x76, 0x4A, 0xB0, 0x58, 0x80};
    // uint8_t NCISetConfig_NFCA_SELRSP[] = {0x20, 0x02, 0x10, 0x03, 0x31, 0x01, 0x40, 0x32, 0x01, 0x00, 0x33, 0x07, 0x04, 0x45, 0x76, 0x4A, 0xB0, 0x58, 0x80};
    // uint8_t NCISetConfig_NFCA_SELRSP[] = {0x20, 0x02, 0x0A, 0x02, 0x32, 0x01, 0x00, 0x33, 0x04, 0x04, 0x01, 0x02, 0x03};
    // uint8_t NCISetConfig_NFCA_SELRSP[] = {0x20, 0x02, 0x0D, 0x02, 0x32, 0x01, 0x00, 0x33, 0x07, 0x04, 0x45, 0x76, 0x4A, 0xB0, 0x58, 0x80};
#endif

    if(mode == 0) return NXPNCI_SUCCESS;

    /* Enable Proprietary interface for T4T card presence check procedure */
#ifdef RW_SUPPORT
    if (mode == NXPNCI_MODE_RW)
    {
        NxpNci_HostTransceive(NCIPropAct, sizeof(NCIPropAct), Answer, sizeof(Answer), &AnswerSize);
        if ((Answer[0] != 0x4F) || (Answer[1] != 0x02) || (Answer[3] != 0x00)) return NXPNCI_ERROR;
    }
#endif

    /* Building Discovery Map command */
    Item = 0;
#ifdef CARDEMU_SUPPORT
    if (mode & NXPNCI_MODE_CARDEMU)
    {
        memcpy(&Command[4+(3*Item)], DM_CARDEMU, sizeof(DM_CARDEMU));
        Item++;
    }
#endif
#ifdef P2P_SUPPORT
    if (mode & NXPNCI_MODE_P2P)
    {
        memcpy(&Command[4+(3*Item)], DM_P2P, sizeof(DM_P2P));
        Item++;
    }
#endif
#ifdef RW_SUPPORT
    if (mode & NXPNCI_MODE_RW)
    {
        memcpy(&Command[4+(3*Item)], DM_RW, sizeof(DM_RW));
        Item+=sizeof(DM_RW)/3;
    }
#endif
#if defined P2P_SUPPORT || defined CARDEMU_SUPPORT || defined RW_SUPPORT
    if (Item != 0)
    {
        memcpy(Command, NCIDiscoverMap, sizeof(NCIDiscoverMap));
        Command[2] = 1 + (Item*3);
        Command[3] = Item;
        NxpNci_HostTransceive(Command, 3 + Command[2], Answer, sizeof(Answer), &AnswerSize);
        if ((Answer[0] != 0x41) || (Answer[1] != 0x00) || (Answer[3] != 0x00)) return NXPNCI_ERROR;
    }
#endif

    /* Configuring routing */
    Item = 0;
#ifdef CARDEMU_SUPPORT
    if (mode & NXPNCI_MODE_CARDEMU)
    {
        memcpy(&Command[5+(5*Item)], R_CARDEMU, sizeof(R_CARDEMU));
        Item++;
    }
#endif
#ifdef P2P_SUPPORT
    if (mode & NXPNCI_MODE_P2P)
    {
        memcpy(&Command[5+(5*Item)], R_P2P, sizeof(R_P2P));
        Item++;
    }
#endif
#if defined P2P_SUPPORT || defined CARDEMU_SUPPORT
    if (Item != 0)
    {
        memcpy(Command, NCIRouting, sizeof(NCIRouting));
        Command[2] = 2 + (Item*5);
        Command[4] = Item;
        NxpNci_HostTransceive(Command, 3 + Command[2] , Answer, sizeof(Answer), &AnswerSize);
        if ((Answer[0] != 0x41) || (Answer[1] != 0x01) || (Answer[3] != 0x00)) return NXPNCI_ERROR;
    }
#endif

    /* Setting NFCA SEL_RSP */
#ifdef CARDEMU_SUPPORT
    if (mode & NXPNCI_MODE_CARDEMU)
    {
        NCISetConfig_NFCA_SELRSP[6] += 0x20;
    }
#endif
#ifdef P2P_SUPPORT
    if (mode & NXPNCI_MODE_P2P)
    {
        NCISetConfig_NFCA_SELRSP[6] += 0x40;
    }
#endif
#if defined P2P_SUPPORT || defined CARDEMU_SUPPORT
    if (NCISetConfig_NFCA_SELRSP[6] != 0x00)
    {
        NxpNci_HostTransceive(NCISetConfig_NFCA_SELRSP, sizeof(NCISetConfig_NFCA_SELRSP), Answer, sizeof(Answer), &AnswerSize);
        if ((Answer[0] != 0x40) || (Answer[1] != 0x02) || (Answer[3] != 0x00)) return NXPNCI_ERROR;
    }
#endif

    /* Setting LLCP support */
#ifdef P2P_SUPPORT
    if (mode & NXPNCI_MODE_P2P)
    {
        NxpNci_HostTransceive(NCISetConfig_NFC, sizeof(NCISetConfig_NFC), Answer, sizeof(Answer), &AnswerSize);
        if ((Answer[0] != 0x40) || (Answer[1] != 0x02) || (Answer[3] != 0x00)) return NXPNCI_ERROR;
    }
#endif
#endif
    return NXPNCI_SUCCESS;
}

bool NxpNci_ConfigureParams(unsigned char *pCmd, unsigned short CmdSize)
{
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;

    NxpNci_HostTransceive(pCmd, CmdSize, Answer, sizeof(Answer), &AnswerSize);
    if ((Answer[0] != 0x40) || (Answer[1] != 0x02) || (Answer[3] != 0x00)) return NXPNCI_ERROR;

    return NXPNCI_SUCCESS;
}

bool NxpNci_StartDiscovery(unsigned char *pTechTab, unsigned char TechTabSize)
{
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;
    uint8_t i;

    NCIStartDiscovery[0] = 0x21;
    NCIStartDiscovery[1] = 0x03;
    NCIStartDiscovery[2] = (TechTabSize * 2) + 1;
    NCIStartDiscovery[3] = TechTabSize;
    for (i=0; i<TechTabSize; i++)
    {
        NCIStartDiscovery[(i*2)+4] = pTechTab[i];
        NCIStartDiscovery[(i*2)+5] = 0x01;
    }

    NCIStartDiscovery_length = (TechTabSize * 2) + 4;
    NxpNci_HostTransceive(NCIStartDiscovery, NCIStartDiscovery_length, Answer, sizeof(Answer), &AnswerSize);
    if ((Answer[0] != 0x41) || (Answer[1] != 0x03) || (Answer[3] != 0x00)) return NXPNCI_ERROR;

    return NXPNCI_SUCCESS;
}

bool NxpNci_StopDiscovery(void)
{
    uint8_t NCIStopDiscovery[] = {0x21, 0x06, 0x01, 0x00};
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;

    NxpNci_HostTransceive(NCIStopDiscovery, sizeof(NCIStopDiscovery), Answer, sizeof(Answer), &AnswerSize);
    NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_1S);

    return NXPNCI_SUCCESS;
}

bool NxpNci_WaitForDiscoveryNotification(NxpNci_RfIntf_t *pRfIntf)
{
    uint8_t NCIRfDiscoverSelect[] = {0x21, 0x04, 0x03, 0x01, PROT_ISODEP, INTF_ISODEP};
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;

#ifdef P2P_SUPPORT
    uint8_t NCIStopDiscovery[] = {0x21, 0x06, 0x01, 0x00};
    uint8_t NCIRestartDiscovery[] = {0x21, 0x06, 0x01, 0x03};
    uint8_t saved_NTF[7];
wait:
#endif
    do
    {
        if(NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_INFINITE) == NXPNCI_ERROR) return NXPNCI_ERROR;
    }while ((Answer[0] != 0x61) || ((Answer[1] != 0x05) && (Answer[1] != 0x03)));

    gNextTag_Protocol = PROT_UNDETERMINED;

    /* Is RF_INTF_ACTIVATED_NTF ? */
    if (Answer[1] == 0x05)
    {
        pRfIntf->Interface = Answer[4];
        pRfIntf->Protocol = Answer[5];
        pRfIntf->ModeTech = Answer[6];
        pRfIntf->MoreTags = false;
        NxpNci_FillInterfaceInfo(pRfIntf, &Answer[10]);

#ifdef P2P_SUPPORT
        /* Verifying if not a P2P device also presenting T4T emulation */
        if ((pRfIntf->Interface == INTF_ISODEP) && (pRfIntf->Protocol == PROT_ISODEP) && ((pRfIntf->ModeTech & MODE_LISTEN) != MODE_LISTEN))
        {
            memcpy(saved_NTF, Answer, sizeof(saved_NTF));
            while(1)
            {
                /* Restart the discovery loop */ 
                NxpNci_HostTransceive(NCIRestartDiscovery, sizeof(NCIRestartDiscovery), Answer, sizeof(Answer), &AnswerSize);
                NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);

                /* Wait for discovery */
                do NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_1S);
                while ((AnswerSize == 4) && (Answer[0] == 0x60) && (Answer[1] == 0x07));

                if ((AnswerSize != 0) && (Answer[0] == 0x61) && (Answer[1] == 0x05))
                {
                    /* Is same device detected ? */
                    if (memcmp(saved_NTF, Answer, sizeof(saved_NTF)) == 0) break;
                    /* Is P2P detected ? */
                    if (Answer[5] == PROT_NFCDEP)
                    {
                        pRfIntf->Interface = Answer[4];
                        pRfIntf->Protocol = Answer[5];
                        pRfIntf->ModeTech = Answer[6];
                        pRfIntf->MoreTags = false;
                        NxpNci_FillInterfaceInfo(pRfIntf, &Answer[10]);
                        break;
                    }
                }
                else
                {
                    if (AnswerSize != 0)
                    {
                        /* Flush any other notification  */
                        while(Answer != 0) NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
                        /* Restart the discovery loop */ 
                        NxpNci_HostTransceive(NCIRestartDiscovery, sizeof(NCIRestartDiscovery), Answer, sizeof(Answer), &AnswerSize);
                        NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
                    }
                    goto wait;
                }
            }
        }
#endif
    }
    else /* RF_DISCOVER_NTF */
    {
        pRfIntf->Interface = INTF_UNDETERMINED;
        pRfIntf->Protocol = Answer[4];
        pRfIntf->ModeTech = Answer[5];
        pRfIntf->MoreTags = true;

        /* Get next NTF for further activation */
        do {
            if(NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS) == NXPNCI_ERROR)    return NXPNCI_ERROR;
        } while ((Answer[0] != 0x61) || (Answer[1] != 0x03));
        gNextTag_Protocol = Answer[4];

        /* Remaining NTF ? */
        while(Answer[AnswerSize-1] == 0x02) NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);

        /* In case of multiple cards, select the first one */
        NCIRfDiscoverSelect[4] = pRfIntf->Protocol;
        if (pRfIntf->Protocol == PROT_ISODEP) NCIRfDiscoverSelect[5] = INTF_ISODEP;
        else if (pRfIntf->Protocol == PROT_NFCDEP) NCIRfDiscoverSelect[5] = INTF_NFCDEP;
        else if (pRfIntf->Protocol == PROT_MIFARE) NCIRfDiscoverSelect[5] = INTF_TAGCMD;
        else NCIRfDiscoverSelect[5] = INTF_FRAME;
        NxpNci_HostTransceive(NCIRfDiscoverSelect, sizeof(NCIRfDiscoverSelect), Answer, sizeof(Answer), &AnswerSize);
        if ((Answer[0] == 0x41) || (Answer[1] == 0x04) || (Answer[3] == 0x00))
        {
            NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
            if ((Answer[0] == 0x61) || (Answer[1] == 0x05))
            {
                pRfIntf->Interface = Answer[4];
                pRfIntf->Protocol = Answer[5];
                pRfIntf->ModeTech = Answer[6];
                NxpNci_FillInterfaceInfo(pRfIntf, &Answer[10]);
            }
#ifdef P2P_SUPPORT
            /* In case of P2P target detected but lost, inform application to restart discovery */
            else if (pRfIntf->Protocol == PROT_NFCDEP)
            {
                /* Restart the discovery loop */
                NxpNci_HostTransceive(NCIStopDiscovery, sizeof(NCIStopDiscovery), Answer, sizeof(Answer), &AnswerSize);
                NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
                NxpNci_HostTransceive(NCIStartDiscovery, NCIStartDiscovery_length, Answer, sizeof(Answer), &AnswerSize);
                goto wait;
            }
#endif
        }
    }

    /* In case of unknown target align protocol information */
    if (pRfIntf->Interface == INTF_UNDETERMINED) pRfIntf->Protocol = PROT_UNDETERMINED;

    return NXPNCI_SUCCESS;
}

#ifdef NFC_FACTORY_TEST
bool NxpNci_FactoryTest_Prbs(NxpNci_TechType_t type, NxpNci_Bitrate_t bitrate)
{
	uint8_t NCIPrbs_1stGen[] = {0x2F, 0x30, 0x04, 0x00, 0x00, 0x01, 0x01};
	uint8_t NCIPrbs_2ndGen[] = {0x2F, 0x30, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01};
    uint8_t *NxpNci_cmd;
    uint16_t NxpNci_cmd_size = 0;
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;

    if(gNfcController_generation == 1)
    {
    	NxpNci_cmd = NCIPrbs_1stGen;
    	NxpNci_cmd_size = sizeof(NCIPrbs_1stGen);
    	NxpNci_cmd[3] = type;
    	NxpNci_cmd[4] = bitrate;
    }
    else if(gNfcController_generation == 2)
    {
    	NxpNci_cmd = NCIPrbs_2ndGen;
    	NxpNci_cmd_size = sizeof(NCIPrbs_2ndGen);
    	NxpNci_cmd[5] = type;
    	NxpNci_cmd[6] = bitrate;
    }

    if (NxpNci_cmd_size != 0)
    {
        NxpNci_HostTransceive(NxpNci_cmd, NxpNci_cmd_size, Answer, sizeof(Answer), &AnswerSize);
        if ((Answer[0] != 0x4F) || (Answer[1] != 0x30) || (Answer[3] != 0x00)) return NXPNCI_ERROR;
    }
    else
    {
    	return NXPNCI_ERROR;
    }

    return NXPNCI_SUCCESS;
}


bool NxpNci_FactoryTest_RfOn(void)
{
	uint8_t NCIRfOn[] = {0x2F, 0x3D, 0x02, 0x20, 0x01};
    uint8_t Answer[MAX_NCI_FRAME_SIZE];
    uint16_t AnswerSize;

    NxpNci_HostTransceive(NCIRfOn, sizeof(NCIRfOn), Answer, sizeof(Answer), &AnswerSize);
    if ((Answer[0] != 0x4F) || (Answer[1] != 0x3D) || (Answer[3] != 0x00)) return NXPNCI_ERROR;

    return NXPNCI_SUCCESS;
}
#endif
