/**************************************************************
			日期:2010-08-02
			
**************************************************************/
#define FOR_PN512_DEVICE
#include "Mifre_TmDef.h"


//const uchar PCD_VER[5]="1.14";
//const uchar PCD_DATE[12]="201.06.08";

//extern PICC_PARA c_para;//与RC531公用此全局结构体
PICC_PARA c_para;//与RC531公用此全局结构体


volatile PN512_RF_WORKSTRUCT PN512_RF_WorkInfo;//by stanley 20130503
 

//////////////////////////////////////////////////////////////////////////////////
///////////////////                                               ////////////////
///////////////////                      API函数                  ////////////////
///////////////////                                               ////////////////
//////////////////////////////////////////////////////////////////////////////////

#if 0
void DelayMs(unsigned int ms)//for stm32f103
{
	#ifdef FOR_LINUX_SYS
	msleep(ms);
	#else
	delay_ms(ms);
	#endif
}

#endif


uchar PiccOpen(void)
{
    int ucRet = RET_RF_OK;
	uchar ucTemp = 0x00;	

	ucRet=PN512_s_RF_Init();

	if(ucRet) // 20071227 liuxl 增加是否由此模块的判断
	{
	    goto PICCOPEN_ERR;
	}
	
    PN512_RF_WorkInfo.ucRFOpen = 0x01;
	 
	return 0;
	
PICCOPEN_ERR: 
	 PN512_sHal_Module_Disable();//模块下电
	return ucRet;	
		
}

uchar PiccReset(void)
{
  if(PN512_RF_WorkInfo.ucRFOpen)
  {
    if(PN512_RF_WorkInfo.ucCurType!=RF_TYPEA)
      return 2;
    return 0;
  }
  else
    return 1;
}

void PiccClose(void)
{
	memset((uchar *)&PN512_RF_WorkInfo, 0x00, sizeof(PN512_RF_WORKSTRUCT));////设置PN512_RF_WorkInfo.open标志为0
	
	//beause reset pin control by cpu
	PN512_s_RFCloseCarrier();
	PN512_sHal_Module_Disable();//模块下电
   	DelayMs(5);
}


