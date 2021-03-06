/*
* spi driver 
*create by skx on 2010-07-02
*
*/


#include "mifre_tmdef.h"
 
#define RF_DEFAULT_WATER_LEVEL     (uchar)0x20


#define EMV_UART_BUFF_LEN	1024
extern uchar gl_aucRecData[EMV_UART_BUFF_LEN];
extern ushort gl_usRecLen;
extern void vOneTwo(unsigned char *One,unsigned short len,unsigned char *Two);

extern RF_WORKSTRUCT RF_WorkInfo;
extern PICC_PARA c_para; 

extern void ScrPrint(unsigned char col,unsigned char row,unsigned char mode,char *str,...);

ulong gl_RF_ulFSList[16] = {16,24,32,40,48,64,96,128,256,256,256,256,256,256,256,256};


void s_RF_vInitWorkInfo(void)
{
	uchar ucRFOpenFlg = 0;

	ucRFOpenFlg = RF_WorkInfo.ucRFOpen;
	memset((uchar *)&RF_WorkInfo, 0x00, sizeof(RF_WorkInfo));
	RF_WorkInfo.ucRFOpen = ucRFOpenFlg;
	
	RF_WorkInfo.ucMifCardType = RF_TYPE_S50;
	RF_WorkInfo.ucCurType = RF_TYPEA; 
	RF_WorkInfo.ucMifActivateFlag = 0;       
	RF_WorkInfo.FSC = c_para.card_buffer_val;
	RF_WorkInfo.FSD = 256;
}


void s_vRFSelectType(uchar ucRFType)
{
     uchar ucTemp = 0x00;

     switch(ucRFType)
   	 {
         case 'a':
		 case 'A':
		 	
            
			ucTemp = 0x5b;
            s_vRFWriteReg(1,RF_REG_TXCONTROL,&ucTemp); 

			ucTemp = 0x19;
            s_vRFWriteReg(1,RF_REG_CODERCONTROL,&ucTemp); 
            ucTemp = 0x73;
            s_vRFWriteReg(1,RF_REG_RXCONTROL1,&ucTemp); 
            ucTemp = 0x08;
            s_vRFWriteReg(1,RF_REG_DECODERCONTROL,&ucTemp); 
            ucTemp = 0x63;
            s_vRFWriteReg(1,RF_REG_CRCPRESETLSB,&ucTemp); 
            ucTemp = 0x63;
            s_vRFWriteReg(1,RF_REG_CRCPRESETMSB,&ucTemp); 
            ucTemp = 0xff;
            s_vRFWriteReg(1,RF_REG_RXTHRESHOLD,&ucTemp); 
            ucTemp = 0x08;
            s_vRFWriteReg(1,RF_REG_RXWAIT,&ucTemp); 
                                                    
            ucTemp = c_para.a_conduct_val;
			s_vRFWriteReg(1,RF_REG_CWCONDUCTANCE,&ucTemp);
			break;
			
		 case 'b':
		 case 'B':
		 	
            ucTemp = 0x4b;
            s_vRFWriteReg(1,RF_REG_TXCONTROL,&ucTemp); 

            ucTemp = 0x20;
            s_vRFWriteReg(1,RF_REG_CODERCONTROL,&ucTemp); 
            ucTemp = 0;
            s_vRFWriteReg(1,RF_REG_TYPEBFRAMING,&ucTemp); 
            ucTemp = 0x73;
            s_vRFWriteReg(1,RF_REG_RXCONTROL1,&ucTemp); 
            ucTemp = 0x19;
            s_vRFWriteReg(1,RF_REG_DECODERCONTROL,&ucTemp); 
            ucTemp = 0xff;
            s_vRFWriteReg(1,RF_REG_CRCPRESETLSB,&ucTemp); 
            ucTemp = 0xff;
            s_vRFWriteReg(1,RF_REG_CRCPRESETMSB,&ucTemp); 
			
			ucTemp = c_para.card_RxThreshold_val;
            s_vRFWriteReg(1,RF_REG_RXTHRESHOLD,&ucTemp); 

            ucTemp = 0x5E;
            s_vRFWriteReg(1,RF_REG_BPSKDEMCONTROL,&ucTemp); 
			    
            ucTemp = c_para.b_modulate_val ;
            s_vRFWriteReg(1,RF_REG_MODCONDUCTANCE,&ucTemp); 
                 
            ucTemp = 0x08;
            s_vRFWriteReg(1,RF_REG_RXWAIT,&ucTemp); 
                                                                                   
			break;
			
		 default:
		 	break;
   	 }

	 
	 s_RFDelay10ETUs(60);
}

uchar s_RF_ucCardTypeCheck(uchar *pCardType)
{
	uchar ucSAK = 0;

	if (RF_WorkInfo.ucUIDLen == 4)
		ucSAK = RF_WorkInfo.ucSAK1;
	else if(RF_WorkInfo.ucUIDLen == 7)
		ucSAK = RF_WorkInfo.ucSAK2;
	else if(RF_WorkInfo.ucUIDLen == 10)
		ucSAK = RF_WorkInfo.ucSAK3;
	else
		return(RET_RF_ERR_PARAM);

	if((ucSAK & 0x20) != 0x20)
	{
		if((ucSAK & 0x18) == 0x08)
		{									
			*pCardType = RF_TYPE_S50;
		}
		else if((ucSAK & 0x18) == 0x18)
		{
			*pCardType = RF_TYPE_S70;
		}
		else
		{
			return RET_RF_ERR_TRANSMIT;
		}
	}
	else
	{
		if((ucSAK & 0x18) == 0x08)
		{
			*pCardType = RF_TYPE_PRO_S50;
		}
		else if((ucSAK & 0x18) == 0x18)
		{
			*pCardType = RF_TYPE_PRO_S70;
		}
		else
		{
			*pCardType = RF_TYPE_PRO;
		}				   
	}
	
	return(RET_RF_OK);
}


uchar s_RF_ucWUPA(uchar *pucResp)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData = 0x00;

     
     s_vRFSelectType('A');
	 
	 
	  s_RFSetTimer(20);
     
          
     ucTempData = 0x03;
     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 
	 
     
     s_vRFClrBitMask(RF_REG_CONTROL, 0x08);   
     
	 s_vRFSetBitMask(RF_REG_TXCONTROL, 0x03);
     
     ucTempData = 0x07;
     s_vRFWriteReg(1,RF_REG_BITFRAMING, &ucTempData);        


	 
     RF_WorkInfo.ulSendBytes = 1;    
	 memset((uchar*)RF_WorkInfo.aucBuffer, 0, sizeof(RF_WorkInfo.aucBuffer));
     RF_WorkInfo.aucBuffer[0] = PICC_WUPA; 

	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < ( EMV_UART_BUFF_LEN- strlen(WUPA_CMD)))
	{
		strcpy(gl_aucRecData+gl_usRecLen, WUPA_CMD);
		gl_usRecLen += strlen(WUPA_CMD);
	}
#endif
	 ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);

 	 if(ucRet == RET_RF_ERR_TIMEOUT)
     {     	  
     	  RF_WorkInfo.usErrNo = 0xA1; 
          return RET_RF_ERR_TIMEOUT;
     }
	 
	 
     if((ucRet == RET_RF_OK) && (RF_WorkInfo.lBitsReceived != 16))
     {
     	 RF_WorkInfo.usErrNo = 0x14; 
         ucRet = RET_RF_ERR_TRANSMIT;
     }

     if(ucRet != RET_RF_OK)
     {
          if(ucRet == RET_RF_ERR_COLLERR)
		  {     	  	  
			  ucRet =  RET_RF_ERR_MULTI_CARD;
		  }
		  else
		  {
			  
			  ucRet = RET_RF_ERR_TRANSMIT;
		  }
     }
     else
     {
		
          if(((RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x10) && ((RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x08)
             && ((RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x04) && ((RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x02)
             && ((RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x01))
          {
               RF_WorkInfo.usErrNo = 0x10; 
               ucRet = RET_RF_ERR_PROTOCOL; 
          }
          else if((RF_WorkInfo.aucBuffer[0] & 0xC0) == 0xC0)
          {
               RF_WorkInfo.usErrNo = 0x12; 
               ucRet = RET_RF_ERR_PROTOCOL; 
          }
         else
          {
				switch(RF_WorkInfo.aucBuffer[0] & 0xC0)
				{
					case 0x00:
						 RF_WorkInfo.ucUIDLen = 4;
						 break;
					case 0x40:
						 RF_WorkInfo.ucUIDLen = 7;
						 break;
					default:
						 RF_WorkInfo.ucUIDLen = 10;
						 break;
				}
                memcpy(pucResp, (uchar*)RF_WorkInfo.aucBuffer, 2);
          }
    }

    
    if(ucRet == RET_RF_OK)    
        s_RFDelay10ETUs(6) ;
    return ucRet;
          
}


uchar s_RF_ucAnticoll(uchar ucSEL, uchar* pucUID)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData = 0x00;
     uchar ucTempVal = 0x00;
     uchar i = 0;

	 
	  s_RFSetTimer(10);
	  
	 ucTempData = 0x28;
	 s_vRFWriteReg(1, RF_REG_DECODERCONTROL,&ucTempData); 
     
     
     s_vRFClrBitMask(RF_REG_CONTROL, 0x08); 
	 
	
	 
     ucTempData = 0x03;
     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 

     
     ucTempData = 0x00;
	s_vRFWriteReg(1,RF_REG_BITFRAMING, &ucTempData);

     RF_WorkInfo.ucAnticol = ucSEL;
	 RF_WorkInfo.aucBuffer[0] = ucSEL; 
     RF_WorkInfo.aucBuffer[1] = 0x20;
     RF_WorkInfo.ulSendBytes = 2;    
     
	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN- strlen(ANTICOL_CMD)))
	{
		strcpy(gl_aucRecData+gl_usRecLen, ANTICOL_CMD);
		gl_usRecLen += strlen(ANTICOL_CMD);
	}
#endif
	 
     ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);
	
	 if(ucRet == RET_RF_ERR_TIMEOUT)  
     {
          RF_WorkInfo.usErrNo = 0xA2; 
          return RET_RF_ERR_TIMEOUT;
     }

     if(ucRet == RET_RF_OK)
     {
           if(RF_WorkInfo.lBitsReceived != 40) 
           {
                ucRet = RET_RF_ERR_TRANSMIT; 
           }
           else
           {
                switch(RF_WorkInfo.ucUIDLen)
				{
				    case 4:
					{
                       if(ucSEL == PICC_ANTICOLL1)
                       {
                             if(RF_WorkInfo.aucBuffer[0] == 0x88)
                             {
                             		RF_WorkInfo.usErrNo = 0x20; 
                                    ucRet = RET_RF_ERR_PROTOCOL; 
                             }
                       }
                       break;
					}

					case 7:
					{
                       if(ucSEL == PICC_ANTICOLL1)
                       {
                             if(RF_WorkInfo.aucBuffer[0] != 0x88)
                             {
                             		RF_WorkInfo.usErrNo = 0x21; 
                                    ucRet = RET_RF_ERR_PROTOCOL; 
                             }
                       }

					   if(ucSEL == PICC_ANTICOLL2)
                       {
                             if(RF_WorkInfo.aucBuffer[0] == 0x88)
                             {
                                    RF_WorkInfo.usErrNo = 0x22; 
                                    ucRet = RET_RF_ERR_PROTOCOL; 
                             }
                       }
					   break;
                    }
                    default:
					{
                       if(ucSEL == PICC_ANTICOLL1)
                       {
                             if(RF_WorkInfo.aucBuffer[0] != 0x88)
                             {
                             		RF_WorkInfo.usErrNo = 0x23; 
                                    ucRet = RET_RF_ERR_PROTOCOL; 
                             }
                       }

					   if(ucSEL == PICC_ANTICOLL2)
                       {
                             if(RF_WorkInfo.aucBuffer[0] != 0x88)
                             {
                             		RF_WorkInfo.usErrNo = 0x24; 
                                    ucRet = RET_RF_ERR_PROTOCOL; 
                             }
                       }
					   break;
					}
				}

                
                ucTempVal = 0;
                for(i = 0; i < 4; i++)
                {
                      ucTempVal ^= RF_WorkInfo.aucBuffer[i];
                }
                if(ucTempVal != RF_WorkInfo.aucBuffer[4])
                {
                      RF_WorkInfo.usErrNo = 0x25; 
                      ucRet = RET_RF_ERR_TRANSMIT;
                }
         }

         memcpy(pucUID,(uchar*)RF_WorkInfo.aucBuffer,5);
     }
     else if(ucRet == RET_RF_ERR_COLLERR)
     {
         ucRet = RET_RF_ERR_MULTI_CARD;
     }
	 else
	 {
		 ucRet = RET_RF_ERR_TRANSMIT;
	 }

	 RF_WorkInfo.ucAnticol = 0;
	 
    if(ucRet == RET_RF_OK)    
        s_RFDelay10ETUs(6) ;
     return ucRet;
}



uchar s_RF_ucSelect(uchar ucSEL, uchar* pucUID, uchar* pucSAK)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData = 0x00;

     
	  s_RFSetTimer(10);
	  
	 
     ucTempData = 0x0F;
     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 

     
     s_vRFClrBitMask(RF_REG_CONTROL, 0x08); 

	 
     
     ucTempData = 0x00;
	 s_vRFWriteReg(1,RF_REG_BITFRAMING, &ucTempData);

     RF_WorkInfo.ulSendBytes = 7;
     
     memcpy((uchar*)RF_WorkInfo.aucBuffer+2, pucUID, 5);

	 RF_WorkInfo.aucBuffer[0] = ucSEL;
     RF_WorkInfo.aucBuffer[1] = 0x70;       

	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN- strlen(SELECT_CMD)))
	{
		strcpy(gl_aucRecData+gl_usRecLen, SELECT_CMD);
		gl_usRecLen += strlen(SELECT_CMD);
	}
#endif

	 
     ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);
     if(ucRet == RET_RF_ERR_TIMEOUT) 
     {
          RF_WorkInfo.usErrNo = 0xA3; 
          return RET_RF_ERR_TIMEOUT;
     }

     if(ucRet != RET_RF_OK)
     {
          ucRet = RET_RF_ERR_TRANSMIT;
     }
     else
     {
           if(RF_WorkInfo.lBitsReceived != 8)
           {
           		RF_WorkInfo.usErrNo = 0x30; 
               ucRet = RET_RF_ERR_TRANSMIT;
           }
           else
           {
			   
			   switch(RF_WorkInfo.ucUIDLen)
			   {
			       case 4:
				   {
                       if((RF_WorkInfo.aucBuffer[0] & 0x04) == 0x04)
					   {
					   		 RF_WorkInfo.usErrNo = 0x31; 
							 ucRet = RET_RF_ERR_PROTOCOL;
					   }
					   RF_WorkInfo.ucSAK1 = RF_WorkInfo.aucBuffer[0];
					   break;
				   }

				   case 7:
				   {
                       if(ucSEL == PICC_ANTICOLL1)
					   {
						   if((RF_WorkInfo.aucBuffer[0] & 0x04) != 0x04)
						   {
					   		     RF_WorkInfo.usErrNo = 0x32; 
								 ucRet = RET_RF_ERR_PROTOCOL;
						   }
						   RF_WorkInfo.ucSAK1 = RF_WorkInfo.aucBuffer[0];
					   }

					   if(ucSEL == PICC_ANTICOLL2)
					   {
						   if((RF_WorkInfo.aucBuffer[0] & 0x04) == 0x04)
						   {
					   		     RF_WorkInfo.usErrNo = 0x33; 
								 ucRet = RET_RF_ERR_PROTOCOL;
						   }
						   RF_WorkInfo.ucSAK2 = RF_WorkInfo.aucBuffer[0];
					   }
					   break;
				   }

				   default:
				   {
					   if(ucSEL == PICC_ANTICOLL1)
					   {
						   if((RF_WorkInfo.aucBuffer[0] & 0x04) != 0x04)
						   {
					   		     RF_WorkInfo.usErrNo = 0x34; 
								 ucRet = RET_RF_ERR_PROTOCOL;
						   }
						   RF_WorkInfo.ucSAK1 = RF_WorkInfo.aucBuffer[0];
					   }

					   if(ucSEL == PICC_ANTICOLL2)
					   {
						   if((RF_WorkInfo.aucBuffer[0] & 0x04) != 0x04)
						   {
					   		     RF_WorkInfo.usErrNo = 0x35; 
								 ucRet = RET_RF_ERR_PROTOCOL;
						   }
						   RF_WorkInfo.ucSAK2 = RF_WorkInfo.aucBuffer[0];
					   }

					   if(ucSEL == PICC_ANTICOLL3)
					   {
						   if((RF_WorkInfo.aucBuffer[0] & 0x04) == 0x04)
						   {
					   		     RF_WorkInfo.usErrNo = 0x36; 
								 ucRet = RET_RF_ERR_PROTOCOL;
						   }
						   RF_WorkInfo.ucSAK3 = RF_WorkInfo.aucBuffer[0];
					   }
					   break;
				   }
			   }
           }
      }
	  *pucSAK = RF_WorkInfo.aucBuffer[0];
	  	
    
    if(ucRet == RET_RF_OK)    
        s_RFDelay10ETUs(6) ;
      return ucRet;
}


