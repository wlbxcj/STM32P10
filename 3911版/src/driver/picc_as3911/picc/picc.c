/********************************************************************************* 
Copyright (C) 2007 ViewAt Technology Co., LTD.                         

System Name    :  vos                                            
Module Name    :  PCD myfare and contactless driver                            
File   Name    :  picc.c                                                  
Revision       :  01.00                                                     
Date           :  2007/8/7           
Dir            :  drv\picc
error code     :  RF¿¨¶Á¿¨Æ÷Ä£¿é£º  -3500~-3999    
                                                                      
***********************************************************************************/

//13/08/07
#include "misc.h"

#include "picc.h"


//13/08/07
#define s_UartPrint trace_debug_printf

extern void initAS3911();

int s_PiccInit(void)
{
	initAS3911();
	return 0;
}


//int  Picc_Open(void)
int  Lib_PiccOpen(void)
{
	s16 ret = 0;
	s_PiccInit();
	ret = emvPiccOpen();
	switch(ret)
	{
	case EMV_ERR_OK:
		return PICC_OK;
		
	default:
		return -1;
	}
}

//int  Picc_Close(void)
int  Lib_PiccClose(void)
{
	s16 ret = 0;
        
	ret =emvPiccClose();
	switch(ret)
	{
	case EMV_ERR_OK:
		return PICC_OK;
		
	default:
		return -1;
	}
}

//int  Picc_Check(BYTE mode,BYTE *CardType,BYTE *SerialNo)
int  Lib_PiccCheck(uchar mode,uchar *CardType,uchar *SerialNo)
{
	s16 ret = 0;
	ret = emvPiccCheck(mode, CardType, SerialNo);
	//s_UartPrint("in Picc_Check ret = %d\r\n", ret);	//sxl
        
	switch(ret)
	{
	case EMV_ERR_OK:
		return PICC_OK;

	case EMV_ERR_COLLISION:
		return PICC_Collision;

	case EMV_ERR_RUNNING:
		return PICC_NotOpen;

	case EMV_ERR_INTERNAL:
		return PICC_ParameterErr;
		
	default:
		return -1;
           
	}
			
}


//int  Picc_Command(APDU_SEND *ApduSend, APDU_RESP *ApduResp)
int  Lib_PiccCommand(APDU_SEND *ApduSend, APDU_RESP *ApduResp)
{
	s16 ret = 0;
	ret = emvPiccCommand(ApduSend, ApduResp);
	LOG("in Picc_Command ret = %d\r\n", ret);
        //s_UartPrint("in Picc_Command ret = %d\r\n", ret);
	switch(ret)
	{
	case EMV_ERR_OK:
		return PICC_OK;

	case EMV_ERR_INTERNAL:
		return PICC_ParameterErr;
		
	case EMV_ERR_RUNNING:
		return PICC_NotOpen;

	case EMV_ERR_PROTOCOL:
		return PICC_ApduErr;
		
	default:
		return -1;
	}
	
}

int  Picc_M1Authority(BYTE Type,BYTE BlkNo,BYTE *Pwd,BYTE *SerialNo)
{
	return -1;
}
int  Picc_M1ReadBlock(BYTE BlkNo,BYTE *BlkValue)
{
	return -1;
}
int  Picc_M1WriteBlock(BYTE BlkNo,BYTE *BlkValue)
{
	return -1;
}
int  Picc_M1Operate(BYTE Type,BYTE BlkNo,BYTE *Value,BYTE UpdateBlkNo)
{
	return -1;
}

//void Picc_Halt(void)
void Lib_PiccHalt(void)
{
}

//int  Picc_Reset(void)
int  Lib_PiccReset(void)
{
	return 0;
}

//int  Picc_Remove(void)
int  Lib_PiccRemove(void)
{
	s16 ret;
	ret = emvPiccRemove();
LOG("/r/n run here \r\n");
	switch(ret)
	{
	case EMV_ERR_OK:
		return PICC_OK;
		
	case EMV_ERR_RUNNING:
		return PICC_NotOpen;

	case EMV_ERR_STOPPED:
		return PICC_HaveCard;

	default:
		return -1;
	}
	
}

int PICC_CARRIER_ON(void)
{	
	return 0;
}

int PICC_CARRIER_OFF(void)
{
	return 0;
}

int PICC_WUPA(uchar *atqa)
{
	return 0;
}

int PICC_WUPB(uchar *atqb)
{
	return 0;
}

int PICC_POLLING(void)
{
	return 0;
}



int PICC_WUPA_RATS(void)
{
	return 0;
}

int PICC_WUPB_ATTRIB(void)
{
	return 0;
}



int PICC_AntiColl_A()
{
	return 0;
}

int PICC_AntiColl_B()
{
	return 0;
}


/*

int Test_Picc(void) 
{
	APDU_SEND ApduSend_;
	APDU_RESP ApduResp_;

	int ret;
	
	s_UartPrint("Test_Picc test...\r\n");
	ret = Picc_Open();
	if(ret)
	{
		s_UartPrint("Picc_Open ret:%d\r\n", ret);
		return -1;
	}
	while(1){
		
		if(!Picc_Check(0, NULL, NULL))
			break;
		s_UartPrint("Lib_PiccCheck ing\r\n");

		if (!Kb_Hit())
		{
			if (KEYCANCEL == Kb_GetKey())
			{
				return -4;
			}  
		} 
	}
	ApduSend_.Command[0]=0x00;	
	ApduSend_.Command[1]=0xa4;	
	ApduSend_.Command[2]=0x04;	
	ApduSend_.Command[3]=0x00; 
	ApduSend_.Lc=0x0e;			
	ApduSend_.Le=0x00;			 
	memcpy(ApduSend_.DataIn,"2PAY.SYS.DDF01",14);

	ret = Picc_Command(&ApduSend_, &ApduResp_);
	if(ret){
		s_UartPrint("Lib_PiccCommand error %d \r\n", ret);
		Picc_Close();
		return -2;
	}
	s_UartPrint("Lib_PiccCommand ok\r\n");
	Picc_Close();
	return 0;	
}


*/