uchar PiccDetect(uchar Mode,uchar *CardType,uchar *SerialInfo,uchar *CID,uchar *Other)
{
   uchar ucRet = RET_RF_OK;
	 uchar ucATSLen = 0;
	 uchar ucATSData[256];
	 uchar ucATQA[2];
	 uchar ucTemp = 0;
	 
	 uchar ucSAK = 0;
	 uchar ucCardType = 0;
	 uchar aucSN[100];
	 uchar ucCID = 0;
	 uchar aucOther[300];

	 uint i=0;

	 memset(aucSN,0,sizeof(aucSN));
	 memset(aucOther, 0, sizeof(aucOther));

   //为与P80保持一致，允许指针为空，即不输出任何信息
     
	 //added初始化工作变量
	 PN512_s_RF_vInitWorkInfo();	 
	 
	 if(PN512_RF_WorkInfo.ucRFOpen == 0)
	 {
			// 模块未开启
			PN512_RF_WorkInfo.usErrNo = 2; // PN512 not powered on
			if(Other!=NULL)
			{
				// 20080505 增加长度域输出
				Other[0] = 2;
				Other[1] = PN512_RF_WorkInfo.usErrNo & 0xff;
				Other[2] = PN512_RF_WorkInfo.usErrNo >> 8;
			}
		 return RET_RF_ERR_NO_OPEN;
	 }
	 
	 
	 	// 打开载波
	PN512_s_RFOpenCarrier();

	 memset(ucATSData, 0x00, sizeof(ucATSData));

	 
	 switch(Mode)
	 {
	     case 0x00:  // 按照MasterCard PayPass规范寻卡并激活
        ucRet = PN512_s_RF_ucPOLL(&ucCardType);
				if(ucRet)
				{
					goto PICC_DETECT_END;
				}			
				PN512_RF_WorkInfo.ucStatus = 	PICC_STATUS_WAKEN;
				if (PN512_RF_WorkInfo.ucCurType == RF_TYPEA) //changed
				{
				
					ucRet = PN512_s_RF_ucActPro(&ucATSLen,ucATSData);
					if(ucRet)
					{
					goto PICC_DETECT_END;
					}		
					PN512_RF_WorkInfo.ucStatus = 	PICC_STATUS_ACTIV;
				
					aucSN[0] = PN512_RF_WorkInfo.ucUIDLen;
					ucCID = 0; // 要求感应区内只能有一张卡片
					
					PN512_s_RF_vGetDetectAInfo(aucSN, aucOther);// 输出所有卡片响应信息
					
					memcpy(aucOther+aucOther[0]+1, ucATSData, ucATSLen);
					aucOther[0] += ucATSLen;
				
				}
				else  // if(*CardType == 'B')
				{
					ucRet = PN512_s_RF_ucActTypeB(&ucATSLen, ucATSData);
					if(ucRet)
					{
					  goto PICC_DETECT_END;
					}				
					PN512_RF_WorkInfo.ucStatus = 	PICC_STATUS_ACTIV;
				
					aucSN[0] = 4;
					memcpy(aucSN+1, (uchar*)&PN512_RF_WorkInfo.ucATQB[1], 4);
					ucCID = 0; // 要求感应区内只能有一张卡片
					aucOther[0] = 14;
					memcpy(aucOther+3, (uchar*)PN512_RF_WorkInfo.ucATQB, 12);
				break;
				}
		 	break;

		 case 'A':
		 case 'a':
      		ucRet = PN512_s_RF_ucWUPA(ucATQA);// 激活卡片
			if(ucRet)
			{
         		goto PICC_DETECT_END;
			}
			
			PN512_RF_WorkInfo.ucStatus = 	PICC_STATUS_WAKEN;
			// 保存获取的ATQA参数
			PN512_RF_WorkInfo.ucATQA[0] = ucATQA[0];
			PN512_RF_WorkInfo.ucATQA[1] = ucATQA[1];
			ucRet = PN512_s_RF_ucGetUID(); // 获取卡片的UID号
			if(ucRet)
			{
         		goto PICC_DETECT_END;
			}
			
			//根据SAK统一判断是否符合14443协议
			if(PN512_RF_WorkInfo.ucUIDLen == 4)
				ucSAK = PN512_RF_WorkInfo.ucSAK1;
			else if(PN512_RF_WorkInfo.ucUIDLen == 7)
				ucSAK = PN512_RF_WorkInfo.ucSAK2;
			else if(PN512_RF_WorkInfo.ucUIDLen == 10)
				ucSAK = PN512_RF_WorkInfo.ucSAK3;
			
			//一些自发卡企业所发的A卡不一定满足SAK的要求，根据应用参数确定是否判断卡片类型
			if(!(PN512_RF_WorkInfo.ucSAK1 & 0x20)
		  && (c_para.card_type_check_w == 0 || (c_para.card_type_check_w == 1 && c_para.card_type_check_val == 0)))
			{
			    // 不是ISO14443-4协议的卡片
				ucRet = RET_RF_ERR_PROTOCOL;
				goto PICC_DETECT_END;				
			}
			
			ucRet = PN512_s_RF_ucRATS(&ucATSLen,ucATSData);
			if(ucRet)
			{
			    goto PICC_DETECT_END;		
			}
			PN512_RF_WorkInfo.ucStatus = 	PICC_STATUS_ACTIV;

			//判断当前卡片类型
			PN512_s_RF_ucCardTypeCheck(&ucCardType);
			PN512_RF_WorkInfo.ucMifCardType = ucCardType;
			PN512_RF_WorkInfo.ucCurType = RF_TYPEA;
			
      		// 输出信息
      		ucCardType = RF_TYPE_PRO;
            
			aucSN[0] = PN512_RF_WorkInfo.ucUIDLen;
			PN512_s_RF_vGetDetectAInfo(aucSN, aucOther);// 输出所有卡片响应信息

			memcpy(aucOther+aucOther[0]+1, ucATSData, ucATSLen);
			aucOther[0] += ucATSLen;	
            
			ucCID = 0; // 要求感应区内只能有一张卡片

		 break;

		 case 'B':
		 case 'b':
      		ucRet = PN512_s_RF_ucWUPB(&ucATSLen,ucATSData);
			if(ucRet)//modifly by skx 增强B卡防冲突能力
			{
				goto PICC_DETECT_END;		
			}
			
			else
			{
				// 接下来将TypeB卡置为HALT状态
				for (i = 0; i < 3; i++) 
				{
					ucRet = PN512_s_RF_ucHALTB(&ucATSLen);
					if (ucRet != RET_RF_ERR_TIMEOUT) 
					{
						break;
					}
				}
			
				if(ucRet == RET_RF_OK)
				{
					ucRet = PN512_s_RF_ucREQB(&ucATSLen, ucATSData);
					if (ucRet != RET_RF_ERR_TIMEOUT)
					{
						ucRet=RET_RF_ERR_MULTI_CARD;
						goto PICC_DETECT_END;
					}
				
				}
				else
				{
					goto PICC_DETECT_END;
				}
			}
			
			ucRet = PN512_s_RF_ucWUPB(&ucATSLen,ucATSData);
			

			PN512_RF_WorkInfo.ucStatus = 	PICC_STATUS_WAKEN;
			ucRet = PN512_s_RF_ucAttrib(&ucTemp);
			if(ucRet)
			{
          		goto PICC_DETECT_END;		
			}	
			PN512_RF_WorkInfo.ucStatus = 	PICC_STATUS_ACTIV;

			aucSN[0] = 4;
            memcpy(aucSN+1, &ucATSData[1], 4);
			ucCID = 0; // 要求感应区内只能有一张卡片
			aucOther[0] = 14;
			memcpy(aucOther+3, ucATSData, 12);
			
			ucCardType = RF_TYPE_B;
			PN512_RF_WorkInfo.ucMifCardType = RF_TYPE_B;
			PN512_RF_WorkInfo.ucCurType = RF_TYPEB;
			
		 break;

		 case 'M':
		 case 'm':
      		ucRet = PN512_s_RF_ucWUPA(ucATQA); // 激活卡片
			if(ucRet)
			{
			   goto PICC_DETECT_END;		
			}
			PN512_RF_WorkInfo.ucStatus = 	PICC_STATUS_WAKEN;
			// 保存获取的ATQA参数
			PN512_RF_WorkInfo.ucATQA[0] = ucATQA[0];
			PN512_RF_WorkInfo.ucATQA[1] = ucATQA[1];
			 
			ucRet = PN512_s_RF_ucGetUID(); // 获取卡片的UID号
			if(ucRet)
			{
         		goto PICC_DETECT_END;
			}
			
			// 根据SAK统一判断是否Mifare One 
			if(PN512_RF_WorkInfo.ucUIDLen == 4)
				ucSAK = PN512_RF_WorkInfo.ucSAK1;
			else if(PN512_RF_WorkInfo.ucUIDLen == 7)
				ucSAK = PN512_RF_WorkInfo.ucSAK2;
			else if(PN512_RF_WorkInfo.ucUIDLen == 10)
				ucSAK = PN512_RF_WorkInfo.ucSAK3;
			
			//一些自发卡企业所发的A卡不一定满足SAK的要求，根据应用参数确定是否判断卡片类型
			if(!(PN512_RF_WorkInfo.ucSAK1 & 0x18)
		  && (c_para.card_type_check_w == 0 || (c_para.card_type_check_w == 1 && c_para.card_type_check_val == 0)))
			{	
				// 非Mifare One 卡
				ucRet = RET_RF_ERR_PROTOCOL;
				goto PICC_DETECT_END;	
			} //
			
			// 输出信息
			//判断当前卡片类型
			PN512_s_RF_ucCardTypeCheck(&ucCardType);
			PN512_RF_WorkInfo.ucMifCardType = ucCardType;
			PN512_RF_WorkInfo.ucCurType = RF_TYPEA;
            
			ucCID = 0; // 要求感应区内只能有一张卡片
			
			aucSN[0] = PN512_RF_WorkInfo.ucUIDLen;
			PN512_s_RF_vGetDetectAInfo(aucSN, aucOther);// 输出所有卡片响应信息
            			
		break;

		default:
			return RET_RF_ERR_PARAM;
	 }
	 
	PN512_RF_WorkInfo.ucMifActivateFlag = 1; 
	PN512_RF_WorkInfo.ucCurPCB = 1;
	ucRet = RET_RF_OK;

PICC_DETECT_END:		

	if(CardType != NULL)
		*CardType= 0;
	if((ucRet == RET_RF_OK) && (CardType != NULL))
	{
		if(PN512_RF_WorkInfo.ucCurType == RF_TYPEB)
			*CardType = 'B';
		else
		{
			if(Mode == 'M')
				*CardType = 'M';
			else
				*CardType = 'A';
		}
	}
	
	if(CID != NULL)
		*CID = ucCID;
	
	if(SerialInfo != NULL)
		memcpy(SerialInfo, aucSN, aucSN[0]+1);

	if(Other != NULL)
		memcpy(Other,aucOther, aucOther[0]+1);

	if(ucRet != RET_RF_OK)
	{
		if(Other!=NULL)
		{
			//增加长度域输出
			if (aucOther[0] == 0) 
			{
				Other[0] = 2;
			}
			Other[1] = PN512_RF_WorkInfo.usErrNo & 0xff;
			Other[2] = PN512_RF_WorkInfo.usErrNo >> 8;
		}
		
		// 寻卡不成功，则直接返回
		//	if(ucRet != RET_RF_ERR_TIMEOUT)
		PN512_s_RFResetCarrier();
		//s_RFCloseCarrier();
		PN512_RF_WorkInfo.ucStatus = 0;
		//s_RFDelay10ETUs(200);		
	}
		
	switch(ucRet)
	{
		case RET_RF_OK:          return 0;
		case RET_RF_ERR_PARAM:   return 1;//E_INVALID_PARA; // 1;
		case RET_RF_ERR_NO_OPEN: return 2;//E_NO_POWER;     // 2;
		case RET_RF_ERR_TIMEOUT: return 3;//E_NOT_DETECTED; // 3;
		case RET_RF_ERR_MULTI_CARD: return 4;//E_MULTI_CARDS; // 4;
		case RET_RF_ERR_PROTOCOL:   return 6;//E_PROTOCOL;    // 6;			
		default: return ucRet;			
	}

}