uchar s_RF_ucGetUID(void)
{
       uchar ucRet = RET_RF_OK;
       uchar ucTempData = 0x00;
       uchar ucUIDTemp[5];
       uchar ucSAK = 0x00;
	   uchar i = 0;

       memset(ucUIDTemp, 0x00, sizeof(ucUIDTemp));

	   ucTempData = PICC_ANTICOLL1; 
	   for(i = 0; i<3; i++)
	   {
	        ucRet = s_RF_ucAnticoll(ucTempData, ucUIDTemp);
			if(ucRet != RET_RF_ERR_TIMEOUT)
				break;
	   }
       if(ucRet != RET_RF_OK)
       {
		   return ucRet;
       }

	   for(i = 0; i<3; i++)
	   {
			ucRet = s_RF_ucSelect(ucTempData, ucUIDTemp, &ucSAK);
			if(ucRet != RET_RF_ERR_TIMEOUT)
				break;
	   }
	   if(ucRet != RET_RF_OK)
	   {
		   return ucRet;
	   }

	   
       memcpy((uchar*)RF_WorkInfo.ucUIDCL1, ucUIDTemp, 5);
	   if(RF_WorkInfo.ucUIDLen == 4)
	   {
            return RET_RF_OK;
	   }

       
 	   ucTempData = PICC_ANTICOLL2;
	   for(i = 0; i<3; i++)
	   {
			ucRet = s_RF_ucAnticoll(ucTempData, ucUIDTemp);
			if(ucRet != RET_RF_ERR_TIMEOUT)
				break;
	   }
	   
	   if(ucRet != RET_RF_OK)
	   {
		   return ucRet;
	   }

	   for(i = 0; i<3; i++)
	   {
			ucRet = s_RF_ucSelect(ucTempData, ucUIDTemp, &ucSAK);
			if(ucRet != RET_RF_ERR_TIMEOUT)
				break;
	   }
	   
       if(ucRet != RET_RF_OK)
	   {
		   return ucRet;
	   }
       
       memcpy((uchar*)RF_WorkInfo.ucUIDCL2, ucUIDTemp, 5);

       if(RF_WorkInfo.ucUIDLen == 7)
	   {
		    return RET_RF_OK;
	   }

       
	   ucTempData = PICC_ANTICOLL3;
	   
	   for(i = 0; i<3; i++)
	   {
			ucRet = s_RF_ucAnticoll(ucTempData, ucUIDTemp);
			if(ucRet != RET_RF_ERR_TIMEOUT)
				break;
	   }
	   
	   if(ucRet != RET_RF_OK)
	   {
			return ucRet;
	   }

	   for(i = 0; i<3; i++)
	   {
			ucRet = s_RF_ucSelect(ucTempData, ucUIDTemp, &ucSAK);
			if(ucRet != RET_RF_ERR_TIMEOUT)
				break;
	   }	   
       
	   if(ucRet != RET_RF_OK)
	   {
		   return ucRet;
	   }

	   
       memcpy((uchar*)RF_WorkInfo.ucUIDCL3, ucUIDTemp, 5);

	   return RET_RF_OK;
}


uchar s_RF_ucActPro(uchar* pucOutLen, uchar* pucATSData)
{
       uchar ucRet = RET_RF_OK;
       uchar ucTempData = 0x00;
       uchar ucSAK = 0x00;
       uchar ucResp[2];
       uchar ucReSendNo = 0;

       
       for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
       {
           
           ucRet = s_RF_ucWUPA(ucResp);
		   if(ucRet != RET_RF_ERR_TIMEOUT)
           {
                 break;
           }
       }

       
       if(ucRet != RET_RF_OK)
       {
           RF_WorkInfo.usErrNo = 0x70; 
		   return ucRet;
       }
       else
       {
             
             if(memcmp(ucResp, (uchar*)RF_WorkInfo.ucATQA, 2))
             {
             		RF_WorkInfo.usErrNo = 0x71; 
                    return RET_RF_ERR_PROTOCOL;
             }
       }

       
	   for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	   {
           ucTempData = PICC_ANTICOLL1;
           ucRet = s_RF_ucSelect(ucTempData, (uchar*)RF_WorkInfo.ucUIDCL1, &ucSAK);
		   if(ucRet != RET_RF_ERR_TIMEOUT)
           {
                break;
           }
	   }

       if(ucRet != RET_RF_OK)
       {
       	   RF_WorkInfo.usErrNo |= 0x100; 
		   return ucRet;
	   }
       
	   if(RF_WorkInfo.ucUIDLen == 4)
	   {
		   goto ACTEND;
	   }

       
       for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	   {
          ucTempData = PICC_ANTICOLL2;
          ucRet = s_RF_ucSelect(ucTempData, (uchar*)RF_WorkInfo.ucUIDCL2, &ucSAK);
          if(ucRet != RET_RF_ERR_TIMEOUT)
          {
                break;
          }
	   }

       if(ucRet != RET_RF_OK)
	   {
	       RF_WorkInfo.usErrNo |= 0x200; 
		   return ucRet;
	   }

       
       if(RF_WorkInfo.ucUIDLen == 7)
	   {
		   goto ACTEND;
	   }

       
       for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	   {
          ucTempData = PICC_ANTICOLL3;
          ucRet = s_RF_ucSelect(ucTempData, (uchar*)RF_WorkInfo.ucUIDCL3, &ucSAK);
          if(ucRet != RET_RF_ERR_TIMEOUT)
          {
                break;
          }
	   }

       if(ucRet != RET_RF_OK)
	   {
	       RF_WorkInfo.usErrNo |= 0x300; 
		   return ucRet;
	   }

ACTEND: 
	   
       for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
       {
            ucRet = s_RF_ucRATS(pucOutLen, pucATSData);
			if(ucRet != RET_RF_ERR_TIMEOUT)
            {
                 break;
            }
       }

       if(ucRet != RET_RF_OK)
	   {
		   return ucRet;
	   }

	   RF_WorkInfo.ucMifActivateFlag = 1;
	   RF_WorkInfo.ucCurPCB = 1;

	   return RET_RF_OK;
}


uchar s_RF_ucRATS(uchar* pucOutLen, uchar* pucATSData)
{
    uchar ucTempData = 0x00;
    uchar ucRet = RET_RF_OK;
    uchar tucTemp[2];
    uchar i = 0;
	uchar ucResend = 0;
    uint  uiFWITemp = 0x00, uiSFGITemp = 0x00;
    *pucOutLen = 0;
   
RATS_BEGIN:
    RF_WorkInfo.FSC    = 32;
    RF_WorkInfo.FSD    = 256;
	RF_WorkInfo.ucFWI  = 4;
    RF_WorkInfo.ulFWT  = 560; 
    RF_WorkInfo.ucSFGI = 0;
    RF_WorkInfo.ulSFGT = 60; 
    
    RF_WorkInfo.ucCIDFlag = 0;
    RF_WorkInfo.ucNADFlag = 0;

    tucTemp[0] = 0xE0;
    tucTemp[1] = 0x80;  

    
    s_RFSetTimer(RF_FWTRATS); 
    
     ucTempData = 0x0F;
     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 

    
    RF_WorkInfo.ulSendBytes = 2;     
    memcpy((uchar*)RF_WorkInfo.aucBuffer, tucTemp, RF_WorkInfo.ulSendBytes);

	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN- strlen(RATS_CMD)))
	{
		strcpy(gl_aucRecData+gl_usRecLen, RATS_CMD);
		gl_usRecLen += strlen(RATS_CMD);
	}
#endif

	
	RF_WorkInfo.ucM1Stage = 1; 
    ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);
	RF_WorkInfo.ucM1Stage = 0; 
	ucResend++;
	
    if(ucRet == RET_RF_ERR_TIMEOUT)
    {
          RF_WorkInfo.usErrNo = 0xA4; 
          return RET_RF_ERR_TIMEOUT;
    }

	if(ucRet != RET_RF_OK) 
	{
		
		if(ucRet == RET_RF_ERR_PARITY || ucRet == RET_RF_ERR_CRC || ucRet == RET_RF_ERR_FRAMING)
		{
			if(ucResend < 3 && RF_WorkInfo.ulBytesReceived <= 3) 
			   goto RATS_BEGIN; 
		}
		  return RET_RF_ERR_TRANSMIT;
	}
	else
    {    		
           i = 0;
           if(RF_WorkInfo.ulBytesReceived != RF_WorkInfo.aucBuffer[0]) 
           {
                RF_WorkInfo.usErrNo = 0x80; 
                ucRet = RET_RF_ERR_TRANSMIT;
				goto RATSEND;
		   }
           
           *pucOutLen = RF_WorkInfo.aucBuffer[0];
           memcpy(pucATSData,(uchar*)RF_WorkInfo.aucBuffer,RF_WorkInfo.ulBytesReceived);

           
           if(RF_WorkInfo.aucBuffer[0] <= 1) 
           {
			   goto RATSEND;
           }

           i = 1; 

           
           if((RF_WorkInfo.aucBuffer[1] & 0x80) != 0x00) 
           {
                RF_WorkInfo.usErrNo = 0x81; 
			    ucRet = RET_RF_ERR_PROTOCOL;
                goto RATSEND;
           }
           else
           {
                
			    RF_WorkInfo.FSC = gl_RF_ulFSList[RF_WorkInfo.aucBuffer[1] & 0x0F];
           }
           
		   
           if((RF_WorkInfo.aucBuffer[1] & 0x10) == 0x10)  
           {
                i++;
           }
           
           if((RF_WorkInfo.aucBuffer[1] & 0x20) == 0x20) 
           {
               i++;
               RF_WorkInfo.ucFWI = (RF_WorkInfo.aucBuffer[i] & 0x0F0) >> 4;

			   uiFWITemp = (uint)0x01 << RF_WorkInfo.ucFWI;
			   RF_WorkInfo.ulFWT = (ulong)(32 + 3) * uiFWITemp; 

			   RF_WorkInfo.ucSFGI = RF_WorkInfo.aucBuffer[i] & 0x0F;

			   uiSFGITemp = (uint)0x01 << RF_WorkInfo.ucSFGI;
			   RF_WorkInfo.ulSFGT = (ulong)(32 + 3) * uiSFGITemp; 
           }

           if((RF_WorkInfo.aucBuffer[1] & 0x40) == 0x40)  
           {
               i++;
               
               
               if(RF_WorkInfo.aucBuffer[i] & 0x02)
               {
                   RF_WorkInfo.ucCIDFlag = 1;
               }
               else
               {
                   RF_WorkInfo.ucCIDFlag = 0;
               }
               
               if(RF_WorkInfo.aucBuffer[i] & 0x01)
               {
                   RF_WorkInfo.ucNADFlag = 1;
               }
               else
               {
                   RF_WorkInfo.ucNADFlag = 0;
               }

               ucRet = RET_RF_OK;
           }
    }

RATSEND:

	if (ucRet == RET_RF_OK)
	{
		RF_WorkInfo.ucInProtocol = 1;
		if(c_para.card_buffer_w == 1 && c_para.card_buffer_val)
			RF_WorkInfo.FSC = c_para.card_buffer_val;
	}
	
    s_RFDelay10ETUs(RF_WorkInfo.ulSFGT/10); 
		
    return ucRet;
}



void s_RF_vHALTA(void)
{
    uchar ucRet = RET_RF_OK;
    uchar ucTempData;
	
     ucTempData = 0x00;
	 s_vRFWriteReg(1,RF_REG_BITFRAMING, &ucTempData);
	
	  s_RFSetTimer(10);

    
     ucTempData = 0x07;
     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 

    
    RF_WorkInfo.ulSendBytes = 2;
    RF_WorkInfo.aucBuffer[0] = PICC_HALT ;     
    RF_WorkInfo.aucBuffer[1] = 0x00;
	
	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN- strlen(HALTA_CMD)))
	{
		strcpy(gl_aucRecData+gl_usRecLen, HALTA_CMD);
		gl_usRecLen += strlen(HALTA_CMD);
	}
#endif

    
    ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);
   
    
	

}


uchar  s_RF_ucWUPB(uchar* pucOutLen, uchar* pucOut)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData;
	 uint uiFWITemp = 0x00;
	 uint uiSFGITemp = 0;

    RF_WorkInfo.ucSFGI = 0;
    RF_WorkInfo.ulSFGT = 60;
    
	 s_vRFFlushFIFO();        

     s_vRFSelectType('B');
	 
	 
     ucTempData = 0x2C;
     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 
       
     
     s_vRFClrBitMask(RF_REG_CONTROL, 0x08); 
 
	 s_RFSetTimer(65);
	  
	
     RF_WorkInfo.ulSendBytes = 3;    
     RF_WorkInfo.aucBuffer[0] = PICC_APF;
     RF_WorkInfo.aucBuffer[1] = 0x00;       
     RF_WorkInfo.aucBuffer[2] = PICC_WUPB;  
     

	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN- strlen(WUPB_CMD)))
	{ 
		strcpy(gl_aucRecData+gl_usRecLen, WUPB_CMD);
		gl_usRecLen += strlen(WUPB_CMD);
	}
#endif

     ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);

     if(ucRet == RET_RF_ERR_TIMEOUT)
     {
           RF_WorkInfo.usErrNo = 0xA5; 
           return RET_RF_ERR_TIMEOUT;
     }
     if(ucRet != RET_RF_OK)
     {
           ucRet = RET_RF_ERR_TRANSMIT;
     }
     else
     {
		    if(RF_WorkInfo.ulBytesReceived != 12)
            {
             	  RF_WorkInfo.usErrNo = 0x50; 
                  ucRet = RET_RF_ERR_TRANSMIT;
				  goto WUPBEND;
            }
            if(RF_WorkInfo.aucBuffer[0] != 0x50)
            {
             	  RF_WorkInfo.usErrNo = 0x51; 
				  ucRet = RET_RF_ERR_PROTOCOL;
                  goto WUPBEND;
            }

		    *pucOutLen = RF_WorkInfo.ulBytesReceived;
            memcpy(pucOut, (uchar*)RF_WorkInfo.aucBuffer, RF_WorkInfo.ulBytesReceived);

            RF_WorkInfo.ucUIDLen = 4;
            
            memcpy((uchar*)RF_WorkInfo.ucUIDB, (uchar*)RF_WorkInfo.aucBuffer + 1, 4);

            

            
            
           			
            
			RF_WorkInfo.FSC = gl_RF_ulFSList[(RF_WorkInfo.aucBuffer[10] >> 4) & 0x0F];

            
            if((RF_WorkInfo.aucBuffer[10] & 0x01) != 0x01) 
            {
             	   RF_WorkInfo.usErrNo = 0x53; 
                   ucRet = RET_RF_ERR_PROTOCOL;
				   goto WUPBEND;
            }

            
            
            RF_WorkInfo.ucFWI = (RF_WorkInfo.aucBuffer[11] & 0x0F0) >> 4;

			uiFWITemp = (uint)0x01 << RF_WorkInfo.ucFWI;
			RF_WorkInfo.ulFWT = (ulong)(32 + 3) * uiFWITemp; 

            
            
            
            if(RF_WorkInfo.aucBuffer[11] & 0x01)
            {
                 RF_WorkInfo.ucCIDFlag = 1;
            }
            else
            {
                 RF_WorkInfo.ucCIDFlag = 0;
            }
            
            if(RF_WorkInfo.aucBuffer[11] & 0x02)
            {
                 RF_WorkInfo.ucNADFlag = 1;
            }
            else
            {
                 RF_WorkInfo.ucNADFlag = 0;
            }
     }

WUPBEND:
	 if(ucRet == 0)
	 {
	 	if(c_para.card_buffer_w == 1 && c_para.card_buffer_val)
			RF_WorkInfo.FSC = c_para.card_buffer_val;
	 }
	
    if(ucRet == RET_RF_OK)    
        s_RFDelay10ETUs(6) ;
     return ucRet;

}

uchar  s_RF_ucREQB(uchar* pucOutLen, uchar* pucOut)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData;
	 uint uiFWITemp = 0x00;
	 uint uiSFGITemp = 0;

    RF_WorkInfo.ucSFGI = 0;
    RF_WorkInfo.ulSFGT = 60; 
    
	 s_vRFFlushFIFO();        

     s_vRFSelectType('B');
	 
	 
     ucTempData = 0x2C;
     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 
       
     
     s_vRFClrBitMask(RF_REG_CONTROL, 0x08); 
 
	 s_RFSetTimer(65);
	  
     RF_WorkInfo.ulSendBytes = 3;    
     RF_WorkInfo.aucBuffer[0] = PICC_APF;
     RF_WorkInfo.aucBuffer[1] = 0x00;       
     RF_WorkInfo.aucBuffer[2] = PICC_REQB;  
    

	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN- strlen(REQB_CMD)))
	{ 
		strcpy(gl_aucRecData+gl_usRecLen, REQB_CMD);
		gl_usRecLen += strlen(REQB_CMD);
	}
