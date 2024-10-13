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

#ifdef RW_SUPPORT
#ifndef NO_NDEF_SUPPORT
#include <tool.h>
#include <RW_NDEF.h>

#define T5T_ADD_MODE_1BYTE    0xE1
#define T5T_ADD_MODE_2BYTE    0xE2
#define T5T_NDEF_TLV		  0x03

typedef enum
{
    Initial,
    Reading_CC,
    Reading_Data,
    Reading_NDEF,
    Writing_Data
} RW_NDEF_T5T_state_t;

typedef struct
{
    unsigned char BlkNb;
    unsigned short MessagePtr;
    unsigned short MessageSize;
    unsigned char *pMessage;
    unsigned char AddrMode;
} RW_NDEF_T5T_Ndef_t;

static RW_NDEF_T5T_state_t eRW_NDEF_T5T_State = Initial;
static RW_NDEF_T5T_Ndef_t RW_NDEF_T5T_Ndef;

static unsigned short T5T_prepare_read_block(unsigned char blk, unsigned char *pCmd)
{
	unsigned short size;

	if(RW_NDEF_T5T_Ndef.AddrMode == T5T_ADD_MODE_1BYTE)
	{
		pCmd[0] = 0x02;
		pCmd[1] = 0x20;
		pCmd[2] = blk;
		size = 3;
	}
	else
	{
		pCmd[0] = 0x02;
		pCmd[1] = 0x30;
		pCmd[2] = 0x00;
		pCmd[3] = blk;
		size = 4;
	}
	return size;
}

static unsigned short T5T_prepare_write_block(unsigned char blk, unsigned char *pCmd, unsigned char *pData)
{
	unsigned short size;

	if(RW_NDEF_T5T_Ndef.AddrMode == T5T_ADD_MODE_1BYTE)
	{
		pCmd[0] = 0x02;
		pCmd[1] = 0x21;
		pCmd[2] = blk;
		memcpy(&pCmd[3], pData, 4);
		size = 7;
	}
	else
	{
		pCmd[0] = 0x02;
		pCmd[1] = 0x31;
		pCmd[2] = 0x00;
		pCmd[3] = blk;
		memcpy(&pCmd[4], pData, 4);
		size = 8;
	}
	return size;
}

void RW_NDEF_T5T_Reset(void)
{
    eRW_NDEF_T5T_State = Initial;
    RW_NDEF_T5T_Ndef.pMessage = NdefBuffer;
}

void RW_NDEF_T5T_Read_Next(unsigned char *pRsp, unsigned short Rsp_size, unsigned char *pCmd, unsigned short *pCmd_size)
{
    /* By default no further command to be sent */
    *pCmd_size = 0;

    switch(eRW_NDEF_T5T_State)
    {
    case Initial:
        /* Read CC */
		RW_NDEF_T5T_Ndef.AddrMode = T5T_ADD_MODE_1BYTE;
		*pCmd_size = T5T_prepare_read_block(0, pCmd);
        eRW_NDEF_T5T_State = Reading_CC;
        break;

    case Reading_CC:
        /* Is CC Read and Is Ndef ?*/
        if ((Rsp_size == 6) && (pRsp[Rsp_size-1] == 0x00) && ((pRsp[1] == T5T_ADD_MODE_1BYTE) || (pRsp[1] == T5T_ADD_MODE_2BYTE)))
        {
			RW_NDEF_T5T_Ndef.AddrMode = pRsp[1];
			/* Read First data */
			*pCmd_size = T5T_prepare_read_block(1, pCmd);
            eRW_NDEF_T5T_State = Reading_Data;
        }
        break;

    case Reading_Data:
        /* Is Read success ?*/
        if ((Rsp_size == 6) && (pRsp[Rsp_size-1] == 0x00))
        {
            if (pRsp[1] == T5T_NDEF_TLV)
            {
				RW_NDEF_T5T_Ndef.MessageSize = pRsp[2];

				/* If provisioned buffer is not large enough or message is empty, notify the application and stop reading */
				if ((RW_NDEF_T5T_Ndef.MessageSize > RW_MAX_NDEF_FILE_SIZE) || (RW_NDEF_T5T_Ndef.MessageSize == 0))
				{
					if(pRW_NDEF_PullCb != NULL) pRW_NDEF_PullCb(NULL, 0, RW_NDEF_T5T_Ndef.MessageSize);
					break;
				}

				/* Is NDEF read already completed ? */
				if (RW_NDEF_T5T_Ndef.MessageSize <= 2)
				{
					memcpy (RW_NDEF_T5T_Ndef.pMessage, &pRsp[3], RW_NDEF_T5T_Ndef.MessageSize);

					/* Notify application of the NDEF reception */
					if(pRW_NDEF_PullCb != NULL) pRW_NDEF_PullCb(RW_NDEF_T5T_Ndef.pMessage, RW_NDEF_T5T_Ndef.MessageSize, RW_NDEF_T5T_Ndef.MessageSize);
				}
				else
				{
					RW_NDEF_T5T_Ndef.MessagePtr = 2;
					memcpy (RW_NDEF_T5T_Ndef.pMessage, &pRsp[3], RW_NDEF_T5T_Ndef.MessagePtr);
					RW_NDEF_T5T_Ndef.BlkNb = 2;

					/* Read NDEF content */
					*pCmd_size = T5T_prepare_read_block(RW_NDEF_T5T_Ndef.BlkNb, pCmd);
					eRW_NDEF_T5T_State = Reading_NDEF;
				}
			}
        }
        break;

    case Reading_NDEF:
        /* Is Read success ?*/
        if ((Rsp_size == 6) && (pRsp[Rsp_size-1] == 0x00))
        {
            /* Is NDEF read already completed ? */
            if ((RW_NDEF_T5T_Ndef.MessageSize - RW_NDEF_T5T_Ndef.MessagePtr) <= 4)
            {
                memcpy (&RW_NDEF_T5T_Ndef.pMessage[RW_NDEF_T5T_Ndef.MessagePtr], &pRsp[1], RW_NDEF_T5T_Ndef.MessageSize - RW_NDEF_T5T_Ndef.MessagePtr);

                /* Notify application of the NDEF reception */
                if(pRW_NDEF_PullCb != NULL) pRW_NDEF_PullCb(RW_NDEF_T5T_Ndef.pMessage, RW_NDEF_T5T_Ndef.MessageSize, RW_NDEF_T5T_Ndef.MessageSize);
            }
            else
            {
                memcpy (&RW_NDEF_T5T_Ndef.pMessage[RW_NDEF_T5T_Ndef.MessagePtr], &pRsp[1], 4);
                RW_NDEF_T5T_Ndef.MessagePtr += 4;
                RW_NDEF_T5T_Ndef.BlkNb++;

                /* Read NDEF content */
				*pCmd_size = T5T_prepare_read_block(RW_NDEF_T5T_Ndef.BlkNb, pCmd);
            }
        }
        break;

    default:
        break;
    }
}