uchar PiccIsoCommand(uchar cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv)
{
  	uchar ucRet = RET_RF_OK;
	uchar ucInData[300];
	uchar ucOutData[300];
	uint  uiSendLen = 0;
	uint  uiRecLen = 0;

	//增加对LC的限制
	if((ApduSend == NULL) || (ApduRecv == NULL) || cid > 14 || ApduSend->Lc > 255)
	{
         // 参数错误
		 return RET_RF_ERR_PARAM;
	}

	if(PN512_RF_WorkInfo.ucRFOpen == 0)
		return RET_RF_ERR_NO_OPEN;// 模块未开启
	if(PN512_RF_WorkInfo.ucMifActivateFlag == 0)
	{
         // 卡片未激活
		 return 3;
		 //return RET_RF_ERR_NOT_ACT;
	}
	
	if(PN512_RF_WorkInfo.ucInProtocol == 0)
	{
		// 卡片未进入协议态
		return RET_RF_ERR_STATUS;
	}
	

	memset(ucInData, 0x00, sizeof(ucInData));
	memset(ucOutData, 0x00, sizeof(ucOutData));

    memcpy(ucInData, ApduSend->Command, 4);
	uiSendLen = 4;

	if((ApduSend->Lc == 0) && (ApduSend->Le == 0))
	{
        ucInData[uiSendLen] = 0x00;
		uiSendLen++;
	}

	if(ApduSend->Lc)
	{
	    ucInData[uiSendLen] = ApduSend->Lc;
		uiSendLen++;
		memcpy(ucInData+uiSendLen, ApduSend->DataIn, ApduSend->Lc);
		uiSendLen = uiSendLen + ApduSend->Lc;
	}

	if(ApduSend->Le)
	{
	   if(ApduSend->Le == 256)
	   {
	       ucInData[uiSendLen] = 0x00;
	   }
	   else
	   {
	       ucInData[uiSendLen] = ApduSend->Le;
	   }
	   uiSendLen++;
	}

	ucRet = PN512_s_RF_ucExchange(uiSendLen,ucInData,&uiRecLen,ucOutData);
	if(ucRet != RET_RF_OK)
	{
	    PN512_s_RF_ucDeselect();
	    
		PN512_RF_WorkInfo.ucStatus=0;
		
		//return ucRet;
		ApduRecv->SWA = PN512_RF_WorkInfo.usErrNo & 0x0FF;
	  	ApduRecv->SWB = PN512_RF_WorkInfo.usErrNo >> 8;
		switch(ucRet)
		{
			case RET_RF_ERR_PARAM:   return EE_INVALID_PARA; // 1;
			case RET_RF_ERR_NO_OPEN: return EE_NO_POWER;     // 2;
			case RET_RF_ERR_NOT_ACT: return EE_NOT_DETECTED; // 3;
			case RET_RF_ERR_TRANSMIT: return EE_TX;          // 4
			case RET_RF_ERR_PROTOCOL:   return EE_PROTOCOL;  // 5;			
			default: return ucRet;			
		}
	}

	ApduRecv->LenOut = uiRecLen - 2;
	if(ApduSend->Le < ApduRecv->LenOut)
		ApduRecv->LenOut = ApduSend->Le; 
	memcpy(ApduRecv->DataOut, ucOutData, ApduRecv->LenOut);
	ApduRecv->SWA = ucOutData[uiRecLen - 2];
	ApduRecv->SWB = ucOutData[uiRecLen - 1];
	 
	return RET_RF_OK;
}