#endif

     ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);

     if(ucRet == RET_RF_ERR_TIMEOUT)
     {
           RF_WorkInfo.usErrNo = 0xA5; 
           return RET_RF_ERR_TIMEOUT;
     }
     if(ucRet != RET_RF_OK)
     {
           ucRet = RET_RF_ERR_TRANSMIT;
     }
     else
     { 
		    if(RF_WorkInfo.ulBytesReceived != 12)
            {
             	  RF_WorkInfo.usErrNo = 0x50; 
                  ucRet = RET_RF_ERR_TRANSMIT;
				  goto WUPBEND;
            }
            if(RF_WorkInfo.aucBuffer[0] != 0x50)
            {
             	  RF_WorkInfo.usErrNo = 0x51; 
				  ucRet = RET_RF_ERR_PROTOCOL;
                  goto WUPBEND;
            }

		    *pucOutLen = RF_WorkInfo.ulBytesReceived;
            memcpy(pucOut, (uchar*)RF_WorkInfo.aucBuffer, RF_WorkInfo.ulBytesReceived);

            RF_WorkInfo.ucUIDLen = 4;
            
            memcpy((uchar*)RF_WorkInfo.ucUIDB, (uchar*)RF_WorkInfo.aucBuffer + 1, 4);

            
			RF_WorkInfo.FSC = gl_RF_ulFSList[(RF_WorkInfo.aucBuffer[10] >> 4) & 0x0F];

            
            if((RF_WorkInfo.aucBuffer[10] & 0x01) != 0x01) 
            {
             	   RF_WorkInfo.usErrNo = 0x53; 
                   ucRet = RET_RF_ERR_PROTOCOL;
				   goto WUPBEND;
            }

           
            RF_WorkInfo.ucFWI = (RF_WorkInfo.aucBuffer[11] & 0x0F0) >> 4;

			uiFWITemp = (uint)0x01 << RF_WorkInfo.ucFWI;
			RF_WorkInfo.ulFWT = (ulong)(32 + 3) * uiFWITemp; 
            
            if(RF_WorkInfo.aucBuffer[11] & 0x01)
            {
                 RF_WorkInfo.ucCIDFlag = 1;
            }
            else
            {
                 RF_WorkInfo.ucCIDFlag = 0;
            }
            
            if(RF_WorkInfo.aucBuffer[11] & 0x02)
            {
                 RF_WorkInfo.ucNADFlag = 1;
            }
            else
            {
                 RF_WorkInfo.ucNADFlag = 0;
            } 
     }

WUPBEND:
	 if(ucRet == 0)
	 {
	 	if(c_para.card_buffer_w == 1 && c_para.card_buffer_val)
			RF_WorkInfo.FSC = c_para.card_buffer_val;
	 }
	
    if(ucRet == RET_RF_OK)    
        s_RFDelay10ETUs(6) ; 
     return ucRet;

}


uchar s_RF_ucAttrib(uchar* pucResp)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData = 0x00;
	 uchar ucResend = 0;

	*pucResp = 0;
	
ATTRIB_BEGIN:
     s_vRFFlushFIFO();        

     
     ucTempData = 0x2C;
     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 

	  s_RFSetTimer(RF_WorkInfo.ulFWT+10);

	 RF_WorkInfo.aucBuffer[0] = PICC_ATTRIB; 
     
     RF_WorkInfo.aucBuffer[1] = RF_WorkInfo.ucUIDB[0];
     RF_WorkInfo.aucBuffer[2] = RF_WorkInfo.ucUIDB[1];
     RF_WorkInfo.aucBuffer[3] = RF_WorkInfo.ucUIDB[2];
     RF_WorkInfo.aucBuffer[4] = RF_WorkInfo.ucUIDB[3];
     
     
     RF_WorkInfo.aucBuffer[5] = 0x00;
     
     
     RF_WorkInfo.aucBuffer[6] = 0x08;

     
     RF_WorkInfo.aucBuffer[7] = 0x01;
     
     RF_WorkInfo.aucBuffer[8] = 0x00;

     RF_WorkInfo.ulSendBytes = 9;    

	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN- strlen(ATTRIB_CMD)))
	{ 
		strcpy(gl_aucRecData+gl_usRecLen, ATTRIB_CMD);
		gl_usRecLen += strlen(ATTRIB_CMD);
	}
#endif
	
     ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);
	ucResend++;

     if(ucRet == RET_RF_ERR_TIMEOUT)
     {
           return RET_RF_ERR_TIMEOUT;
     }
     else if(ucRet != RET_RF_OK)
     {
		
		if(ucRet == RET_RF_ERR_PARITY || ucRet == RET_RF_ERR_CRC || ucRet == RET_RF_ERR_FRAMING)
		{
			
			
		
		
		}
           
           ucRet = RET_RF_ERR_TRANSMIT;
     }
     else
     {
           if(RF_WorkInfo.ulBytesReceived != 1)
           {
           		RF_WorkInfo.usErrNo = 0x95; 
                ucRet = RET_RF_ERR_TRANSMIT;
           }
           else
           {
			    if((RF_WorkInfo.aucBuffer[0] & 0x0F) != 0x00)
                {
                		RF_WorkInfo.usErrNo = 0x96; 
                       
                       ucRet = RET_RF_ERR_PROTOCOL;
                }
           }

           *pucResp = RF_WorkInfo.aucBuffer[0];
     }

	if (ucRet == RET_RF_OK)
	{
		RF_WorkInfo.ucInProtocol = 1;
	}

    return ucRet;
}


uchar s_RF_ucHALTB(uchar* pucResp)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData = 0x00;

     s_vRFFlushFIFO();

     
     ucTempData = 0x2C;
     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 

     s_RFSetTimer(RF_WorkInfo.ulFWT + 10);

     
     RF_WorkInfo.ulSendBytes = 5;    
     RF_WorkInfo.aucBuffer[0] = PICC_HALT; 
     
     RF_WorkInfo.aucBuffer[1] = RF_WorkInfo.ucUIDB[0];
     RF_WorkInfo.aucBuffer[2] = RF_WorkInfo.ucUIDB[1];
     RF_WorkInfo.aucBuffer[3] = RF_WorkInfo.ucUIDB[2];
     RF_WorkInfo.aucBuffer[4] = RF_WorkInfo.ucUIDB[3];

	 ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);

     if(ucRet == RET_RF_OK)
     {
         if(RF_WorkInfo.ulBytesReceived != 1)
		 {
               ucRet = RET_RF_ERR_TRANSMIT;
		 }
		 else if(RF_WorkInfo.aucBuffer[0] == 0x0)
         {
               *pucResp = RF_WorkInfo.aucBuffer[0];
         }
         else
         {
               ucRet = RET_RF_ERR_PROTOCOL;
         }
     }
     else if(ucRet == RET_RF_ERR_TIMEOUT)
	 {
		 ucRet = RET_RF_ERR_TIMEOUT;
	 }
	 else
     {
         ucRet = RET_RF_ERR_TRANSMIT;
     }

     return ucRet;
}



uchar s_RF_ucActTypeB(uchar* pucOutLen, uchar* pucOut)
{
      uchar ucRet = RET_RF_OK;
      uchar ucTempData = 0x00;
      uchar ucReSendNo = 0;

      
      for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
      {
            ucRet = s_RF_ucWUPB(pucOutLen, pucOut);
            if(ucRet != RET_RF_ERR_TIMEOUT)
            {
                  break;
            }
      }

      if(ucRet != RET_RF_OK)
	  {
           RF_WorkInfo.usErrNo = 0x90; 
		   return ucRet;
	  }

      
      if(memcmp(pucOut, (uchar*)RF_WorkInfo.ucATQB, 12))
	  {
	  	   RF_WorkInfo.usErrNo = 0x91; 
           return RET_RF_ERR_PROTOCOL;
	  }

      
      for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
      {
           ucRet = s_RF_ucAttrib(&ucTempData);
           if(ucRet != RET_RF_ERR_TIMEOUT)
           {
                 break;
           }
      }

      if(ucRet != RET_RF_OK)
	  {
	  	   RF_WorkInfo.usErrNo |= 0x400; 
		   return ucRet;
	  }

      RF_WorkInfo.ucMifActivateFlag = 1;
      RF_WorkInfo.ucCurPCB = 1;

      return RET_RF_OK;
}


uchar s_RF_ucPOLL(uchar* pucMifType)
{
       uchar ucRet = RET_RF_OK;
       uchar ucTempData = 0x00;
       uchar ucATQA[2]; 
       uchar ucATQB[12]; 
	   uchar i = 0;
	   uchar ucSAK; 


       memset(ucATQA, 0x00, sizeof(ucATQA));
       memset(ucATQB, 0x00, sizeof(ucATQB));

       
       ucRet = s_RF_ucWUPA(ucATQA);

	   
	   if(ucRet == RET_RF_OK)
       {
           
            RF_WorkInfo.ucATQA[0] = ucATQA[0];
            RF_WorkInfo.ucATQA[1] = ucATQA[1];

			
            ucRet = s_RF_ucGetUID();
			
			if(ucRet == RET_RF_OK)
            {
                    
                    s_RF_vHALTA();
					
					ucRet = s_RF_ucREQA(ucATQA); 
	
					if (ucRet != RET_RF_ERR_TIMEOUT) 
					{
						return RET_RF_ERR_MULTI_CARD;
					}
				
                    
                    ucRet = s_RF_ucWUPB(&ucTempData,ucATQB);
					
					if(ucRet != RET_RF_ERR_TIMEOUT)
                    {                    
                        RF_WorkInfo.usErrNo = 0x41; 
                        
						return RET_RF_ERR_MULTI_CARD;
                    }

                    
					s_RF_ucCardTypeCheck(pucMifType);

					
					RF_WorkInfo.ucMifCardType = *pucMifType;
					RF_WorkInfo.ucCurType = RF_TYPEA; 
					
					
                    return RET_RF_OK;
            }
            else
            {
				return ucRet;
            }
       }
       else if(ucRet == RET_RF_ERR_TIMEOUT)
       {
            
            ucRet = s_RF_ucWUPB(&ucTempData,ucATQB);
			if(ucRet == RET_RF_OK)
            {
                   
                   memcpy((uchar*)RF_WorkInfo.ucATQB, ucATQB, ucTempData);
                   
				   for (i = 0; i < 3; i++) 
				   {
					   ucRet = s_RF_ucHALTB(&ucTempData);
					   if (ucRet != RET_RF_ERR_TIMEOUT) 
					   {
						   break;
					   }
				   }
				   
                   if(ucRet == RET_RF_OK)
                   {
					   ucRet = s_RF_ucREQB(&ucTempData, ucATQB);
					   if (ucRet != RET_RF_ERR_TIMEOUT)
					   {
						   return RET_RF_ERR_MULTI_CARD;
					   }

                        ucRet = s_RF_ucWUPA(ucATQA);
                        if(ucRet != RET_RF_ERR_TIMEOUT)
                        {                        
             	            RF_WorkInfo.usErrNo = 0x60; 
                            
							return RET_RF_ERR_MULTI_CARD;
                        }

                        
                        RF_WorkInfo.ucMifCardType = RF_TYPE_B;
						*pucMifType = RF_WorkInfo.ucMifCardType;
						RF_WorkInfo.ucCurType = RF_TYPEB;
                        return RET_RF_OK;
                   }
                   else
				   {
                        return ucRet;
				   }
            }
			if(ucRet == RET_RF_ERR_TIMEOUT)
			{
				return E_NOT_DETECTED; 
			}
			return ucRet;
       }

	   
	   return ucRet;

}


uchar  s_RF_ucIfCardExit(void)
{
       uchar ucRet = RET_RF_OK;
       uchar ucTempData[12];
       uchar ucTemp = 0x00;
       uchar ucReSendNo = 0;

       if(RF_WorkInfo.ucCurType == RF_TYPEA)
       { 
            
            for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
            {
                  ucRet = s_RF_ucWUPA(ucTempData);
                  if(ucRet != RET_RF_ERR_TIMEOUT)
                  { 
                       break;
                  }

                  if((ucRet == RET_RF_ERR_TIMEOUT) && (ucReSendNo == 2))
                  { 
                        
                        return RET_RF_OK;
                  }
            }
            
            s_RF_vHALTA(); 
            return RET_RF_ERR_CARD_EXIST;
       }
       else
       {
            
            for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
            { 
                  ucRet = s_RF_ucWUPB(&ucTemp, ucTempData);
				  if(ucRet != RET_RF_ERR_TIMEOUT)
                  {
                       break;
                  }
                  if((ucRet == RET_RF_ERR_TIMEOUT) && (ucReSendNo == 2))
                  {
                        
                        return RET_RF_OK;
                  }
            }
           

           {
                 ucRet = s_RF_ucHALTB(&ucTemp);
                 if(ucRet != RET_RF_ERR_TIMEOUT)
                 {          
					 return RET_RF_ERR_CARD_EXIST; 
					 
                 }				 
				 
                 else 
                 {
                        return RET_RF_OK;
                 }

           }
       }

	   return RET_RF_ERR_CARD_EXIST;
}


uchar s_RF_ucProcess(uint uiSendLen, uint* puiRecLen)
{
       uchar ucRet = RET_RF_OK;
       uchar ucTempData = 0x00;

       
	   s_RFSetTimer(RF_WorkInfo.ulFWT+3); 


       if(RF_WorkInfo.ucCurType == RF_TYPEA)
       {
		     ucTempData = 0x0F;
		     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 
       }
       else
       {
		     ucTempData = 0x2C;
		     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 
       }


		
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN- strlen(CPDU_CMD)))
		{
			strcpy(gl_aucRecData+gl_usRecLen, CPDU_CMD);
			gl_usRecLen += strlen(CPDU_CMD);
		}
#endif
		
        
        RF_WorkInfo.ulSendBytes = uiSendLen;    
        ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);
 
        if(ucRet == RET_RF_ERR_TIMEOUT)
		{
			ucRet = RET_RF_ERR_TIMEOUT;
		}
		else if(ucRet != RET_RF_OK)
        {
			if (ucRet == RET_RF_ERR_CRC || ucRet == RET_RF_ERR_PARITY)
			{
				
				if(RF_WorkInfo.ulBytesReceived < 1)
     				s_RFDelay10ETUs(RF_WorkInfo.ulFWT/10) ;
			}
			
			ucRet = RET_RF_ERR_TRANSMIT;
        }

        if(ucRet == RET_RF_OK)
        {
		    *puiRecLen = RF_WorkInfo.ulBytesReceived;
		    if(RF_WorkInfo.ulBytesReceived == 0) 
			{
				
				ucRet = RET_RF_ERR_TRANSMIT;
			}
            			
        }

        if(ucRet == RET_RF_OK) 
     		s_RFDelay10ETUs(6) ;
        return( ucRet);
}


uchar s_RF_ucDeselect(void)
{
      uchar ucRet = RET_RF_OK;
      uchar ucTempData = 0x00;
      uchar i = 0;

      if(RF_WorkInfo.ucCurType == RF_TYPEA)
      {
		     ucTempData = 0x0F;
		     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 
       }
       else
       {
		     ucTempData = 0x2C;
		     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 
       }

      while(1)
      {
          
          s_RFSetTimer(RF_FWTDESELECT+2); 

          RF_WorkInfo.aucBuffer[0] = 0xC2; 

          
          RF_WorkInfo.ulSendBytes = 1;   
          ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);

          if((ucRet != RET_RF_OK) && (ucRet != RET_RF_ERR_TIMEOUT))
          {
                 ucRet = RET_RF_ERR_TRANSMIT;
          }

		  if((ucRet ==  RET_RF_ERR_TRANSMIT) || (ucRet == RET_RF_ERR_TIMEOUT))
          {
			    i++;
                if(i > 2)
                {
                       break;
                }

				s_RFDelay10ETUs(10);

                continue;
          }

          if(ucRet == RET_RF_OK)
          {
			  if (RF_WorkInfo.ulBytesReceived != 1 || RF_WorkInfo.aucBuffer[0] != 0xC2) 
			  {
				  ucRet = RET_RF_ERR_PROTOCOL;
			  }
                break;
          }
      }

      
      if(ucRet != RET_RF_OK)
      {
          s_RFResetCarrier();
      }

      RF_WorkInfo.ucMifActivateFlag = 0;
      RF_WorkInfo.ucHalt = 1;

	  return  ucRet;
}



uchar  s_RF_ucMifRBlock(uint* puiRecLen)
{
       uchar ucResult = RET_RF_OK;       
	   uchar ucReSendNo = 0x00;
	   
	   
       for(ucReSendNo = 0; ucReSendNo < 2; ucReSendNo++)
       {
             
             RF_WorkInfo.aucBuffer[0] = RF_WorkInfo.ucCurPCB | 0xB2;
             ucResult = s_RF_ucProcess(1, puiRecLen);
             if((ucResult != RET_RF_ERR_TIMEOUT) && (ucResult != RET_RF_ERR_TRANSMIT))
             {              
                    
                    break;
             }
       }
       return ucResult;      
}