void RW_NDEF_T5T_Write_Next(unsigned char *pRsp, unsigned short Rsp_size, unsigned char *pCmd, unsigned short *pCmd_size)
{
    /* By default no further command to be sent */
    *pCmd_size = 0;

    switch(eRW_NDEF_T5T_State)
    {
    case Initial:
        /* Read CC */
		RW_NDEF_T5T_Ndef.AddrMode = T5T_ADD_MODE_1BYTE;
		*pCmd_size = T5T_prepare_read_block(0, pCmd);
        eRW_NDEF_T5T_State = Reading_CC;
        break;

    case Reading_CC:
        /* Is CC Read and Is Ndef */
        if ((Rsp_size == 6) && (pRsp[Rsp_size-1] == 0x00) && ((pRsp[1] == T5T_ADD_MODE_1BYTE) || (pRsp[1] == T5T_ADD_MODE_2BYTE)))
        {
			RW_NDEF_T5T_Ndef.AddrMode = pRsp[1];
            /* Is size enough ? */
            if ((pRsp[3]*8 >= RW_NdefMessage_size) && (0xFF >= RW_NdefMessage_size))
            {
				unsigned char data[4] = {0x03};
				data[1] = RW_NdefMessage_size;
                memcpy(&data[2], pRW_NdefMessage, 2);
				/* Write First data */
				*pCmd_size = T5T_prepare_write_block(1, pCmd, data);
                RW_NDEF_T5T_Ndef.MessagePtr = 2;
                RW_NDEF_T5T_Ndef.BlkNb = 2;
                eRW_NDEF_T5T_State = Writing_Data;
            }
        }
        break;

    case Writing_Data:
        /* Is Write success ?*/
        if ((Rsp_size == 2) && (pRsp[Rsp_size-1] == 0x00))
        {
            /* Is NDEF write already completed ? */
            if (RW_NdefMessage_size <= RW_NDEF_T5T_Ndef.MessagePtr)
            {
                /* Notify application of the NDEF send completion */
                if(pRW_NDEF_PushCb != NULL) pRW_NDEF_PushCb(pRW_NdefMessage, RW_NdefMessage_size);
            }
            else
            {
                /* Write NDEF content */
				*pCmd_size = T5T_prepare_write_block(RW_NDEF_T5T_Ndef.BlkNb, pCmd, pRW_NdefMessage+RW_NDEF_T5T_Ndef.MessagePtr);
                RW_NDEF_T5T_Ndef.MessagePtr+=4;
                RW_NDEF_T5T_Ndef.BlkNb++;
            }
        }
        break;

    default:
        break;
    }
}
#endif
#endif