uchar PiccRemove(uchar mode, uchar cid)
{
    uchar ucRet = RET_RF_OK;
    uchar ucTemp = 0x00;

    if(PN512_RF_WorkInfo.ucRFOpen == 0)
		{
	         // 模块未开启
			 return RET_RF_ERR_NO_OPEN;
		}

	if( ((mode != 'H'&& mode != 'h') && (mode != 'r'&&mode != 'R')) || cid > 14)
		return 1;

	if( (!PN512_RF_WorkInfo.ucRemove_Step || mode=='H' || mode=='h') && PN512_RF_WorkInfo.ucStatus == 0 )
        {
                PiccDetect(0,NULL,NULL,NULL,NULL); //13/01/14          
		//return 3;//card not waken up
        }
		
	if(PN512_RF_WorkInfo.ucStatus == PICC_STATUS_WAKEN)
	{
		PN512_RF_WorkInfo.ucMifActivateFlag = 0;
		PN512_RF_WorkInfo.ucStatus=0;
		if (PN512_RF_WorkInfo.ucCurType == RF_TYPEA) 
		{
			PN512_s_RF_vHALTA();
			ucRet = RET_RF_OK;
		}
		else
		{
			ucRet = PN512_s_RF_ucHALTB(&ucTemp); 
		}
		if(mode=='H' || mode=='h')
		{ 
			if(ucRet)
			{
				PN512_s_RFResetCarrier();//disable output carrier 
			}
			return 0;
		}
	}
	else if(!PN512_RF_WorkInfo.ucRemove_Step || mode=='H'||mode=='h')
	{
		ucRet = PN512_s_RF_ucDeselect();
		if(mode=='H' || mode=='h')
		{ 
			PN512_RF_WorkInfo.ucMifActivateFlag = 0;
			PN512_RF_WorkInfo.ucStatus=0;
			return ucRet;
		}
	}
	
	if (PN512_RF_WorkInfo.ucRemove_Step == 1) 
	{
		if (PN512_RF_WorkInfo.ucRemoved == PICC_STATUS_REMOVE) 
		{
			return 0x00;
		}
	}
	PN512_RF_WorkInfo.ucRemove_Step = 1;
	ucRet = PN512_s_RF_ucIfCardExit();
	
	PN512_RF_WorkInfo.ucStatus=0;

	if(ucRet == RET_RF_OK)
	{
		PN512_s_RFResetCarrier();//keeps for 1000etu
		PN512_RF_WorkInfo.ucRemoved=PICC_STATUS_REMOVE;		
		return 0;
	}
	else
	{
		return 6;// RET_RF_ERR_CARD_EXIST;//not removed yet
	}
	
}