uchar s_RF_ucMifSBlock( uint* puiRecLen)
{
    uchar ucTempData = 0;
    uchar ucRet = RET_RF_OK;	

	if ((RF_WorkInfo.aucBuffer[0] & 0x30) != 0x30)
	{
		
		return RET_RF_ERR_PROTOCOL;
	}
	if ((RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08) 
	{
		
		return RET_RF_ERR_PROTOCOL;
	}
	else
	{ 
		ucTempData = RF_WorkInfo.aucBuffer[1];
		if((ucTempData & 0xC0) != 0x00)
		{
			
			return RET_RF_ERR_PROTOCOL;
		}
		
		RF_WorkInfo.ucWTX = ucTempData & 0x3F;
		if(RF_WorkInfo.ucWTX == 0x00)
		{
			return RET_RF_ERR_PROTOCOL;
		}
		if(RF_WorkInfo.ucWTX  > 59)
		{
			RF_WorkInfo.ucWTX = 59;
		}
     
		RF_WorkInfo.ulFWTTemp = RF_WorkInfo.ucWTX * (RF_WorkInfo.ulFWT+1);		
	}

	RF_WorkInfo.aucBuffer[0] = 0xf2;  
	RF_WorkInfo.aucBuffer[1] = ucTempData  & 0x3F;        

    s_RFSetTimer(RF_WorkInfo.ulFWTTemp+1); 

    if(RF_WorkInfo.ucCurType == RF_TYPEA)
    {
	     ucTempData = 0x0F;
	     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 
	}
	else
	{
	     ucTempData = 0x2C;
	     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 
	}
	   
	RF_WorkInfo.ulSendBytes = 2;     
        
	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN- strlen(CPDU_CMD)))
	{
		strcpy(gl_aucRecData+gl_usRecLen, CPDU_CMD);
		gl_usRecLen += strlen(CPDU_CMD);
	}
#endif
	
	ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);

	if(ucRet == RET_RF_ERR_TIMEOUT)
	{
		ucRet = RET_RF_ERR_TIMEOUT;
	}
	else if(ucRet != RET_RF_OK)
	{	
		ucRet = RET_RF_ERR_TRANSMIT;
	}
	else
	{
			*puiRecLen = RF_WorkInfo.ulBytesReceived;
			if(RF_WorkInfo.ulBytesReceived == 0)
			{
				ucRet = RET_RF_ERR_TRANSMIT;
			}			
	}

	if (ucRet != RET_RF_ERR_TIMEOUT) 
     		s_RFDelay10ETUs(6) ;
    return( ucRet); 
}

#ifdef USE_NEW_PROTOCOL
uchar s_RF_ucExchange(uint uiSendLen, uchar* pucInData, uint* puiRecLen, uchar* pucOutData)
{
	
	uchar r_link_flag,s_link_flag;
	ushort i_resend_count,r_resend_count,s_resend_count;

	uchar temp_wtxm=0;
	ulong temp_fwt=0;

	volatile uchar event_now;

	uint  uiSendLeftLen = 0, uiTempLen = 0, uiSendCurrPos = 0;
	uint uiCurrRecvLen = 0;

	uchar reci_link_flag=0;
	
	uchar ucRet=0;

	#define send_i 			   0x01
	#define send_r_block	   0x02
	#define send_s_block 	   0x04
	#define recieve_i 		   0x06
	#define recieve_r 		   0x07

	#define Link_flag          0x10
	#define unLink_flag        (~Link_flag)
	#define BLOCK_NO			0x01

	
	i_resend_count=0;
	r_resend_count=0;
	s_resend_count=0;

	*puiRecLen = 0; 
	
	uiSendLeftLen=uiSendLen;
	
	event_now=send_i;
	
	//RF_WorkInfo.ucCurPCB=0;//SET CURRENT PCD IS 0，BUT CAN NOT SET HERE,SET IN DETECT FUN
	while(1)
		{
			switch(event_now)
				{
					case send_i:
						r_resend_count=0;
						s_resend_count=0;
						
						if(i_resend_count>=3) return RET_RF_ERR_PROTOCOL;
						i_resend_count++;

						if( uiSendLeftLen>(RF_WorkInfo.FSC - 3) )
						{	
							RF_WorkInfo.ucCurPCB &= BLOCK_NO;
							RF_WorkInfo.ucCurPCB |=(0x02|Link_flag);
							uiTempLen=RF_WorkInfo.FSC - 3;
							
							RF_WorkInfo.aucBuffer[0] = RF_WorkInfo.ucCurPCB;		   
							memcpy((uchar*)&RF_WorkInfo.aucBuffer[1], pucInData + uiSendCurrPos,uiTempLen);
							ucRet = s_RF_ucProcess(uiTempLen+1, &uiCurrRecvLen);
							event_now=recieve_r;
						}
						else
						{
							RF_WorkInfo.ucCurPCB &= BLOCK_NO;
							RF_WorkInfo.ucCurPCB |=(0x02);

							uiTempLen=uiSendLeftLen;
							
							RF_WorkInfo.aucBuffer[0] = RF_WorkInfo.ucCurPCB;		   
							memcpy((uchar*)&RF_WorkInfo.aucBuffer[1], pucInData + uiSendCurrPos,uiTempLen);
							ucRet = s_RF_ucProcess(uiTempLen+1, &uiCurrRecvLen);
							event_now=recieve_i;
						}
					break;
					case send_r_block:
						
						
						if(r_resend_count>=3)
						{
							return RET_RF_ERR_PROTOCOL;
						}
						r_resend_count++;

						if(reci_link_flag)
						{
							RF_WorkInfo.ucCurPCB&=BLOCK_NO;
							RF_WorkInfo.aucBuffer[0] = RF_WorkInfo.ucCurPCB | 0xA2;
								
						}
						else
						{
							RF_WorkInfo.ucCurPCB&=BLOCK_NO;
							RF_WorkInfo.aucBuffer[0] = RF_WorkInfo.ucCurPCB | 0xB2;
						}
						ucRet = s_RF_ucProcess(1,&uiCurrRecvLen);

						if(r_link_flag)
							event_now=recieve_r;

						else
							event_now=recieve_i;
						
					break;

					case send_s_block:
						
						i_resend_count=0;
						
						r_resend_count=0;
						
						if(s_resend_count>=3) return RET_RF_ERR_PROTOCOL;
						s_resend_count++;
						
						temp_fwt=RF_WorkInfo.ulFWT;
						RF_WorkInfo.ulFWT=RF_WorkInfo.ucWTX * RF_WorkInfo.ulFWT;

						
						RF_WorkInfo.aucBuffer[0] = 0xf2;
						RF_WorkInfo.aucBuffer[1] = temp_wtxm;
						ucRet = s_RF_ucProcess(2, &uiCurrRecvLen);
						RF_WorkInfo.ulFWT=temp_fwt;

						if(s_link_flag)
							event_now=recieve_r;

						else
							event_now=recieve_i;
					break;
		
					case recieve_i:

						r_link_flag=0;
						s_link_flag=0;

						
						if( ucRet )
						{
							
							event_now=send_r_block;
						}
						else if(uiCurrRecvLen >	(RF_WorkInfo.FSD - 2))
						{
							
							return RET_RF_ERR_PROTOCOL;
						}
						else if( RF_WorkInfo.aucBuffer[0] == (0x12|(RF_WorkInfo.ucCurPCB & BLOCK_NO)) )
						{
							
							memcpy(&pucOutData[*puiRecLen], (uchar*)&RF_WorkInfo.aucBuffer[1], uiCurrRecvLen-1);
							*puiRecLen += uiCurrRecvLen - 1;
							RF_WorkInfo.ucCurPCB=(RF_WorkInfo.ucCurPCB+1) & BLOCK_NO;

							r_resend_count=0;
							s_resend_count=0;
							reci_link_flag=1;
							event_now=send_r_block;
						}
						else if( RF_WorkInfo.aucBuffer[0] == (0x02|(RF_WorkInfo.ucCurPCB & BLOCK_NO)) )
						{
							
							memcpy(&pucOutData[*puiRecLen], (uchar*)&RF_WorkInfo.aucBuffer[1], uiCurrRecvLen-1);
							*puiRecLen += uiCurrRecvLen - 1;
							RF_WorkInfo.ucCurPCB=(RF_WorkInfo.ucCurPCB+1) & BLOCK_NO;
							return RET_RF_OK;
						}
						else if(RF_WorkInfo.aucBuffer[0] == (0xa2|( (RF_WorkInfo.ucCurPCB+1) & BLOCK_NO)))
						{
							event_now=send_i;
						}
						else if(RF_WorkInfo.aucBuffer[0] == 0xf2)
						{
							
								temp_wtxm = RF_WorkInfo.aucBuffer[1];
								if((temp_wtxm & 0xC0) != 0x00)
								{
									
									return RET_RF_ERR_PROTOCOL;
								}
								
								RF_WorkInfo.ucWTX = temp_wtxm & 0x3F;
								if(RF_WorkInfo.ucWTX == 0x00)
								{
									return RET_RF_ERR_PROTOCOL;
								}
								if(RF_WorkInfo.ucWTX  > 59)
								{
									RF_WorkInfo.ucWTX = 59;
								}
								
								
								event_now=send_s_block;
						}
						else
						{
							return RET_RF_ERR_PROTOCOL;
						}	
					break;
					case recieve_r:
			
						r_link_flag=1;
						s_link_flag=1;
						
						reci_link_flag=0;
						
						
						if( ucRet )
						{
							
							event_now=send_r_block;
						}
						else if(uiCurrRecvLen >	(RF_WorkInfo.FSD - 2))
						{
							return RET_RF_ERR_PROTOCOL;
						}

						
						else if(RF_WorkInfo.aucBuffer[0] == (0xa2|( RF_WorkInfo.ucCurPCB & BLOCK_NO)))
						{	
							i_resend_count=0;

							RF_WorkInfo.ucCurPCB=(RF_WorkInfo.ucCurPCB+1) & BLOCK_NO;

							
							uiSendCurrPos += uiTempLen;
							uiSendLeftLen -= uiTempLen;
							event_now=send_i;
						}
						else if( RF_WorkInfo.aucBuffer[0] == (0x02|( (RF_WorkInfo.ucCurPCB+1) & BLOCK_NO )) )
						{
							event_now=send_i;
						}
						
						else if(RF_WorkInfo.aucBuffer[0] == 0xf2)
						{
								temp_wtxm = RF_WorkInfo.aucBuffer[1];
								if((temp_wtxm & 0xC0) != 0x00)
								{
									
									return RET_RF_ERR_PROTOCOL;
								}
								
								RF_WorkInfo.ucWTX = temp_wtxm & 0x3F;
								if(RF_WorkInfo.ucWTX == 0x00)
								{
									return RET_RF_ERR_PROTOCOL;
								}
								if(RF_WorkInfo.ucWTX  > 59)
								{
									RF_WorkInfo.ucWTX = 59;
								}
								
								event_now=send_s_block;
						}
						else
						{
							return RET_RF_ERR_PROTOCOL;
						}	
					break;
					
					default:
					break;
				}
		}
}
#else