uchar M1ToPro_RATS(uchar* pucOutLen, uchar* pucATSData)
{
	uchar ucReSendNo = 0;
	uchar ucRet = 0;
	
	// SELECT操作使PICC进入ACTIVATE状态，应再发送RATS获取ATS内容
	for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	{
	    ucRet = PN512_s_RF_ucRATS(pucOutLen, pucATSData);
		if(ucRet != RET_RF_ERR_TIMEOUT)
	    {
	         break;
	    }
	}

	if(ucRet != RET_RF_OK)
	{
	   return ucRet;
	}

	PN512_RF_WorkInfo.ucMifActivateFlag = 1;
	PN512_RF_WorkInfo.ucCurPCB = 1;
//PN512_RF_WorkInfo.ucInProtocol = 1;

	return RET_RF_OK;
}



uchar M1Authority(uchar Type,uchar BlkNo,uchar *Pwd,uchar *SerialNo)
{
    uchar ucRet = 0;
	uchar ucTemp = 0;

    if((Pwd == NULL) || (SerialNo == NULL))
    {
         return RET_RF_ERR_PARAM;
    }

	 // 增加参数判断
		// 一些自发卡企业所发的A卡不一定满足SAK的要求，根据应用参数确定是否判断卡片类型
	 if((PN512_RF_WorkInfo.ucMifCardType == RF_TYPE_S50)
		&& (c_para.card_type_check_w == 0 || (c_para.card_type_check_w == 1 && c_para.card_type_check_val == 0)))
	 {
	     if(BlkNo > 63)
		 {
			 return RET_RF_ERR_PARAM;
		 }
	 }	

	if(PN512_RF_WorkInfo.ucRFOpen == 0)
		return RET_RF_ERR_NO_OPEN;// 模块未开启
	if(PN512_RF_WorkInfo.ucMifActivateFlag == 0)
	{
         // 卡片未激活
		 return 3;//RET_RF_ERR_NOT_ACT;
	}
	if(PN512_RF_WorkInfo.ucInProtocol== 1) 
	{
         // 卡片当前协议状态错误,已进入协议态
		 return RET_RF_ERR_STATUS;
	}


    switch(Type)
    {
      case 'a':
			case 'A':
				PN512_RF_WorkInfo.aucBuffer[0] = PICC_AUTHENTA;
			break;
			case 'b':
			case 'B':
				PN512_RF_WorkInfo.aucBuffer[0] = PICC_AUTHENTB;
			break;
			default:
				return RET_RF_ERR_PARAM;
    }

	PN512_RF_WorkInfo.aucBuffer[1] = BlkNo;
	memcpy((uchar*)&PN512_RF_WorkInfo.aucBuffer[2], Pwd, 6);
	memcpy((uchar*)&PN512_RF_WorkInfo.aucBuffer[8], SerialNo, 4);

	PN512_RF_WorkInfo.ulSendBytes = 12;

	PN512_s_RFSetTimer(560); // 定时560ETU
	ucRet = PN512_s_RFExchangeCmd(PN512_MFAuthent_CMD);
	if(ucRet)
	{
	     return ucRet;
	}

  	PN512_s_vRFReadReg(1,PN512_STATUS2_REG,&ucTemp);
	if((ucTemp & 0x0F) == 0X08)
	{   // CryptOn为高，说明验证成功
	    return RET_RF_OK;
	}
	return RET_RF_ERR_AUTH;
}


uchar M1ReadBlock(uchar BlkNo,uchar *BlkValue)
{
     uchar ucRet = RET_RF_OK;

     if(BlkValue == NULL)
     {
         // 参数错误
		 		return RET_RF_ERR_PARAM;
     }
	 
	 //增加参数判断
		//一些自发卡企业所发的A卡不一定满足SAK的要求，根据应用参数确定是否判断卡片类型
	 if((PN512_RF_WorkInfo.ucMifCardType == RF_TYPE_S50)
		&& (c_para.card_type_check_w == 0 || (c_para.card_type_check_w == 1 && c_para.card_type_check_val == 0)))
	 {
	   if(BlkNo > 63)
		 {
			 return RET_RF_ERR_PARAM;
		 }
	 }

	 if(PN512_RF_WorkInfo.ucRFOpen == 0)
			return RET_RF_ERR_NO_OPEN;// 模块未开启
	 if(PN512_RF_WorkInfo.ucMifActivateFlag == 0)
	 {
         // 卡片未激活
		 return 3;//RET_RF_ERR_NOT_ACT
	 }
	if(PN512_RF_WorkInfo.ucInProtocol== 1) 
	{
         // 卡片当前协议状态错误,已进入协议态
		 return RET_RF_ERR_STATUS;
	}

   	PN512_s_vRFFlushFIFO();

	 PN512_RF_WorkInfo.aucBuffer[0] = PICC_READ;
	 PN512_RF_WorkInfo.aucBuffer[1] = BlkNo;

	 PN512_RF_WorkInfo.ulSendBytes = 2;

	 PN512_s_RFSetTimer(560);
	 PN512_RF_WorkInfo.ucM1Stage = 1;
	 ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);
	 PN512_RF_WorkInfo.ucM1Stage = 0;
	
	 if((ucRet != RET_RF_OK) || (PN512_RF_WorkInfo.lBitsReceived != 0x80)) // 16 BYTES
	 {
         if(ucRet != RET_RF_ERR_TIMEOUT)
         {
              if((PN512_RF_WorkInfo.lBitsReceived == 4) || (PN512_RF_WorkInfo.lBitsReceived == 8))
              {
                    if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0a) == 0)
                    {
                          return 4;//RET_RF_ERR_NO_AUTH
                    }
					else if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0a) == 0x0a)
					{
					      return RET_RF_OK;
					}
					else
					{
					      return RET_RF_ERR_TRANSMIT;
					}
              }
			  else
			  {
			        return RET_RF_ERR_TRANSMIT;
			  }
         }
	 }
	 else
	 {
	     memcpy(BlkValue, (uchar*)PN512_RF_WorkInfo.aucBuffer, 16);
	 }

//	 s_RFDelay10ETUs(10);

	 return ucRet;
}


uchar M1WriteBlock(uchar BlkNo,uchar *BlkValue)
{
     uchar ucRet = RET_RF_OK;

     if(BlkValue == NULL)
     {
         // 参数错误
		 return RET_RF_ERR_PARAM;
     }
	 
	 //增加参数判断
		//一些自发卡企业所发的A卡不一定满足SAK的要求，根据应用参数确定是否判断卡片类型
	 if((PN512_RF_WorkInfo.ucMifCardType == RF_TYPE_S50)
		&& (c_para.card_type_check_w == 0 || (c_para.card_type_check_w == 1 && c_para.card_type_check_val == 0)))
	 {
	     if(BlkNo > 63)
		 {
			 return RET_RF_ERR_PARAM;
		 }
	 }


	 if(PN512_RF_WorkInfo.ucRFOpen == 0)
		return RET_RF_ERR_NO_OPEN;// 模块未开启
	 if(PN512_RF_WorkInfo.ucMifActivateFlag == 0)
	 {
         // 卡片未激活
		 return 3;//RET_RF_ERR_NOT_ACT
	 }
	if(PN512_RF_WorkInfo.ucInProtocol== 1) 
	{
         // 卡片当前协议状态错误,已进入协议态
		 return RET_RF_ERR_STATUS;
	}
	
     PN512_s_vRFFlushFIFO();

	 PN512_RF_WorkInfo.aucBuffer[0] = PICC_WRITE;
	 PN512_RF_WorkInfo.aucBuffer[1] = BlkNo;

	 PN512_RF_WorkInfo.ulSendBytes = 2;

	 PN512_s_RFSetTimer(560);

	 PN512_RF_WorkInfo.ucM1Stage = 1;
	 ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);
	 PN512_RF_WorkInfo.ucM1Stage = 0;
     if(ucRet != RET_RF_ERR_TIMEOUT)
     {
          if((PN512_RF_WorkInfo.lBitsReceived == 4) || (PN512_RF_WorkInfo.lBitsReceived == 8))
          {
                if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0a) == 0x0)
                {
                      ucRet = 4;//RET_RF_ERR_NO_AUTH
                }
				else if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0a) == 0x0a)
				{
				      ucRet = RET_RF_OK;
				}
				else
				{	
				      ucRet = RET_RF_ERR_TRANSMIT;
				}
          }
		  else
		  {		
		        ucRet = RET_RF_ERR_TRANSMIT;
		  }
     }

	 if(ucRet == RET_RF_OK)
	 {
	      PN512_s_vRFFlushFIFO();

	      memcpy((uchar*)PN512_RF_WorkInfo.aucBuffer, BlkValue, 16);
	      PN512_RF_WorkInfo.ulSendBytes = 16;

	      PN512_s_RFSetTimer(2048);
		  
		  PN512_RF_WorkInfo.ucM1Stage = 1;
		  ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);		  
		  PN512_RF_WorkInfo.ucM1Stage = 0;

		  if(ucRet != RET_RF_ERR_TIMEOUT)
		  {
               if((PN512_RF_WorkInfo.lBitsReceived == 4) || (PN512_RF_WorkInfo.lBitsReceived == 8))
	           {
	                if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0a) == 0x0)
	                {
	                      ucRet = 4;//RET_RF_ERR_NO_AUTH;
	                }
					else if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0a) == 0x0a)
					{
					      ucRet = RET_RF_OK;
					}
					else
					{	
					      ucRet = RET_RF_ERR_TRANSMIT;
					}
	           }
			   else
			   {	
			        ucRet = RET_RF_ERR_TRANSMIT;
			   }
		  }
	 }