uchar s_RF_ucExchange(uint uiSendLen, uchar* pucInData, uint* puiRecLen, uchar* pucOutData)
{	  
    uint  uiSendLeftLen = 0, uiTempLen = 0, uiSendCurrPos = 0,uiCurrRecvLen = 0;
	uchar ucRet = RET_RF_OK;
	uchar tempResult = 0x00;
	uchar i = 0;
    uchar ucResendNo = 0x00;   
    uchar ucSBlk = 0; 
    uchar ucRFlg = 0;
    uchar ucRNckFlg = 0;
    uchar ucIBlkFlg = 0; 
	
	*puiRecLen = 0; 

	uiSendLeftLen =	uiSendLen;	  
	
	ucSBlk = 0; // 20081019 for record s(wtx) counts
	
	while(uiSendLeftLen	> (RF_WorkInfo.FSC - 3))
	{		   
		 uiTempLen = RF_WorkInfo.FSC - 3;
		 RF_WorkInfo.ucCurPCB = (RF_WorkInfo.ucCurPCB + 1) & 0x01;
		 RF_WorkInfo.aucBuffer[0] = RF_WorkInfo.ucCurPCB | 0x12;		   
		 memcpy((uchar*)&RF_WorkInfo.aucBuffer[1], pucInData + uiSendCurrPos,	uiTempLen);
		 ucRet = s_RF_ucProcess(uiTempLen+1, &uiCurrRecvLen);
		 
		 if((ucRet	== RET_RF_ERR_TIMEOUT) ||	(ucRet	== RET_RF_ERR_TRANSMIT))
		 {
			    tempResult = ucRet;
			    ucRet = s_RF_ucMifRBlock(&uiCurrRecvLen);
				if(ucRet != RET_RF_OK)
				{			
					 return tempResult;
				}
		 }		 
		 
		 if(ucRet != RET_RF_OK)
		 {
			 return ucRet;
		 }

		 if(uiCurrRecvLen >	(RF_WorkInfo.FSD - 2))
		 {
				return RET_RF_ERR_PROTOCOL;
		 }
		 
		 if (uiCurrRecvLen == 0)
		 { 
			    
				return RET_RF_ERR_TRANSMIT;
		 }

 		// 20081017 NO NAD
		if (RF_WorkInfo.aucBuffer[0] & 0x04)
		{
			return RET_RF_ERR_PROTOCOL;
		}
		
		 if((RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
		 {
				
				return RET_RF_ERR_PROTOCOL;
		 }
		 else if((RF_WorkInfo.aucBuffer[0] & 0xE2) == 0xA2)
		 {
				// R_Block
				if((RF_WorkInfo.aucBuffer[0] & 0x10) == 0x10)
				{
					 
					 return RET_RF_ERR_PROTOCOL;
				}
				
				if((RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
				{
					 
					 return	RET_RF_ERR_PROTOCOL;
				}
				
				if((RF_WorkInfo.aucBuffer[0] & 0x01) == RF_WorkInfo.ucCurPCB)
				{
					 
				     uiSendCurrPos += uiTempLen;
				     uiSendLeftLen -= uiTempLen;
				 
				     ucResendNo = 0;
				}
				else
			    {   
				     if(ucResendNo++ > 3)
				     {
						 
						   return RET_RF_ERR_PROTOCOL;
				     }
				     
				     RF_WorkInfo.ucCurPCB	= (RF_WorkInfo.ucCurPCB + 1) & 0x01;				  
			   }
		 }
		 else if((RF_WorkInfo.aucBuffer[0] & 0xC7) == 0xC2)
		 {				
				
				while(1)
				{
						ucRet = s_RF_ucMifSBlock(&uiCurrRecvLen);
						ucSBlk++; 
						if((ucRet == RET_RF_ERR_TIMEOUT)	|| (ucRet == RET_RF_ERR_TRANSMIT))
						{
	   					    if(ucRet == RET_RF_ERR_TIMEOUT)
							{
								 			  
								 if(ucSBlk >= 3)
								 {
									   
									   return RET_RF_ERR_PROTOCOL;
								 }
							}
							tempResult = ucRet;
							
							ucRet = s_RF_ucMifRBlock(&uiCurrRecvLen);
							if(ucRet != RET_RF_OK)
							{
								   
								   return tempResult;
							}
						}						
						
						if(ucRet != RET_RF_OK)
						{
							 return ucRet;
						}

						if(uiCurrRecvLen > (RF_WorkInfo.FSD - 2))
						{
								
								return RET_RF_ERR_PROTOCOL;
						}

						if (uiCurrRecvLen == 0)
						{  
							    
								return RET_RF_ERR_TRANSMIT;
						}
										
						if((RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
						{
								
								return RET_RF_ERR_PROTOCOL;
						}
						else if((RF_WorkInfo.aucBuffer[0]	& 0xE2)	== 0xA2)
						{						
								
								if((RF_WorkInfo.aucBuffer[0] & 0x10) == 0x10)
								{
									   
									   return RET_RF_ERR_PROTOCOL;
								}

								if((RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
								{
									   
									   return	RET_RF_ERR_PROTOCOL;
								}
								
								if((RF_WorkInfo.aucBuffer[0] & 0x01) == RF_WorkInfo.ucCurPCB)
								{
								   
								   uiSendCurrPos +=	uiTempLen;
								   uiSendLeftLen -=	uiTempLen;
								   
								   
								   ucResendNo =	0;
								   ucSBlk =	0;
								}
								else
								{	 
									 		  
									 if(ucResendNo++ > 3)
									 {
										   
										   return RET_RF_ERR_PROTOCOL;
									 }
									 
									 RF_WorkInfo.ucCurPCB	= (RF_WorkInfo.ucCurPCB + 1) &	0x01;						  
								}
								break;
						 }
						 else if((RF_WorkInfo.aucBuffer[0] & 0xC7) != 0xC2)
						 {
                               
							   return	RET_RF_ERR_PROTOCOL;
						 }
				}                
		 }
		 else
		 {
								  
				return RET_RF_ERR_PROTOCOL;
		 }
	}
	
	ucResendNo = 0; 
	ucSBlk =	0;
	
	ucIBlkFlg = 0;
	while(1)
	{		   
		 RF_WorkInfo.ucCurPCB = (RF_WorkInfo.ucCurPCB + 1) & 0x01;
		   
		 memcpy((uchar*)&RF_WorkInfo.aucBuffer[1], pucInData + uiSendCurrPos, uiSendLeftLen);
		 RF_WorkInfo.aucBuffer[0] = RF_WorkInfo.ucCurPCB | 0x02;
		 ucRet	= s_RF_ucProcess(uiSendLeftLen+1, &uiCurrRecvLen); 		
		 ucIBlkFlg++;

		 if((ucRet	== RET_RF_ERR_TIMEOUT) || (ucRet	== RET_RF_ERR_TRANSMIT))
		 {	   
			    tempResult = ucRet;				
			    ucRet = s_RF_ucMifRBlock(&uiCurrRecvLen);
				if(ucRet != RET_RF_OK)
				{ 
					   return tempResult;
				}
		 }
		 
		 if(ucRet != RET_RF_OK)
		 {	 
			 return ucRet;
		 }

		 if(uiCurrRecvLen >	(RF_WorkInfo.FSD - 2))
		 {
				
				return RET_RF_ERR_PROTOCOL;
		 }

		 if (uiCurrRecvLen == 0)
		 { 	  
			    
			 return RET_RF_ERR_TRANSMIT;
		 }
		
		if (RF_WorkInfo.aucBuffer[0] & 0x04)
		{
			return RET_RF_ERR_PROTOCOL;
		}
			
		 if((RF_WorkInfo.aucBuffer[0] & 0xE2) == 0xA2)
		 {	
				
				if((RF_WorkInfo.aucBuffer[0] & 0x10) == 0x10)
				{
					   
					   return	RET_RF_ERR_PROTOCOL;
				}

				if((RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
				{
					   
					   return	RET_RF_ERR_PROTOCOL;
				}
				
				if((RF_WorkInfo.aucBuffer[0] & 0x01) == RF_WorkInfo.ucCurPCB)
				{
					   
					   return	RET_RF_ERR_PROTOCOL;
				}
				else
				{
					   if(ucResendNo++ > 3
					   ||ucIBlkFlg > 2)
				       {
						   
						   return	RET_RF_ERR_PROTOCOL;
				       }
					   
					   RF_WorkInfo.ucCurPCB = (RF_WorkInfo.ucCurPCB + 1) & 0x01; 
				}
		 }
		 else if((RF_WorkInfo.aucBuffer[0] & 0xC7) == 0xC2)
		 {				

				while(1)
				{	
					ucRet = s_RF_ucMifSBlock(&uiCurrRecvLen);
					ucSBlk++;
					if((ucRet == RET_RF_ERR_TIMEOUT)	|| (ucRet == RET_RF_ERR_TRANSMIT))
					{
						 if(ucRet == RET_RF_ERR_TIMEOUT)
						{
							 			  
							 if(ucSBlk >= 3)
							 {
								   
								   return RET_RF_ERR_PROTOCOL;
							 }
						}
						tempResult = ucRet;
						
						ucRet = s_RF_ucMifRBlock(&uiCurrRecvLen);	
						if(ucRet != RET_RF_OK)
						{
							   
							   return tempResult;
						}
					}
					
					if(ucRet != RET_RF_OK)
					{
						 return ucRet;
					}

					if (uiCurrRecvLen == 0)
					{ 
						    
							return RET_RF_ERR_TRANSMIT;
					}

					if(uiCurrRecvLen > (RF_WorkInfo.FSD - 2))
					{
						  
						  return RET_RF_ERR_PROTOCOL;
					}
					  
					if((RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
					{	
						
						break;
					}
					else if((RF_WorkInfo.aucBuffer[0]	& 0xE2)	== 0xA2)
					{
						
						if((RF_WorkInfo.aucBuffer[0] & 0x10) == 0x10)
						{
							   
							   return	RET_RF_ERR_PROTOCOL;
						}

						if((RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
						{
							   
							   return	RET_RF_ERR_PROTOCOL;
						}
						
						if((RF_WorkInfo.aucBuffer[0] & 0x01) == RF_WorkInfo.ucCurPCB)
						{
							  
							   return	RET_RF_ERR_PROTOCOL;
						}
						else
						{
							   if(ucResendNo > 3)
							   {
								   
								   return	RET_RF_ERR_PROTOCOL;
							   }
							   
							   RF_WorkInfo.ucCurPCB = (RF_WorkInfo.ucCurPCB +	1) & 0x01; 
						}
						break;
					}
					else if((RF_WorkInfo.aucBuffer[0] & 0xC7) != 0xC2)
					{
                        
						return RET_RF_ERR_PROTOCOL;
					}	
					

				}

				if((RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
				{	
					
					break;
				}
		 }
		 else if((RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
		 {			
				
				break;
		 }
		 else
		 {
				
				return RET_RF_ERR_PROTOCOL;
		 }		 
	}
	
	ucSBlk =	0;
	
	while(1)
	{
		   if((RF_WorkInfo.aucBuffer[0] & 0x01) != RF_WorkInfo.ucCurPCB)
		   {
				 
				 return	RET_RF_ERR_PROTOCOL;
		   }

		   if((RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
		   {
				  
				  return RET_RF_ERR_PROTOCOL;
		   }
		   
		   
		   if((RF_WorkInfo.aucBuffer[0] & 0x04) == 0x04)
		   {
				  
				  return RET_RF_ERR_PROTOCOL;
		   }

		   if(uiCurrRecvLen < 1)
		   {
			   return RET_RF_ERR_TRANSMIT;
		   }
		   
		   memcpy(&pucOutData[*puiRecLen], (uchar*)&RF_WorkInfo.aucBuffer[1], uiCurrRecvLen-1);
		   *puiRecLen += uiCurrRecvLen - 1;			 
		   			
		   if((RF_WorkInfo.aucBuffer[0] & 0x10) == 0x00)
		   {
				 
				 break;
		   }
		   
		   RF_WorkInfo.ucCurPCB = (RF_WorkInfo.ucCurPCB + 1) & 0x01;
		   
		   i = 0;
		   while(1)
		   {
				
				RF_WorkInfo.aucBuffer[0] = RF_WorkInfo.ucCurPCB | 0xA2;

				ucRFlg = 0;
			    ucRet = s_RF_ucProcess(1,&uiCurrRecvLen);
				
				if((ucRet == RET_RF_ERR_TIMEOUT) || (ucRet == RET_RF_ERR_TRANSMIT))
				{
					   i++;
					   if(i	== 3)
					   {
						    
							return ucRet;
					   }
					   
					   continue;
				}	
				else
				{
					ucRFlg = 1;
				}			
				
				if(ucRet != RET_RF_OK)
				{
					 return ucRet;
				}

				if (uiCurrRecvLen == 0)
				{  
					    
						return RET_RF_ERR_TRANSMIT;
				}

				if(uiCurrRecvLen > (RF_WorkInfo.FSD - 2))
				{
					  
					  return RET_RF_ERR_PROTOCOL;
				}
				  
				if((RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
				{
					
					break;  
				}
				else if((RF_WorkInfo.aucBuffer[0] & 0xE2) == 0xA2)
				{	   
					   
					   if((RF_WorkInfo.aucBuffer[0] & 0x10) == 0x10)
					   {
							
							return RET_RF_ERR_PROTOCOL;
					   }
				
					   if((RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
					   {
							
							return RET_RF_ERR_PROTOCOL;
					   }
					   			
					 if(ucRFlg == 1 && (RF_WorkInfo.aucBuffer[0] & 0x01) == RF_WorkInfo.ucCurPCB)
					 {
						
						return RET_RF_ERR_PROTOCOL;
					 }
					   if((RF_WorkInfo.aucBuffer[0] & 0x01) != RF_WorkInfo.ucCurPCB)
					   {
							
							return RET_RF_ERR_PROTOCOL;
					   }

					   i++;
					   if(i	== 3)
					   {
						   
						   return RET_RF_ERR_PROTOCOL;
					   }					   
					   continue;
				}
				else if((RF_WorkInfo.aucBuffer[0] & 0xC7) == 0xC2)
				{
				    
					while(1)
					{
						ucRet = s_RF_ucMifSBlock(&uiCurrRecvLen);
						ucSBlk++;
						if((ucRet == RET_RF_ERR_TIMEOUT)	|| (ucRet == RET_RF_ERR_TRANSMIT))
						{
							 if(ucRet == RET_RF_ERR_TIMEOUT)
							{
								 					  
								 if(ucSBlk >= 3)
								 {
									   
									   return RET_RF_ERR_PROTOCOL;
								 }
							}
							tempResult = ucRet;
							
							ucRet = s_RF_ucMifRBlock(&uiCurrRecvLen);
							if(ucRet != RET_RF_OK)
							{
								   
								   return tempResult;
							}
						}
						
						if(ucRet != RET_RF_OK)
						{
							 return ucRet;
						}

						if (uiCurrRecvLen == 0)
						{ 
							    
								return RET_RF_ERR_TRANSMIT;
						}
						
						if(uiCurrRecvLen > (RF_WorkInfo.FSD - 2))
						{
							  
							  return RET_RF_ERR_PROTOCOL;
						}
						  
						if((RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
						{	
							
							break;
						}
						else if((RF_WorkInfo.aucBuffer[0]	& 0xE2)	== 0xA2)
						{
							
							if((RF_WorkInfo.aucBuffer[0] & 0x10) == 0x10)
							{
								   
								   return	RET_RF_ERR_PROTOCOL;
							}

							if((RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
							{
								   
								   return	RET_RF_ERR_PROTOCOL;
							}
							
							if((RF_WorkInfo.aucBuffer[0] & 0x01) != RF_WorkInfo.ucCurPCB)
							{
								   
								   return	RET_RF_ERR_PROTOCOL;
							}
							else
							{
								   if(i++ > 3)
								   {
									   
									   return	RET_RF_ERR_PROTOCOL;
								   }
							}
							break;
						}
						else if((RF_WorkInfo.aucBuffer[0] & 0xC7) != 0xC2)
						{
							
							return RET_RF_ERR_PROTOCOL;
						}
					}

					if((RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
					{	
						
						break;
					}
				}
				else
				{
                     
					 return RET_RF_ERR_PROTOCOL;
				}
		   }
	       
		   i = 0; 
		   ucSBlk= 0;
	}
	
	
	if ((*puiRecLen > 0) && (ucRet == RET_RF_OK))
	{
	  ucRet = RET_RF_OK;
	}
	else
	{
         *puiRecLen = 0;
		 
		 if (ucRet == RET_RF_OK) 
		 {
			 ucRet = RET_RF_ERR_TRANSMIT;
		 }
	}
	
	return ucRet;
}

#endif




void s_RF_vGetDetectAInfo(uchar *SerialInfo, uchar *Other)
{
	switch(RF_WorkInfo.ucUIDLen)
	{
	    case 4:
			memcpy(SerialInfo+1, (uchar*)RF_WorkInfo.ucUIDCL1, 4);
			break;
		case 7:
			
			memcpy(SerialInfo+1, (uchar*)RF_WorkInfo.ucUIDCL1+1, 3);
			memcpy(SerialInfo+4, (uchar*)RF_WorkInfo.ucUIDCL2, 4);
			break;
		case 10:
			memcpy(SerialInfo+1, (uchar*)RF_WorkInfo.ucUIDCL1+1, 3);
			memcpy(SerialInfo+4, (uchar*)RF_WorkInfo.ucUIDCL2+1, 3);
			memcpy(SerialInfo+7, (uchar*)RF_WorkInfo.ucUIDCL3, 4);
			break;
	}

	
	memcpy(Other+3, (uchar*)RF_WorkInfo.ucATQA, 2);
	switch(RF_WorkInfo.ucUIDLen)
	{
	   case 4:				   	  
	   	  Other[0] = 5;
		  Other[5] = RF_WorkInfo.ucSAK1;
		  break;
	   case 7:	   	  
	   	  Other[0] = 6;
		  Other[5] = RF_WorkInfo.ucSAK1;
		  Other[6] = RF_WorkInfo.ucSAK2;
		  break;
	   case 10:	   	  
	   	  Other[0] = 7;
		  Other[5] = RF_WorkInfo.ucSAK1;
		  Other[6] = RF_WorkInfo.ucSAK2;
		  Other[7] = RF_WorkInfo.ucSAK3;
		  break;
	}	
	
	return;
}


void s_RFDelay10ETUs(ulong nETU)
{
    ulong  TmpETU = 0;
	uchar ucTemp = 0;
	ushort usTimeValue = 0;

	if(nETU <= 0) 
		return;
	
	TmpETU = nETU*10;

	s_vRFSetBitMask(RF_REG_CONTROL,0x04);
	s_RFSetTimer(TmpETU);
	s_vRFSetBitMask(RF_REG_CONTROL, 0x02);

	ucTemp = 1;
	while (ucTemp)
	{
		s_vRFReadReg(1,RF_REG_TIMERVALUE,&ucTemp);		
	}

}


void s_RFSetTimer(ulong nETU)
{
    uchar ucTempData;   
    
    if(nETU == RF_FWTMIN)
	{
         
		 
         ucTempData = 0x03;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         
		 
         ucTempData = 0xFF;
         s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}
	else if(nETU < 256)
	{
         
         ucTempData = 0x07;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         ucTempData = nETU; 
		 s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}
	else if(nETU < 511)
	{
		 
		 ucTempData = 0x08;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         ucTempData = nETU/2;
         if((nETU % 2) != 0)
		 {
			 ucTempData = ucTempData + 1;
		 }
		 s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}
	else if(nETU < 1021)
	{
		 
         ucTempData = 0x09;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         ucTempData = nETU/4;
         if((nETU % 4) != 0)
		 {
			 ucTempData = ucTempData + 1;
		 }
		 s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}
	else if(nETU < 2041)
	{
         
         ucTempData = 0x0A;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         ucTempData = nETU/8;
         if((nETU % 8) != 0)
		 {
			 ucTempData = ucTempData + 1;
		 }
		 s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}
	else if(nETU < 4081)
	{
         
         ucTempData = 0x0B;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         ucTempData = nETU/16;
         if((nETU % 16) != 0)
		 {
			 ucTempData = ucTempData + 1;
		 }
		 s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}
	else if(nETU < 8161)
	{
         
         ucTempData = 0x0C;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         ucTempData = nETU/32;
         if((nETU % 32) != 0)
		 {
			 ucTempData = ucTempData + 1;
		 }
		 s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}
	else if(nETU < 16321)
	{
         
         ucTempData = 0x0D;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         ucTempData = nETU/64;
         if((nETU % 64) != 0)
		 {
			 ucTempData = ucTempData + 1;
		 }
		 s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}
	else if(nETU < 32641)
	{
         
         ucTempData = 0x0E;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         ucTempData = nETU/128;
         if((nETU % 128) != 0)
		 {
			 ucTempData = ucTempData + 1;
		 }
		 s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}
	else if(nETU < 65281)
	{
         
         ucTempData = 0x0F;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         ucTempData = nETU/256;
         if((nETU % 256) != 0)
		 {
			 ucTempData = ucTempData + 1;
		 }
		 s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}
	else if(nETU < 130561)
	{
         
         ucTempData = 0x10;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         ucTempData = nETU/512;
         if((nETU % 512) != 0)
		 {
			 ucTempData = ucTempData + 1;
		 }
		 s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}
	else if(nETU < 261121)
	{
         
         ucTempData = 0x11;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         ucTempData = nETU/1024; 
         if((nETU % 1024) != 0)
		 {
			 ucTempData = ucTempData + 1;
		 }
		 s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}
	else if(nETU < 522241)
	{
         
         ucTempData = 0x12;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         ucTempData = nETU/2048; 
         if((nETU % 2048) != 0)
		 {
			 ucTempData = ucTempData + 1;
		 }
		 s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}
	else
	{
		 
         ucTempData = 0x13;
         s_vRFWriteReg(1, RF_REG_TIMERCLOCK, &ucTempData); 

         ucTempData = nETU/4096; 
         if((nETU % 4096) != 0)
		 {
			 ucTempData = ucTempData + 1;
		 }
		 s_vRFWriteReg(1, RF_REG_TIMERRELOAD, &ucTempData);
	}

}


void s_RFOpenCarrier(void)
{	     
	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN- strlen(CARRIER_ON)))
	{ 
		strcpy(gl_aucRecData+gl_usRecLen, CARRIER_ON);
		gl_usRecLen += strlen(CARRIER_ON);
	}
#endif
      
      s_vRFSetBitMask(RF_REG_TXCONTROL, 0x03);
}


void s_RFCloseCarrier(void)
{
	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN- strlen(CARRIER_OFF)))
	{ 
		strcpy(gl_aucRecData+gl_usRecLen, CARRIER_OFF);
		gl_usRecLen += strlen(CARRIER_OFF);
	}
#endif
	
	s_vRFClrBitMask(RF_REG_TXCONTROL, 0x03);
	if(RF_WorkInfo.ucPollMode == 0) 
		s_RFDelay10ETUs(100); 
	else 
		s_RFDelay10ETUs(60);

}


void s_RFResetCarrier(void)
{
	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN- strlen(CARRIER_OFF)))
	{ 
		strcpy(gl_aucRecData+gl_usRecLen, CARRIER_OFF);
		gl_usRecLen += strlen(CARRIER_OFF);
	}
#endif
	s_vRFClrBitMask(RF_REG_TXCONTROL, 0x03);

	if(RF_WorkInfo.ucPollMode == 0) 
		s_RFDelay10ETUs(100); 
	else 
		s_RFDelay10ETUs(60); 
	
      
      
      s_vRFSetBitMask(RF_REG_TXCONTROL, 0x03); 
	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN- strlen(CARRIER_ON)))
	{ 
		strcpy(gl_aucRecData+gl_usRecLen, CARRIER_ON);
		gl_usRecLen += strlen(CARRIER_ON);
	}
#endif
}


#ifdef Debug_Isr
uint gl_LoAlert=0,gl_TxIRQ=0,gl_RxIRQ=0,gl_HiAlert_Idle=0,gl_HiAlert=0,gl_Idle=0,gl_timer=0;
uchar gl_err_flg1=0,gl_err_flg2=0,gl_err_flg3=0,gl_err_flg4=0,gl_loalertNum=0;
uchar gl_err_flg5=0,gl_err_flg6=0,gl_err_flg5_1=0,gl_err_flg6_1=0,gl_err_flg5_2=0,gl_err_flg6_2=0,gl_err_flg5_3=0,gl_err_flg6_3=0;
#endif

void s_RFIsr(void)
{
	
    volatile uchar ucTempData;
    volatile uchar ucIrqPending;
    ulong ulTemp;
	ulong i = 0;

	uchar ucTemp = 0;
	uchar ucOver = 2;
	volatile uchar j = 0;
	
	/*
	if( (EIF & (RFINT))!=RFINT )//CHECK WHETHER IS RF INT HAPPEN,IF NOT RETURN
	{
		printf("*****not rf int err******\n");
		//return;	
	}
	*/
	
	sHal_MaskCpuInt();
	s_vRFSwitchNCS(1);

	
    s_vRFReadReg(1,RF_REG_ERRORFLAG,(uchar*)&RF_WorkInfo.ucErrFlags);

	#ifdef Debug_Isr
	 		if(gl_loalertNum==0)
	 				gl_err_flg1=RF_WorkInfo.ucErrFlags;
	 		if(gl_loalertNum==1)
	 				gl_err_flg2=RF_WorkInfo.ucErrFlags;
	 		if(gl_loalertNum==2)
	 				gl_err_flg3=RF_WorkInfo.ucErrFlags;
	 		if(gl_loalertNum==3)
	 				gl_err_flg4=RF_WorkInfo.ucErrFlags;

			
			s_vRFReadReg(1,RF_REG_INTERRUPTEN,&ucTempData);
			s_vRFReadReg(1,RF_REG_INTERRUPTRQ,&ucIrqPending);

	 		if(gl_loalertNum==0)
			{
				gl_err_flg5=ucTempData;
				gl_err_flg6=ucIrqPending;
			}
	 				
	 		if(gl_loalertNum==1)
			{
				gl_err_flg5_1=ucTempData;
				gl_err_flg6_1=ucIrqPending;
			}
			if(gl_loalertNum==2)
			{
				gl_err_flg5_2=ucTempData;
				gl_err_flg6_2=ucIrqPending;
			}
			if(gl_loalertNum==3)
			{
				gl_err_flg5_3=ucTempData;
				gl_err_flg6_3=ucIrqPending;
			}
	 		gl_loalertNum++;
	#endif
	
    RF_WorkInfo.ucErrFlags &= 0x1f;

    while (1)
    {
        s_vRFReadReg(1,RF_REG_PRIMARYSTATUS,&ucTempData);
		
        if ((ucTempData & 0x08) == 0)
        {
        	#ifdef Debug_Isr
			debug_printf(0,0,0,"isr in but not isr happen !,stat:%02x",ucTempData);
			#endif
			break;
        }

		
        s_vRFReadReg(1,RF_REG_INTERRUPTEN,&ucTempData);
        s_vRFReadReg(1,RF_REG_INTERRUPTRQ,&ucIrqPending);
	
        ucIrqPending &= ucTempData;
        RF_WorkInfo.ucINTSource |= ucIrqPending;

        
        if((ucIrqPending & 0x01) != 0)
        {
			#ifdef Debug_Isr
			gl_LoAlert++;
			#endif
            s_vRFReadReg(1, RF_REG_FIFOLENGTH, &ucTempData);
            ulTemp = 64 - ucTempData;
			

           
            if( (RF_WorkInfo.ulSendBytes - RF_WorkInfo.ulBytesSent) <= ulTemp)
            {
                
                ulTemp = RF_WorkInfo.ulSendBytes - RF_WorkInfo.ulBytesSent;
                ucTempData = 0x01;
			
                
                s_vRFWriteReg(1, RF_REG_INTERRUPTEN, &ucTempData); 				
            }
		

			for(i = 0; i<ulTemp; i++) 
			{
				s_vRFWriteReg(1,RF_REG_FIFODATA,(uchar*)RF_WorkInfo.aucBuffer+RF_WorkInfo.ulBytesSent);
				RF_WorkInfo.ulBytesSent++;
			}
			
            
            ucTempData = 0x01;
            s_vRFWriteReg(1, RF_REG_INTERRUPTRQ, &ucTempData);		
        }

        if((ucIrqPending & 0x10 ) != 0)
        {
			#ifdef Debug_Isr
			gl_TxIRQ++;
			#endif
            
            ucTempData = 0x10;
            s_vRFWriteReg(1,RF_REG_INTERRUPTRQ,&ucTempData);

            
            ucTempData = 0x82;
            s_vRFWriteReg(1, RF_REG_INTERRUPTEN,&ucTempData);
			
            if (((RF_WorkInfo.ucAnticol == PICC_ANTICOLL1) && (RF_WorkInfo.lBitsReceived == 7))
                || ((RF_WorkInfo.ucAnticol == PICC_ANTICOLL2) && (RF_WorkInfo.lBitsReceived == 7))
				|| ((RF_WorkInfo.ucAnticol == PICC_ANTICOLL3) && (RF_WorkInfo.lBitsReceived == 7)))
			{
                
                ucTempData = 0x02;                          
                s_vRFWriteReg(1,RF_REG_CHANNELREDUNDANCY,&ucTempData); 
            }
        }

        if((ucIrqPending & 0x08) != 0)
        {
			#ifdef Debug_Isr
			gl_RxIRQ++;
			#endif
            
           if (RF_WorkInfo.ucErrFlags == 0 || RF_WorkInfo.ucM1Stage == 1)
            {
                ucTempData = 0;
                s_vRFWriteReg(1,RF_REG_COMMAND,&ucTempData);
                ucIrqPending |= 0x04;
            }
           else
            {
            	
                RF_WorkInfo.ucSaveErrState = RF_WorkInfo.ucErrFlags;
                
                s_vRFFlushFIFO();

                
                RF_WorkInfo.ulBytesReceived = 0x00;
                
                ucIrqPending &= ~0x08;
                
                ucTempData = 0x08;
                s_vRFWriteReg(1,RF_REG_INTERRUPTRQ,&ucTempData);
        
            }
        }

        if((ucIrqPending & 0x0E) != 0)
        {
			#ifdef Debug_Isr
			gl_HiAlert_Idle++;
			#endif
            
			{		 
	            s_vRFReadReg(1,RF_REG_FIFOLENGTH,&ucTempData);
	  
				if(ucTempData > 0 && RF_WorkInfo.ucCurResult == RET_RF_OK)
				{
	                if ((RF_BUFFER_LEN - RF_WorkInfo.ulBytesReceived) < (ulong) ucTempData)
	                {
	                    ucTempData = (uchar)( RF_BUFFER_LEN - RF_WorkInfo.ulBytesReceived);                             
	                }
					for(i = 0; i<ucTempData; i++)
					{				
						s_vRFReadReg(1,RF_REG_FIFODATA,
						         (uchar*)RF_WorkInfo.aucBuffer+RF_WorkInfo.ulBytesReceived);
	            	    RF_WorkInfo.ulBytesReceived++;				
					}
                                  
	                s_vRFReadReg(1,RF_REG_FIFOLENGTH,&ucTempData);
					 
	                if ((RF_WorkInfo.ulBytesReceived == RF_BUFFER_LEN) && (ucTempData!=0))
	                {
	                	
	                    RF_WorkInfo.ucCurResult = RET_RF_ERR_OVERFLOW; 
						break;
					}  
				}
			
	            ucTempData = 0x0A & ucIrqPending;
	            s_vRFWriteReg(1,RF_REG_INTERRUPTRQ,&ucTempData);
			}
        }

        if((ucIrqPending & 0x02) != 0)
        {
			#ifdef Debug_Isr
			gl_HiAlert++;
			#endif
            
            s_vRFReadReg(1, RF_REG_PRIMARYSTATUS,&ucTempData);
            if((ucTempData & 0x70) == 0x70) 
            {
                s_vRFSetBitMask(RF_REG_CONTROL, 0x04);
            }
        }
      
		if(RF_WorkInfo.ucSaveErrState != 0 && RF_WorkInfo.ucM1Stage == 0)
		{
			ucTempData = 0;
            s_vRFWriteReg(1,RF_REG_COMMAND,&ucTempData);
			ucIrqPending |= 0x04;
		}

		if((ucIrqPending & 0x04) != 0)
        {
			#ifdef Debug_Isr
			gl_Idle++;
			#endif
            
            ucTempData = 0x20;
            
            s_vRFClrBitMask(RF_REG_INTERRUPTEN,ucTempData);

            ucTempData = 0x24;
            
            s_vRFWriteReg(1,RF_REG_INTERRUPTRQ,&ucTempData);

            ucIrqPending &= ~0x20;
            RF_WorkInfo.ucINTSource = (RF_WorkInfo.ucINTSource & ~0x20)|0x04;			    
        }

        if((ucIrqPending & 0x20) != 0)
        {
			#ifdef Debug_Isr
			gl_timer++;
			#endif
            
			ucTempData = 0x20;
            s_vRFWriteReg(1,RF_REG_INTERRUPTRQ,&ucTempData);

            if(RF_WorkInfo.ucCurResult == RET_RF_OK)
            {
                #ifdef Debug_Isr
                s_printf("TimerOut!\n");
                #endif
                RF_WorkInfo.ucCurResult = RET_RF_ERR_TIMEOUT;
            }
            
        }
			
    }
ISR_END:
sHal_EnCpuInt();
}

uchar s_RFExchangeCmd(uchar ucCmd)
{
	uchar ucTempRegVal; 
	uchar ucTempData;		
	uchar ucWaterLevelBak;	 
	uchar ucTimerCtl;		 
	uchar ucIntWait;		
	uchar ucIntEnable;		 
	uchar ucrxMultiple;
	uchar ucFlg = 0; 
	uint uiTemp = 0;
	uint uiBeginTime = 0; 
	uchar uc1, uc2;

	RF_WorkInfo.ucCurResult		=	RET_RF_OK;
	RF_WorkInfo.ucINTSource		=	0;
	RF_WorkInfo.ulBytesSent		=	0;		
	RF_WorkInfo.ulBytesReceived	=	0;		
	RF_WorkInfo.lBitsReceived 	=	0;		
	RF_WorkInfo.ucErrFlags		=	0;
	RF_WorkInfo.ucSaveErrState	=	0x00;
	RF_WorkInfo.ucCollPos     =   0;
	RF_WorkInfo.ucCmd = RF_CMD_IDLE;

	ucrxMultiple		   = 0x00;
	ucIntEnable 		   = 0x00;
	ucIntWait			   = 0x00;
	ucTimerCtl			   = 0x00;
	
#ifdef Debug_Isr
	gl_loalertNum=0;
	gl_LoAlert=gl_TxIRQ=gl_RxIRQ=gl_HiAlert_Idle=gl_HiAlert=gl_Idle=gl_timer=0;
	gl_err_flg1=gl_err_flg2=gl_err_flg3=gl_err_flg4=0;
	gl_err_flg5=gl_err_flg6=gl_err_flg5_1=gl_err_flg6_1=gl_err_flg5_2=gl_err_flg6_2=gl_err_flg5_3=gl_err_flg6_3=0;
#endif	

	
    ucTempData = 0x3F;
    s_vRFWriteReg(1,RF_REG_INTERRUPTEN,&ucTempData); 
    s_vRFWriteReg(1,RF_REG_INTERRUPTRQ,&ucTempData); 

    
    s_vRFFlushFIFO();    

    
	ucTempData = RF_CMD_IDLE;
	s_vRFWriteReg(1,RF_REG_COMMAND,&ucTempData); 
    
    
    s_vRFReadReg(1,RF_REG_FIFOLEVEL,&ucWaterLevelBak);
    ucTempData = RF_DEFAULT_WATER_LEVEL;
    s_vRFWriteReg(1,RF_REG_FIFOLEVEL,&ucTempData);
	
	
	s_vRFSetBitMask(RF_REG_CONTROL,0x04); 
		
	switch(ucCmd)
	{		
		case RF_CMD_IDLE:
		{
            ucTimerCtl  = 0x02;  
            ucIntEnable = 0x00; 
            ucIntWait   = 0x00;
			break;
		}

        case RF_CMD_WRITEE2:                
        {
            
            ucTimerCtl  = 0x00;
            ucIntEnable = 0x11; 
            ucIntWait   = 0x10; 
            break;
        }   
        case RF_CMD_READE2:                 
        {
            ucTimerCtl  = 0x00; 
            ucIntEnable = 0x07; 
            ucIntWait   = 0x04; 
            break;
        }
        case RF_CMD_LOADKEYE2:              
        {
            ucTimerCtl  = 0x00;
            ucIntEnable = 0x05; 
            ucIntWait   = 0x04; 
            break;
        }           
        case RF_CMD_LOADCONFIG:             
        {
            ucTimerCtl  = 0x00;
            ucIntEnable = 0x05; 
            ucIntWait   = 0x04; 
            break;
        }
        case RF_CMD_AUTHENT1:               
        {           
            ucTimerCtl  = 0x02;
            ucIntEnable = 0x05; 
            ucIntWait   = 0x04; 
            break;
        }
        case RF_CMD_CALCCRC:                
        {
            ucTimerCtl  = 0x00;          
            ucIntEnable = 0x11; 
            ucIntWait   = 0x10; 
            break;
        }
        case RF_CMD_AUTHENT2:               
        {
            ucTimerCtl  = 0x02;
            ucIntEnable = 0x04; 
            ucIntWait   = 0x04; 
            break;
        }
        case RF_CMD_LOADKEY: 
        {
            ucTimerCtl  = 0x00;
            ucIntEnable = 0x05; 
            ucIntWait   = 0x04; 
            break;
        }   
		
		case RF_CMD_RECEIVE:
		{
			s_vRFReadReg(1,RF_REG_BITFRAMING,&ucTempData);
			RF_WorkInfo.lBitsReceived = -(long)(ucTempData >> 4);
            ucTimerCtl  = 0x04;
            ucIntEnable = 0x06; 
            ucIntWait   = 0x04; 
			break;
		}
		case RF_CMD_TRANSMIT:
		{
            ucTimerCtl  = 0x00;          
            ucIntEnable = 0x05; 
            ucIntWait   = 0x04; 
			break;
		}
		case RF_CMD_TRANSCEIVE:
		{	
			
			s_vRFReadReg(1,RF_REG_BITFRAMING,&ucTempData);
			RF_WorkInfo.lBitsReceived = -(long)(ucTempData >> 4);
            ucTimerCtl  = 0x06;
 
            ucIntEnable = 0x3D; 
            ucIntWait   = 0x04; 
			break;
		}
		default:
		{
			RF_WorkInfo.ucCurResult = RET_RF_ERR_PARAM;
			break;
		}
	}
	if (RF_WorkInfo.ucCurResult == RET_RF_OK)
	{

		
		#ifdef RF_L1_RECDBG
		if(gl_usRecLen < (EMV_UART_BUFF_LEN- (RF_WorkInfo.ulSendBytes & 0xFF)*3 ))
		{		
			for(uc1 = 0; uc1<(RF_WorkInfo.ulSendBytes & 0xFF); uc1++)
			{
				vOneTwo((uchar *)RF_WorkInfo.aucBuffer + uc1, 1, gl_aucRecData + gl_usRecLen);		
				gl_usRecLen += 2;
				gl_aucRecData[gl_usRecLen] = ' ';
				gl_usRecLen++;
			}
		}
		#endif

		
        
        s_vRFReadReg(1,RF_REG_DECODERCONTROL,&ucTempData);         
        ucrxMultiple = ucTempData & 0x40;
        if (ucrxMultiple == 0)
        {
            
        }
        
        s_vRFClrBitMask(RF_REG_DECODERCONTROL,0x40);
		
        s_vRFWriteReg(1,RF_REG_TIMERCONTROL,&ucTimerCtl);
        
        
        if ((~ucTimerCtl & 0x02) != 0 )
        {
			s_vRFSetBitMask(RF_REG_CONTROL,0x02);			
        }
		
        
        s_vRFWriteReg(1,RF_REG_COMMAND,&ucCmd);               
        
		
		
        ucIntEnable |= 0x20;                        
        ucIntWait   |= 0x20;                      
        ucTempData = ucIntEnable | 0x80;	
        
		//AT skx
		EXTI_DisableITBit(RFINT);//mask BIT8 first
        s_vRFWriteReg(1,RF_REG_INTERRUPTEN,&ucTempData); 
        
		//EXTI_EnableEventBit(BIT8);//do not need enable eventbit
		sHal_EnCpuInt();//使能CPU中断
		EXTI_GenerateSWInterrupt(RFINT);//手动产生一次软件中断
        
		
		uiBeginTime = GetTimerCount();
            		 
		while(1)
		{
			if((RF_WorkInfo.ucINTSource & ucIntWait) != 0)
			{		
				break;
			}
			
			if( GetTimerCount()-uiBeginTime >100*10)
			//if( GetTimerCount()-uiBeginTime >10)
			{
				#ifdef RF_L1_RECDBG
				debug_printf(0,0,0,"*******************************************");
				debug_printf(0,0,0,"rf exchange cmd delay 10 s,INTSource:%02x",RF_WorkInfo.ucINTSource);
				debug_printf(0,0,0,"rf exchange cmd delay 10 s,INTSource:%02x",RF_WorkInfo.ucINTSource);
				ucTempData=0;
				s_vRFReadReg(1,RF_REG_PRIMARYSTATUS,&ucTempData);
				debug_printf(0,0,0,"rf interface state:%02x",ucTempData);
				#endif
				break;
			}
			
			//debug_printf(0,0,0,"rf echange loop,int:%02x\n",RF_WorkInfo.ucINTSource);
			
		}
		
	
		sHal_MaskCpuInt();
		
		
		s_vRFSetBitMask(RF_REG_CONTROL,0x04);	
        
        ucTempData = 0x3F;
        s_vRFWriteReg(1,RF_REG_INTERRUPTEN,&ucTempData); 
        s_vRFWriteReg(1,RF_REG_INTERRUPTRQ,&ucTempData);

        
        ucTempData = RF_CMD_IDLE;
        s_vRFWriteReg(1, RF_REG_COMMAND,&ucTempData); 
         
		
		RF_WorkInfo.ucErrFlags |= RF_WorkInfo.ucSaveErrState;

		

          
        if((ucCmd == RF_CMD_WRITEE2) || (ucCmd == RF_CMD_READE2)
           || (ucCmd == RF_CMD_LOADKEYE2) || (ucCmd == RF_CMD_LOADKEY))
        {   
            RF_WorkInfo.ucErrFlags &= 0x10;
        }
        else 
        {
            
            RF_WorkInfo.ucErrFlags &=0x1f;        
        }

		
		#ifdef Debug_Isr
		{
			debug_printf(0,0,0,"int:%d/%d/%d/%d/%d/%d/%d",gl_LoAlert,gl_TxIRQ,gl_RxIRQ,gl_HiAlert_Idle,gl_HiAlert,gl_Idle,gl_timer);
			debug_printf(0,0,0,"errFlg:%02x/%02x/%02x/%02x;%d",gl_err_flg1,gl_err_flg2,gl_err_flg3,gl_err_flg4,gl_loalertNum);
			debug_printf(0,0,0,"int:%02x,%02x/%02x,%02x/%02x,%02x/%02x,%02x",
				gl_err_flg5,gl_err_flg6,gl_err_flg5_1,gl_err_flg6_1,gl_err_flg5_2,gl_err_flg6_2,gl_err_flg5_3,gl_err_flg6_3);
			//getkey();
		}
		#endif
		
		if(RF_WorkInfo.ucErrFlags != 0)
		{
			if((RF_WorkInfo.ucErrFlags & 0x01) != 0)
			{
							
     	  		RF_WorkInfo.usErrNo = 0x03; 
				RF_WorkInfo.ucCurResult = RET_RF_ERR_COLLERR;

			}
			else if((RF_WorkInfo.ucErrFlags & 0x02) != 0)
			{
				
				RF_WorkInfo.usErrNo = 0x04; 
				RF_WorkInfo.ucCurResult = RET_RF_ERR_PARITY;
			}
			else if((RF_WorkInfo.ucErrFlags & 0x04) != 0)
			{
				
				RF_WorkInfo.usErrNo = 0x05; 
				RF_WorkInfo.ucCurResult = RET_RF_ERR_FRAMING;
			}
			else if( ((RF_WorkInfo.ucErrFlags & 0x10) != 0) )
			{
				RF_WorkInfo.ucCurResult = RET_RF_ERR_FIFO;
				
				s_vRFFlushFIFO();
				RF_WorkInfo.usErrNo = 0x06; 
				
			}
			else if((RF_WorkInfo.ucErrFlags & 0x08) != 0)
			{
				
				RF_WorkInfo.usErrNo = 0x07; 
				RF_WorkInfo.ucCurResult = RET_RF_ERR_CRC;
			}
			else if(RF_WorkInfo.ucCurResult == RET_RF_OK) 
			{
				RF_WorkInfo.usErrNo = 0x08; 
				RF_WorkInfo.ucCurResult = RET_RF_ERR_TRANSMIT;
			}
		} 

        if((ucCmd == RF_CMD_TRANSCEIVE) || (ucCmd == RF_CMD_RECEIVE))
        {
            
            s_vRFReadReg (1, RF_REG_SECONDARYSTATUS, &ucTempData);
	

            ucTempData &= 0x07;
            if (ucTempData != 0)
            {
                
                RF_WorkInfo.lBitsReceived += (RF_WorkInfo.ulBytesReceived-1) * 8 + ucTempData;
            }
            else 
            {
                
                RF_WorkInfo.lBitsReceived += RF_WorkInfo.ulBytesReceived * 8;
            } 
			
			#ifdef RF_L1_RECDBG
			if(gl_usRecLen < (EMV_UART_BUFF_LEN-strlen(PICC_RSP)-4- (RF_WorkInfo.ulBytesReceived & 0xFF)*3))
			{ 
				strcpy(gl_aucRecData+gl_usRecLen, "\n");
				gl_usRecLen += strlen("\n");
				
				strcpy(gl_aucRecData+gl_usRecLen, PICC_RSP);
				gl_usRecLen += strlen(PICC_RSP);
										
				for(uc1 = 0; uc1<(RF_WorkInfo.ulBytesReceived & 0xFF); uc1++)
				{
					vOneTwo((uchar *)RF_WorkInfo.aucBuffer + uc1, 1, gl_aucRecData + gl_usRecLen);	
					gl_usRecLen += 2;
					gl_aucRecData[gl_usRecLen] = ' ';
					gl_usRecLen++;
				}
				
				strcpy(gl_aucRecData+gl_usRecLen, "\n");
				gl_usRecLen += strlen("\n");
			}
			#endif
			
        }
        
        if (ucrxMultiple == 0) 
        {
            s_vRFClrBitMask(RF_REG_DECODERCONTROL,0x40);
        }
	}
  
	s_vRFWriteReg(1,RF_REG_FIFOLEVEL,&ucWaterLevelBak);
        
        /*
        debug_printf(0,0,0,"recv buf:%02x,%02x,%02x,%02x,len:%d",
                     RF_WorkInfo.aucBuffer[0],RF_WorkInfo.aucBuffer[1],
                     RF_WorkInfo.aucBuffer[2],RF_WorkInfo.aucBuffer[3],
                     RF_WorkInfo.lBitsReceived);
        //getkey();
	*/	 
	return RF_WorkInfo.ucCurResult;
}


uchar s_RF_ucREQA(uchar *pucResp)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData = 0x00;

     
     s_vRFSelectType('A');
	 
	 
	 s_RFSetTimer(RF_DEFAULT); 
	  
	 
     ucTempData = 0x03;
     s_vRFWriteReg(1, RF_REG_CHANNELREDUNDANCY, &ucTempData); 
	 
     
     s_vRFClrBitMask(RF_REG_CONTROL, 0x08);   
	 
     
     ucTempData = 0x07;
     s_vRFWriteReg(1,RF_REG_BITFRAMING, &ucTempData);        

	 
     RF_WorkInfo.ulSendBytes = 1;    
	 memset((uchar*)RF_WorkInfo.aucBuffer, 0, sizeof(RF_WorkInfo.aucBuffer));
     RF_WorkInfo.aucBuffer[0] = PICC_REQA;

	
#ifdef RF_L1_RECDBG
	if(gl_usRecLen < (EMV_UART_BUFF_LEN-strlen(REQA_CMD)))
	{
		strcpy(gl_aucRecData+gl_usRecLen, REQA_CMD);
		gl_usRecLen += strlen(REQA_CMD);
	}
#endif

	 ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);

 	 if(ucRet == RET_RF_ERR_TIMEOUT)
     {     	  
     	  RF_WorkInfo.usErrNo = 0xA1; 
          return RET_RF_ERR_TIMEOUT;
     }
	 
	 
     if((ucRet == RET_RF_OK) && (RF_WorkInfo.lBitsReceived != 16))
     {
     	 RF_WorkInfo.usErrNo = 0x14; 
         ucRet = RET_RF_ERR_TRANSMIT;
     }

     if(ucRet != RET_RF_OK)
     {
          if(ucRet == RET_RF_ERR_COLLERR)
		  {     	  	  
			  ucRet =  RET_RF_ERR_MULTI_CARD;
		  }
		  else
		  {
			  
			  ucRet = RET_RF_ERR_TRANSMIT;
		  }
     }
     else
     {
		
          if(((RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x10) && ((RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x08)
             && ((RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x04) && ((RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x02)
             && ((RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x01))
          {
               RF_WorkInfo.usErrNo = 0x10; 
               ucRet = RET_RF_ERR_PROTOCOL; 
          }
          else if((RF_WorkInfo.aucBuffer[0] & 0x20) != 0x00)
          {
               RF_WorkInfo.usErrNo = 0x11; 
               ucRet = RET_RF_ERR_PROTOCOL;
          }
          else if((RF_WorkInfo.aucBuffer[0] & 0xC0) == 0xC0)
          {
               RF_WorkInfo.usErrNo = 0x12; 
               ucRet = RET_RF_ERR_PROTOCOL;
          }
          else if((RF_WorkInfo.aucBuffer[1] & 0xf0) != 0x00)
          {
               RF_WorkInfo.usErrNo = 0x13; 
               ucRet = RET_RF_ERR_PROTOCOL;
          }
          else
          {
				switch(RF_WorkInfo.aucBuffer[0] & 0xC0)
				{
					case 0x00:
						 RF_WorkInfo.ucUIDLen = 4;
						 break;
					case 0x40:
						 RF_WorkInfo.ucUIDLen = 7;
						 break;
					default:
						 RF_WorkInfo.ucUIDLen = 10;
						 break;
				}
                memcpy(pucResp, (uchar*)RF_WorkInfo.aucBuffer, 2);
          }
    }
    return ucRet;
}

uchar s_RF_ucCardStatusCheck(uchar ucInProtocolNeed)
{
	if(RF_WorkInfo.ucRFOpen == 0)
		return RET_RF_ERR_NO_OPEN;

	if(RF_WorkInfo.ucMifActivateFlag == 0)
	{
        
		 return RET_RF_ERR_NOT_ACT;
	}
	if (ucInProtocolNeed == 1)
	{
		if(RF_WorkInfo.ucInProtocol == 0) 
		{
	         
			 return RET_RF_ERR_STATUS;
		}
	}
	else
	{
		if(RF_WorkInfo.ucInProtocol == 1) 
		{
			 return RET_RF_ERR_STATUS;
		}
	}
	return RET_RF_OK;
}



uchar s_RF_ucPOLL_EMV(uchar* pucMifType)
{
	uchar ucRet = RET_RF_OK;
	uchar ucTempData = 0x00;
	uchar ucATQA[2];
	uchar ucATQB[12];
	uchar i = 0;
	uchar ucSAK; 
	uchar ucTypeA = 0;
	uchar ucTypeB = 0;

	memset(ucATQA, 0x00, sizeof(ucATQA));
	memset(ucATQB, 0x00, sizeof(ucATQB));

	ucTypeA = 0;
	ucTypeB = 0;

	
	ucRet = s_RF_ucWUPA(ucATQA);
	if (ucRet != RET_RF_ERR_TIMEOUT) 
	{
		ucTypeA = 1;
		s_RF_vHALTA();
	}
	
	ucRet = s_RF_ucWUPB(&ucTempData,ucATQB);
	if (ucRet != RET_RF_ERR_TIMEOUT) 
	{
		ucTypeB = 1;
	}

	if(ucTypeA == 0 && ucTypeB == 0)
	{
		return E_NOT_DETECTED;
	}
	if(ucTypeA == 1 && ucTypeB == 1)
	{
		return RET_RF_ERR_MULTI_CARD;
	}

	if(ucTypeA == 0 && ucTypeB ==1)
	{
		
		ucRet = s_RF_ucWUPA(ucATQA);
		if (ucRet != RET_RF_ERR_TIMEOUT) 
		{
			ucTypeA = 1;

			if(ucTypeA == 1 && ucTypeB == 1)
			{
				return RET_RF_ERR_MULTI_CARD;
			}
			s_RF_vHALTA();
		}
	}

	if (ucTypeA == 1) 
	{
		for(i = 0; i<3; i++)
		{
			ucRet = s_RF_ucWUPA(ucATQA);
			if(ucRet != RET_RF_ERR_TIMEOUT)
				break;
		}
		if(ucRet == RET_RF_OK)
		{
			
			RF_WorkInfo.ucATQA[0] = ucATQA[0];
			RF_WorkInfo.ucATQA[1] = ucATQA[1];

			ucRet = s_RF_ucGetUID();
			if(ucRet == RET_RF_OK)
			{
				s_RF_ucCardTypeCheck(pucMifType);
				RF_WorkInfo.ucMifCardType = *pucMifType;
				RF_WorkInfo.ucCurType = RF_TYPEA;
				return RET_RF_OK;
			}

			return ucRet;
		}
		
		return ucRet;
	}
	else
	{
		if (ucTypeB == 0) 
		{
			
			return RET_RF_ERR_TIMEOUT;
		}

		for(i = 0; i<3; i++)
		{
			ucRet = s_RF_ucWUPB(&ucTempData,ucATQB);
			if(ucRet != RET_RF_ERR_TIMEOUT)
				break;
		}
		
		if(ucRet == RET_RF_OK)
		{
			memcpy((uchar*)RF_WorkInfo.ucATQB, ucATQB, ucTempData);
			RF_WorkInfo.ucMifCardType = RF_TYPE_B;
			*pucMifType = RF_WorkInfo.ucMifCardType;
			RF_WorkInfo.ucCurType = RF_TYPEB;
			return RET_RF_OK;
		}

		return ucRet;
	}
}


uchar s_RF_ucPOLL_EMV_Step(uchar* pucType)
{
	uchar ucRet = RET_RF_OK;
	uchar ucTempData = 0x00;
	uchar ucATQA[2];
	uchar ucATQB[12];
	uchar i = 0;
	uchar ucSAK; 
	uchar ucTypeA = 0;
	uchar ucTypeB = 0;

	memset(ucATQA, 0x00, sizeof(ucATQA));
	memset(ucATQB, 0x00, sizeof(ucATQB));

	ucTypeA = 0;
	ucTypeB = 0;

	
	ucRet = s_RF_ucWUPA(ucATQA);
	if (ucRet != RET_RF_ERR_TIMEOUT) 
	{
		ucTypeA = 1;
		s_RF_vHALTA();
	}
	
	ucRet = s_RF_ucWUPB(&ucTempData,ucATQB);
	if (ucRet != RET_RF_ERR_TIMEOUT) 
	{
		ucTypeB = 1;
	}

	if(ucTypeA == 0 && ucTypeB == 0)
	{
		return RET_RF_ERR_TIMEOUT;
	}
	if(ucTypeA == 1 && ucTypeB == 1)
	{
		s_RFResetCarrier();
		return RET_RF_ERR_MULTI_CARD;
	}

	if(ucTypeA == 0 && ucTypeB ==1)
	{
		ucRet = s_RF_ucWUPA(ucATQA);
		if (ucRet != RET_RF_ERR_TIMEOUT) 
		{
			ucTypeA = 1;
			if(ucTypeA == 1 && ucTypeB == 1)
			{
				s_RFResetCarrier();
				return RET_RF_ERR_MULTI_CARD;
			}
		}
		*pucType = 'B';
	}
	else		
		*pucType = 'A';
	
	return 0;
	
}


uchar s_RF_Activate(uchar ucType, uchar *pucMifType)
{	
	uchar ucRet = RET_RF_OK;
	uchar ucTempData = 0x00;
	uchar ucATQA[2];  
	uchar ucATQB[12];
	uchar i = 0;
	uchar ucSAK; 
	uchar ucTypeA = 0;
	uchar ucTypeB = 0;

	memset(ucATQA, 0x00, sizeof(ucATQA));
	memset(ucATQB, 0x00, sizeof(ucATQB));

	if (ucType == 'A') 
	{
		
		for(i = 0; i<3; i++) 
		{
			ucRet = s_RF_ucWUPA(ucATQA);
			if(ucRet != RET_RF_ERR_TIMEOUT)
				break;
		}
		if(ucRet == RET_RF_OK)
		{
			RF_WorkInfo.ucATQA[0] = ucATQA[0];
			RF_WorkInfo.ucATQA[1] = ucATQA[1];
			ucRet = s_RF_ucGetUID();
			if(ucRet == RET_RF_OK)
			{
				s_RF_ucCardTypeCheck(pucMifType);
				RF_WorkInfo.ucMifCardType = *pucMifType;
				RF_WorkInfo.ucCurType = RF_TYPEA;
				return RET_RF_OK;
			}

 			s_RFResetCarrier();
			return ucRet;
		}
		
		
		s_RFResetCarrier();
		return ucRet;
	}
	else
	{
		
		for(i = 0; i<3; i++)
		{
			ucRet = s_RF_ucWUPB(&ucTempData,ucATQB);
			if(ucRet != RET_RF_ERR_TIMEOUT)
				break;
		}
		
		if(ucRet == RET_RF_OK)
		{
			memcpy((uchar*)RF_WorkInfo.ucATQB, ucATQB, ucTempData);
			RF_WorkInfo.ucMifCardType = RF_TYPE_B;
			*pucMifType = RF_WorkInfo.ucMifCardType;
			RF_WorkInfo.ucCurType = RF_TYPEB;
			return RET_RF_OK;
		}
		s_RFResetCarrier();
		return ucRet;
	}
}
uchar  s_RF_ucRemoval(void)
{
	uchar ucRet = RET_RF_OK;
	uchar ucTempData[12];
	uchar ucTemp = 0x00;
	uchar ucReSendNo = 0;

	if(RF_WorkInfo.ucCurType == RF_TYPEA)
	{ 
	    for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	    {
	          ucRet = s_RF_ucWUPA(ucTempData);
	          if(ucRet != RET_RF_ERR_TIMEOUT)
	          { 
	    		   s_RF_vHALTA(); 
				   break;
	          }

	          if((ucRet == RET_RF_ERR_TIMEOUT) && (ucReSendNo == 2))
	          { 
	                return RET_RF_OK;
	          }
	    }
	}

	else
	{
	    for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	    { 
	          ucRet = s_RF_ucWUPB(&ucTemp, ucTempData);
	          if(ucRet != RET_RF_ERR_TIMEOUT)
	          { 
				   break;
	          }
	          if((ucRet == RET_RF_ERR_TIMEOUT) && (ucReSendNo == 2))
	          {
	                return RET_RF_OK;
	          }

	    }
	}
	return RET_RF_ERR_CARD_EXIST;
}


void s_vRFSetBitMask (uchar ucRegAddr, uchar ucBitMask)
{
    uchar ucTemp;

    s_vRFReadReg(1, ucRegAddr, &ucTemp);
    ucTemp |= ucBitMask;
    s_vRFWriteReg(1, ucRegAddr, &ucTemp);
}


void s_vRFClrBitMask(uchar ucRegAddr, uchar ucBitMask)
{
    uchar ucTemp;

    s_vRFReadReg(1, ucRegAddr, &ucTemp); 
    ucTemp &= ~ucBitMask;

    s_vRFWriteReg(1, ucRegAddr, &ucTemp); 
}


void s_vRFFlushFIFO(void)
{
    uchar ucTemp = 0;
   s_vRFReadReg(1, RF_REG_CONTROL, &ucTemp);
    ucTemp |= 0x01;
    s_vRFWriteReg(1, RF_REG_CONTROL, &ucTemp);
}




uchar s_M1Authority(uchar Type,uchar BlkNo,uchar *Pwd,uchar *SerialNo)
{
    uchar ucRet = 0;
	uchar ucTemp = 0;
	uchar i = 0;
	uchar k = 0; 
	uchar j = 0;
	uchar ucKeyMode = 0;

	ucRet = s_RF_ucCardStatusCheck(0);
	if(ucRet)
	{
		if(ucRet == RET_RF_ERR_NOT_ACT)
			return 3;
		return ucRet;
	}
	
    if((Pwd == NULL) || (SerialNo == NULL))
    {
         return RET_RF_ERR_PARAM;
    }
    
    switch(Type)
    {
        case 'a':
		case 'A':
			ucKeyMode = PICC_AUTHENTA;
			break;
		case 'b':
		case 'B':
			ucKeyMode = PICC_AUTHENTB;
			break;
		default:
			return RET_RF_ERR_PARAM;
    }
    
	 if((RF_WorkInfo.ucMifCardType == RF_TYPE_S50)
		&& (c_para.card_type_check_w == 0 || (c_para.card_type_check_w == 1 && c_para.card_type_check_val == 0)))
	 {
	     if(BlkNo > 63)
		 {
			 return RET_RF_ERR_PARAM;
		 }
	 }	

	RF_WorkInfo.ucM1Stage = 1;

    for(i=0;i<6;i++)
    {		
        k=(Pwd[i] & 0xF0 ) >> 4;
        j= Pwd[i] & 0x0F;
        RF_WorkInfo.aucBuffer[i*2] = (( k << 4 ) ^ 0xF0) + k;
        RF_WorkInfo.aucBuffer[i*2+1] = (( j << 4 ) ^ 0xF0) + j; 
    }
    
    s_RFSetTimer(560);
    RF_WorkInfo.ulSendBytes = 12;     
    
    ucRet = s_RFExchangeCmd(RF_CMD_LOADKEY);
	RF_WorkInfo.ucM1Stage = 0;
	
    if(ucRet != RET_RF_OK) 
    {
        return ucRet;
    }
	
	
    
    ucTemp = 0x07;
    s_vRFWriteReg(1,RF_REG_CHANNELREDUNDANCY,&ucTemp); 
    
    s_RFSetTimer(560);
    
	RF_WorkInfo.aucBuffer[0]=ucKeyMode;
        
    RF_WorkInfo.aucBuffer[1] = BlkNo;
    memcpy((uchar*)RF_WorkInfo.aucBuffer+2, SerialNo, 4);
    RF_WorkInfo.ulSendBytes = 6;    
        
    
	RF_WorkInfo.ucM1Stage = 1;
    ucRet = s_RFExchangeCmd(RF_CMD_AUTHENT1);
	RF_WorkInfo.ucM1Stage = 0;

    if(ucRet != RET_RF_OK) 
    {
        return ucRet;
    } 
    
	
    
    ucTemp = 0x03;
    s_vRFWriteReg(1,RF_REG_CHANNELREDUNDANCY,&ucTemp); 

    RF_WorkInfo.ulSendBytes = 0;      
    
	RF_WorkInfo.ucM1Stage = 1;
    
    ucRet = s_RFExchangeCmd(RF_CMD_AUTHENT2);

	RF_WorkInfo.ucM1Stage = 0;
    if(ucRet != RET_RF_OK) 
    {
        return ucRet;
    }  

    s_vRFReadReg(1,RF_REG_CONTROL,&ucTemp);

    if((ucTemp & 0x08) !=0) 
    {
        return RET_RF_OK;
    }
    else
    { 
        return RET_RF_ERR_AUTH;
    }    
	 
}


uchar s_M1ReadBlock(uchar BlkNo,uchar *BlkValue)
{
     uchar ucRet = RET_RF_OK;
	 uchar ucTemp = 0;

	ucRet = s_RF_ucCardStatusCheck(0);
	if(ucRet)
	{
		if(ucRet == RET_RF_ERR_NOT_ACT)
			return 3;
		return ucRet;
	}
	
     if(BlkValue == NULL)
     {
        
		 return RET_RF_ERR_PARAM;
     }
	 
	 if((RF_WorkInfo.ucMifCardType == RF_TYPE_S50)
		&& (c_para.card_type_check_w == 0 || (c_para.card_type_check_w == 1 && c_para.card_type_check_val == 0)))
	 {
	     if(BlkNo > 63)
		 {
			 return RET_RF_ERR_PARAM;
		 }
	 }
	
	 s_RFSetTimer(1024);
	 RF_WorkInfo.ucM1Stage = 1;

    ucTemp = 0x0F;
    s_vRFWriteReg(1,RF_REG_CHANNELREDUNDANCY,&ucTemp);  


	 RF_WorkInfo.aucBuffer[0] = PICC_READ;
	 RF_WorkInfo.aucBuffer[1] = BlkNo;
 
    RF_WorkInfo.ulSendBytes = 2;  
    
    ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);

	 RF_WorkInfo.ucM1Stage = 0;
    s_vRFReadReg(1,RF_REG_SECONDARYSTATUS,&ucTemp);
	
	if(ucRet == RET_RF_ERR_CRC && RF_WorkInfo.ulBytesReceived == 1 
		&& (ucTemp&0x07) == 4 && RF_WorkInfo.aucBuffer[0]==0x04)
		return 4;

	if(ucRet)
		return ucRet;

	if(RF_WorkInfo.ulBytesReceived != 16)
		return RET_RF_ERR_TRANSMIT;

	memcpy(BlkValue,(uchar*)RF_WorkInfo.aucBuffer,16);
	return 0;
}


uchar s_M1WriteBlock(uchar BlkNo,uchar *BlkValue)
{
     uchar ucRet = RET_RF_OK;
	 uchar ucTemp = 0;

	ucRet = s_RF_ucCardStatusCheck(0);
	if(ucRet)
	{
		if(ucRet == RET_RF_ERR_NOT_ACT)
			return 3;
		return ucRet;
	}
	
     if(BlkValue == NULL)
     {
		 return RET_RF_ERR_PARAM;
     }
	 
	 
	 if((RF_WorkInfo.ucMifCardType == RF_TYPE_S50)
		&& (c_para.card_type_check_w == 0 || (c_para.card_type_check_w == 1 && c_para.card_type_check_val == 0)))
	 {
	     if(BlkNo > 63)
		 {
			 return RET_RF_ERR_PARAM;
		 }
	 }
	 
	 RF_WorkInfo.aucBuffer[0] = PICC_WRITE;
	 RF_WorkInfo.aucBuffer[1] = BlkNo;

	 RF_WorkInfo.ulSendBytes = 2;

	 s_RFSetTimer(560);

	 RF_WorkInfo.ucM1Stage = 1;
	 
    ucTemp = 0x07;
    s_vRFWriteReg(1,RF_REG_CHANNELREDUNDANCY,&ucTemp);  
    
    RF_WorkInfo.ulSendBytes = 2;      
    RF_WorkInfo.aucBuffer[0] = PICC_WRITE;
    RF_WorkInfo.aucBuffer[1] = BlkNo;      
    
    ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);

	RF_WorkInfo.ucM1Stage = 0;

    s_vRFReadReg(1,RF_REG_SECONDARYSTATUS,&ucTemp);
	if(ucRet && ucRet != RET_RF_ERR_CRC)
		return ucRet;

	if((ucTemp & 0x07) != 4)
		return RET_RF_ERR_TRANSMIT;
	if(RF_WorkInfo.ulBytesReceived != 1)
		return RET_RF_ERR_TRANSMIT;
	ucTemp = RF_WorkInfo.aucBuffer[0] & 0x0f;
	if(ucTemp == 0x04)
		return 4;
	if(ucTemp != 0x0a)
		return RET_RF_ERR_TRANSMIT;
	
    s_RFSetTimer(1024);     

    RF_WorkInfo.ulSendBytes = 16;
    memcpy((uchar*)RF_WorkInfo.aucBuffer, BlkValue, 16);

	RF_WorkInfo.ucM1Stage = 1;
    
	ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);

	RF_WorkInfo.ucM1Stage = 0;
    
	s_vRFReadReg(1,RF_REG_SECONDARYSTATUS,&ucTemp);	
	if(ucRet && ucRet != RET_RF_ERR_CRC)
		return RET_RF_ERR_TRANSMIT;

	if((ucTemp & 0x07) != 4)
		return RET_RF_ERR_TRANSMIT;
	if(RF_WorkInfo.ulBytesReceived != 1)
		return RET_RF_ERR_TRANSMIT;
	ucTemp = RF_WorkInfo.aucBuffer[0] & 0x0f;
	if(ucTemp == 0x04)
		return 4;
	if(ucTemp != 0x0a)
		return RET_RF_ERR_TRANSMIT;
	
	return RET_RF_OK;
}


uchar s_M1Operate(uchar Type, uchar BlkNo, uchar *Value, uchar UpdateBlkNo)
{
    uchar ucRet = RET_RF_OK;
	uchar ucTemp = 0;

	ucRet = s_RF_ucCardStatusCheck(0);
	if(ucRet)
	{
		if(ucRet == RET_RF_ERR_NOT_ACT)
			return 3;
		return ucRet;
	}
	
    if(Value == NULL)
    {
		 return RET_RF_ERR_PARAM;
    }
	     
	 
	 if((RF_WorkInfo.ucMifCardType == RF_TYPE_S50)
		&& (c_para.card_type_check_w == 0 || (c_para.card_type_check_w == 1 && c_para.card_type_check_val == 0)))
	 {
	     if(BlkNo > 63 || UpdateBlkNo > 63)
		 {
			 return RET_RF_ERR_PARAM;
		 }
	 }
	
	
	switch(Type)
	{
	    case '+':
			RF_WorkInfo.aucBuffer[0] = PICC_INCREMENT;
			break;
		case '-':
			RF_WorkInfo.aucBuffer[0] = PICC_DECREMENT;
			break;
		case '>':
			RF_WorkInfo.aucBuffer[0] = PICC_RESTORE;
			break;
		default:
			return RET_RF_ERR_PARAM;
	}

	RF_WorkInfo.aucBuffer[1] = BlkNo;
	RF_WorkInfo.ulSendBytes = 2;

	s_RFSetTimer(2048);
	RF_WorkInfo.ucM1Stage = 1;

	 
    ucTemp = 0x07;
    s_vRFWriteReg(1,RF_REG_CHANNELREDUNDANCY,&ucTemp);	
	
	 ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);
	 RF_WorkInfo.ucM1Stage = 0;

	s_vRFReadReg(1,RF_REG_SECONDARYSTATUS,&ucTemp);	

	if (ucRet && ucRet != RET_RF_ERR_CRC)
		return ucRet; 
	
	if ((ucTemp & 0x07) != 4)
		return RET_RF_ERR_TRANSMIT;
	
	if (RF_WorkInfo.ulBytesReceived != 1)
		return RET_RF_ERR_TRANSMIT;
	
	ucTemp = RF_WorkInfo.aucBuffer[0] & 0x0F;
	if (ucTemp == 0x04)
		return 4;
	
	if (ucTemp != 0x0A)
		return RET_RF_ERR_TRANSMIT;
	

	
	memcpy((uchar*)RF_WorkInfo.aucBuffer, Value,4);
	RF_WorkInfo.ulSendBytes = 4; 
	RF_WorkInfo.ucM1Stage = 1;
	 	
	 
	 
	
	

	ucRet = s_RFExchangeCmd(RF_CMD_TRANSMIT);
	 RF_WorkInfo.ucM1Stage = 0;
	if (ucRet)
		return ucRet;
	
	
	
	
	RF_WorkInfo.aucBuffer[0] = PICC_TRANSFER;
	RF_WorkInfo.aucBuffer[1] = UpdateBlkNo;
	RF_WorkInfo.ulSendBytes = 2; 
	 RF_WorkInfo.ucM1Stage = 1;
	 	
	 ucRet = s_RFExchangeCmd(RF_CMD_TRANSCEIVE);
	 RF_WorkInfo.ucM1Stage = 0;

	s_vRFReadReg(1,RF_REG_SECONDARYSTATUS,&ucTemp);	

	if (ucRet && ucRet != RET_RF_ERR_CRC) 
		return ucRet; 
	
	if ((ucTemp & 0x07) != 4)
		return RET_RF_ERR_TRANSMIT;
	
	if (RF_WorkInfo.ulBytesReceived != 1)
		return RET_RF_ERR_TRANSMIT;
	
	ucTemp = RF_WorkInfo.aucBuffer[0] & 0x0F;
	if (ucTemp == 0x04)
		return 4;
	
	if (ucTemp != 0x0A)
		return RET_RF_ERR_TRANSMIT;
	
	return RET_RF_OK;
} 