//     s_RFDelay10ETUs(10);

	 return ucRet;
}


uchar M1Operate(uchar Type, uchar BlkNo, uchar *Value, uchar UpdateBlkNo)
{
    uchar ucRet = RET_RF_OK;

    if(Value == NULL)
    {
         // 参数错误
		 return RET_RF_ERR_PARAM;
    }
	
	 // 增加参数判断
		// 一些自发卡企业所发的A卡不一定满足SAK的要求，根据应用参数确定是否判断卡片类型
	 if((PN512_RF_WorkInfo.ucMifCardType == RF_TYPE_S50)
		&& (c_para.card_type_check_w == 0 || (c_para.card_type_check_w == 1 && c_para.card_type_check_val == 0)))
	 {
	     if(BlkNo > 63 || UpdateBlkNo > 63)
		 {
			 return RET_RF_ERR_PARAM;
		 }
	 }

	if(PN512_RF_WorkInfo.ucRFOpen == 0)
		return RET_RF_ERR_NO_OPEN;// 模块未开启
	if(PN512_RF_WorkInfo.ucMifActivateFlag == 0)
	{
         // 卡片未激活
		 return 3;//RET_RF_ERR_NOT_ACT
	}

	if(PN512_RF_WorkInfo.ucInProtocol== 1) 
	{
         // 卡片当前协议状态错误,已进入协议态
		 return RET_RF_ERR_STATUS;
	}
	
	PN512_s_vRFFlushFIFO();
	switch(Type)
	{
	    case '+':
			PN512_RF_WorkInfo.aucBuffer[0] = PICC_INCREMENT;
			break;
		case '-':
			PN512_RF_WorkInfo.aucBuffer[0] = PICC_DECREMENT;
			break;
		case '>':
			PN512_RF_WorkInfo.aucBuffer[0] = PICC_RESTORE;
			break;
		default:
			return RET_RF_ERR_PARAM;
	}

	PN512_RF_WorkInfo.aucBuffer[1] = BlkNo;
	PN512_RF_WorkInfo.ulSendBytes = 2;

	PN512_s_RFSetTimer(2048);
	 PN512_RF_WorkInfo.ucM1Stage = 1;
	ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);
	 PN512_RF_WorkInfo.ucM1Stage = 0;

	if(ucRet != RET_RF_ERR_TIMEOUT)
	{
	    if((PN512_RF_WorkInfo.lBitsReceived == 4) || (PN512_RF_WorkInfo.lBitsReceived == 8))
        {
            if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0a) == 0x0)
            {
                  ucRet = 4;//RET_RF_ERR_NO_AUTH
            }
			else if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0a) == 0x0a)
			{
			      ucRet = RET_RF_OK;
			}
			else if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0f) == 0x01)
			{
			      ucRet = RET_RF_ERR_VAL;
			}
			else
			{
			      ucRet = RET_RF_ERR_TRANSMIT;
			}
        }
	    else
	    {
	        ucRet = RET_RF_ERR_TRANSMIT;
	    }
	}

    if(ucRet == RET_RF_OK)
    {
          PN512_s_vRFFlushFIFO();

		  if((Type == '+') || (Type == '-'))
		  {
		       memcpy((uchar*)PN512_RF_WorkInfo.aucBuffer, Value, 4);
		  }
		  else
		  {
		       memset((uchar*)PN512_RF_WorkInfo.aucBuffer, 0x00, 4);
		  }

		  PN512_RF_WorkInfo.ulSendBytes = 4;
		  PN512_s_RFSetTimer(2048);
		  PN512_RF_WorkInfo.ucM1Stage = 1;
	      ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);
		  PN512_RF_WorkInfo.ucM1Stage = 0;

		  if(ucRet == RET_RF_OK)
		  {
                if((PN512_RF_WorkInfo.lBitsReceived == 4) || (PN512_RF_WorkInfo.lBitsReceived == 8))
		        {
		            if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0a) == 0x0)
		            {
		                  ucRet = 4;//RET_RF_ERR_NO_AUTH
		            }
					else if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0f) == 0x01)
					{
					      ucRet = RET_RF_ERR_VAL;
					}
					else
					{
					      ucRet = RET_RF_ERR_TRANSMIT;
					}
		        }
				else
			    {
			        ucRet = RET_RF_ERR_TRANSMIT;
			    }
		  }
		  else if(ucRet == RET_RF_ERR_TIMEOUT)
		  {
		        ucRet = RET_RF_OK;
		  }
    }

	if(ucRet == RET_RF_OK)
	{
          PN512_s_vRFFlushFIFO();

		  PN512_RF_WorkInfo.aucBuffer[0] = PICC_TRANSFER;
		  PN512_RF_WorkInfo.aucBuffer[1] = UpdateBlkNo;
		  PN512_RF_WorkInfo.ulSendBytes = 2;
		  PN512_s_RFSetTimer(2048);
		  
		  PN512_RF_WorkInfo.ucM1Stage = 1;
	      ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);
		  PN512_RF_WorkInfo.ucM1Stage = 0;

          if(ucRet != RET_RF_ERR_TIMEOUT)
          {
                if((PN512_RF_WorkInfo.lBitsReceived == 4) || (PN512_RF_WorkInfo.lBitsReceived == 8))
		        {
		            if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0a) == 0x0)
		            {
		                  ucRet = 4;//RET_RF_ERR_NO_AUTH
		            }
					else if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0a) == 0x0a)
					{
					      ucRet = RET_RF_OK;
					}
					else if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0f) == 0x01)
					{
					      ucRet = RET_RF_ERR_VAL;
					}
					else
					{
					      ucRet = RET_RF_ERR_TRANSMIT;
					}
		        }
			    else
			    {
			        ucRet = RET_RF_ERR_TRANSMIT;
			    }
          }
	}

	return ucRet;
} 

uchar PiccSetup(uchar mode, PICC_PARA *picc_para)
{
	
	if(picc_para == NULL)
		return RET_RF_ERR_PARAM;
	
	//mode=toupper(mode);
	
	if((mode!='R'&&mode!='r') && (mode!='w'&&mode!='W'))
		return RET_RF_ERR_PARAM;
	
	
	if(PN512_RF_WorkInfo.ucRFOpen == 0)
		return RET_RF_ERR_NO_OPEN;// PN512 not powered on

	if(mode=='R' || mode=='r')
	{
		memset(picc_para,0x00,sizeof(PICC_PARA));
		memcpy(picc_para,&c_para,sizeof(PICC_PARA));
		//memcpy(picc_para->drv_ver,PN512_PCD_VER,5);
		//memcpy(picc_para->drv_date,PN512_PCD_DATE,12); 
	}
	else
	{
		if(picc_para->a_conduct_w==1)
		{
			c_para.a_conduct_val=picc_para->a_conduct_val;
			if(c_para.a_conduct_val>0x3f)c_para.a_conduct_val=0x3f;
		}
		if(picc_para->m_conduct_w==1)
		{
			c_para.m_conduct_val=picc_para->m_conduct_val;
			if(c_para.m_conduct_val>0x3f)c_para.m_conduct_val=0x3f;
		}
		if(picc_para->b_modulate_w==1)
		{
			c_para.b_modulate_val=picc_para->b_modulate_val;
			if(c_para.b_modulate_val>0x3f)c_para.b_modulate_val=0x3f;
		}
		if(picc_para->card_buffer_w==1) 
		{
			c_para.card_buffer_w=1;
			c_para.card_buffer_val=picc_para->card_buffer_val;
			if(c_para.card_buffer_val>256)c_para.card_buffer_val=256;
 		}		
 		 
		if(picc_para->card_type_check_w==1) 
		{
			c_para.card_type_check_w=1;
			c_para.card_type_check_val=picc_para->card_type_check_val;
 		}		
		
		if(picc_para->card_RxThreshold_w==1) //接收灵敏度
		{
			c_para.card_RxThreshold_w=1;
			c_para.card_RxThreshold_val=picc_para->card_RxThreshold_val;
 		}

		
		if(picc_para->f_modulate_w==1) //felica调制深度设置
		{
			c_para.f_modulate_w=1;
			c_para.f_modulate_val=picc_para->f_modulate_val;
		}
	}
	return 0;

}


uchar PiccCmdExchange(uint uiSendLen, uchar* paucInData, uint* puiRecLen, uchar* paucOutData)
{
  uchar ucRet = RET_RF_OK;

	if(PN512_RF_WorkInfo.ucRFOpen == 0)
		return EE_NO_POWER;// 模块未开启

	if(uiSendLen == 0 || paucInData == NULL || puiRecLen == NULL || paucOutData == NULL)
        {
 
            
		return EE_INVALID_PARA;
        }
	
	//for felica
	if (PN512_RF_WorkInfo.ucCurType == RF_TYPEF)
	{
		return PN512_s_RF_ucExchange_Felica(uiSendLen, paucInData, puiRecLen, paucOutData);
	}

	if(PN512_RF_WorkInfo.ucMifActivateFlag == 0)
	{
         // 卡片未激活
		 return EE_NOT_DETECTED;
	}

	
	ucRet = PN512_s_RF_ucExchange(uiSendLen, paucInData,  puiRecLen,  paucOutData);

	// need to do picc reset 解决2nd测试中第一类问题
	if(ucRet != RET_RF_OK)
	{		
			PN512_s_RFResetCarrier();
	}
	switch(ucRet)
	{
		case RET_RF_ERR_PARAM:   return EE_INVALID_PARA; // 1;
		case RET_RF_ERR_NO_OPEN: return EE_NO_POWER;     // 2;
		case RET_RF_ERR_NOT_ACT: return EE_NOT_DETECTED; // 3;
		case RET_RF_ERR_TRANSMIT: return EE_TX;          // 4
		case RET_RF_ERR_PROTOCOL:   return EE_PROTOCOL;  // 5;			
		default: 	return ucRet;							
	}
}

//add PiccInitFelica fun
uchar PiccInitFelica(uchar ucRate,uchar ucPol)
{
	return 0;
}
