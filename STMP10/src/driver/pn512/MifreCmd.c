/**************************************************************
			本驱动为PN512底层驱动，创建于2010-06-08
			进行严格三层划分
			
**************************************************************/
#define FOR_PN512_DEVICE
#include "Mifre_TmDef.h"
#include <string.h>
#include "comm.h"
#include "kf701dh.h"
#include "systimer.h"


extern PICC_PARA c_para;
extern PN512_RF_WORKSTRUCT PN512_RF_WorkInfo;
ulong PN512_gl_RF_ulFSList[16] = {16,24,32,40,48,64,96,128,256,256,256,256,256,256,256,256};

extern void WaitNuS(u32 x);
extern void EXTI_DisableITBit(u32 EXTI_Line);

//uchar PN512_B_gl_RxThreshold = 0x84;//接收灵敏度可调节 add by skx


// 初始化工作变量
void PN512_s_RF_vInitWorkInfo(void)
{
	uchar ucRFOpenFlg = 0;

	ucRFOpenFlg = PN512_RF_WorkInfo.ucRFOpen;
	memset((uchar *)&PN512_RF_WorkInfo, 0x00, sizeof(PN512_RF_WorkInfo));
	PN512_RF_WorkInfo.ucRFOpen = ucRFOpenFlg;
	
	PN512_RF_WorkInfo.ucMifCardType = RF_TYPE_S50;
	PN512_RF_WorkInfo.ucCurType = RF_TYPEA; 
	PN512_RF_WorkInfo.ucMifActivateFlag = 0;       
	PN512_RF_WorkInfo.FSC = 32;
	PN512_RF_WorkInfo.FSD = 256;
	
	c_para.card_buffer_val = PN512_RF_WorkInfo.FSC;
}


void PN512_s_vRFSelectType(uchar ucRFType)
{
     uchar ucTemp = 0x00;

     switch(ucRFType)
   	 {
     case 'a':
		 case 'A':
			ucTemp = 0x80;
			PN512_s_vRFWriteReg(1,PN512_TXMODE_REG,&ucTemp); // ISO14443A, 106kBits/s, TxCRCEn
			ucTemp = 0x80;
			PN512_s_vRFWriteReg(1,PN512_RXMODE_REG,&ucTemp); // ISO14443A, 106KBits/s, RxCRCEn, RxNoErr
			ucTemp = 0x40;
			PN512_s_vRFWriteReg(1,PN512_TXAUTO_REG,&ucTemp); // Enable Force100%ASK

			ucTemp = 0x84;
			PN512_s_vRFWriteReg(1,PN512_RXSEL_REG,&ucTemp); // RxWait = 6, Modulation signal from the internal analog part

			//ucTemp = c_para.card_RxThreshold_val; // 0x84;
			ucTemp = 0x84; // 0x84;	
			//ucTemp = 0x80; // 0x84;	
                        
			PN512_s_vRFWriteReg(1, PN512_RXTHRESHOLD_REG, &ucTemp);    //* MinLevel = 5; CollLevel = 5 *
						
			ucTemp = c_para.a_conduct_val;
			PN512_s_vRFWriteReg(1,PN512_CWGSP_REG,&ucTemp);//电导系数

			// added by liuxl according to RC522 20070918
			ucTemp = 0x39;// 01 "6363" CRC_PRE
			PN512_s_vRFWriteReg(1, PN512_MODE_REG,&ucTemp);//配置CRC_PRE
 
			ucTemp = 0x26; // Pulse width
			// LOW value: #clocksLOW   = (ModWidth % 8)+1.
			// HIGH value: #clocksHIGH = 16 - #clocksLOW.
			PN512_s_vRFWriteReg(1,PN512_MODWIDTH_REG,&ucTemp);

			ucTemp = 0x00; // TYPEB: RxSOF, RxEOF, TxSOF, TxEOF, TxEGT = 1(2ETU), SOF Width = 11ETU
			PN512_s_vRFWriteReg(1,PN512_TYPEB_REG,&ucTemp);
                        
                        ucTemp = 0x40;//50->40 VAx_reg_RFCfgReg_RxGain_value
                        PN512_s_vRFWriteReg(1,PN512_RFCFG_REG,&ucTemp);
                        //13/07/08 3cm
                        //trace_debug_printf("_glbPiccSetPara[%02x]\n",_glbPiccSetPara);

                        
                        
                        
			ucTemp = c_para.b_modulate_val; // c_para.B_ModGsP_Val;//0x17;
			PN512_s_vRFWriteReg(1,PN512_MODGSP_REG,&ucTemp);// 调制深度
                        
                        if(  (_glbPiccSetPara&0x20)==0 )
                        {
			ucTemp = 0x94; // 0x84->94;	
			//ucTemp = 0x80; // 0x84;	
			PN512_s_vRFWriteReg(1, PN512_RXTHRESHOLD_REG, &ucTemp);    //* MinLevel = 5; CollLevel = 5 *
                          
                        
                        ucTemp = 0x40;//50->40 VAx_reg_RFCfgReg_RxGain_value
                        PN512_s_vRFWriteReg(1,PN512_RFCFG_REG,&ucTemp);
                        
			ucTemp = 0x3f; // 3a->3d->4d->3f对接收解调器的设置
			PN512_s_vRFWriteReg(1, PN512_DEMOD_REG, &ucTemp); //VAx_reg_DemodReg_value
                        
                        ucTemp = 0;
			PN512_s_vRFWriteReg(1,PN512_MODGSP_REG,&ucTemp);// 调制深度
                        
                        }
                        
			break;
			
		 case 'b':
		 case 'B':
			ucTemp = 0x83;
			PN512_s_vRFWriteReg(1,PN512_TXMODE_REG,&ucTemp); // ISO14443B, 106kBits/s, TxCRCEn
			ucTemp = 0x83;
			PN512_s_vRFWriteReg(1,PN512_RXMODE_REG,&ucTemp); // ISO14443B, 106KBits/s, RxCRCEn, RxNoErr
			ucTemp = 0x00;
			PN512_s_vRFWriteReg(1,PN512_TXAUTO_REG,&ucTemp); // Disable  Force100%ASK
			ucTemp = 0x84;
			PN512_s_vRFWriteReg(1,PN512_RXSEL_REG,&ucTemp); // RxWait = 3

			//ucTemp = PN512_B_gl_RxThreshold;//接收灵敏度
			ucTemp=c_para.card_RxThreshold_val;//接收灵敏度
			PN512_s_vRFWriteReg(1,PN512_RXTHRESHOLD_REG,&ucTemp);

			ucTemp = c_para.a_conduct_val;
			PN512_s_vRFWriteReg(1,PN512_CWGSP_REG,&ucTemp);//电导系数 VAx_reg_CWGsP_value_B

			//  for debug according to RC522
			ucTemp = 0x3B;// 11 "FFFF" CRC_PRE
			PN512_s_vRFWriteReg(1, PN512_MODE_REG,&ucTemp);		
			// debug end*/  
  
			ucTemp = 0xC1; //0xD1; // TYPEB: RxSOF, RxEOF, TxSOF, TxEOF, TxEGT = 1(2ETU), SOF Width = 11ETU
//			ucTemp = 0xD1; // TYPEB: RxSOF, RxEOF, TxSOF, TxEOF, TxEGT = 1(2ETU), SOF Width = 11ETU
			PN512_s_vRFWriteReg(1,PN512_TYPEB_REG,&ucTemp);

			ucTemp = c_para.b_modulate_val;//0x17; VAx_reg_ModGsP_value_B
			PN512_s_vRFWriteReg(1,PN512_MODGSP_REG,&ucTemp);// 调制深度
                        

                        //13/07/06
                        //trace_debug_printf("B _glbPiccSetPara[%02x]\n",_glbPiccSetPara);
//#if 0   //3cm
                        //if(  (_glbPiccSetPara&0x04)==0 )
                        {      
//#if 0
                        //test
                        PN512_s_vRFReadReg(1,PN512_DEMOD_REG,&ucTemp);
                        trace_debug_printf("PN512_DEMOD_REG val[%x]\n",ucTemp);
                          
			ucTemp = 0x4d; // 3a->3d->4d对接收解调器的设置
			PN512_s_vRFWriteReg(1, PN512_DEMOD_REG, &ucTemp); //VAx_reg_DemodReg_value_B
//#endif                    
                        
			ucTemp = 0x54; //58->68->58->48->50->52->51->53->54->4d->50->54->64(3cm)采用默认值 PN512测外部RF场强灵敏度（注意与接收器的接收灵敏度区分） 12/09/21 48->58
			PN512_s_vRFWriteReg(1,PN512_RFCFG_REG,&ucTemp);//VAx_reg_RFCfgReg_RxGain_value_B|
                        
#if 0                       
			//ucTemp=c_para.card_RxThreshold_val;//接收灵敏度
                        ucTemp = 0x55;
			PN512_s_vRFWriteReg(1,PN512_RXTHRESHOLD_REG,&ucTemp);
#endif                        
                        }
//#endif       
      

                        
//#if 0
                        
                        //13/07/08
                        //if(  (_glbPiccSetPara&0x01)==0 )
                        {
                        //0cm
//#if 0
			//ucTemp=c_para.card_RxThreshold_val;//接收灵敏度
                        ucTemp = 0x75; //75->65->85->a5->55 VAx_reg_RxThreshold_MinLevel_value_B|VAx_reg_RxThreshold_CollLevel_value_B
			PN512_s_vRFWriteReg(1,PN512_RXTHRESHOLD_REG,&ucTemp);
//#endif
                        
                        ucTemp = 0x84;//85->84->86->83->85->80->83->88->84
                        PN512_s_vRFWriteReg(1,PN512_GSNON_REG,&ucTemp);//VAx_reg_CWGsN_value_B|VAx_reg_ModGsN_value_B
                        }
//#endif                        
                        //13/07/09 3cm
                        if(  (_glbPiccSetPara&0x04)==0 )
                        {
                          ucTemp = 0x55; //75->65->85->a5->55 VAx_reg_RxThreshold_MinLevel_value_B|VAx_reg_RxThreshold_CollLevel_value_B
			  PN512_s_vRFWriteReg(1,PN512_RXTHRESHOLD_REG,&ucTemp);
                          
			  ucTemp = 0x58; //58->68->58->48->50->52->51->53->54->4d->50->54->64(3cm)采用默认值 PN512测外部RF场强灵敏度（注意与接收器的接收灵敏度区分） 12/09/21 48->58
			  PN512_s_vRFWriteReg(1,PN512_RFCFG_REG,&ucTemp);//VAx_reg_RFCfgReg_RxGain_value_B|
                          
			  ucTemp = 0x3f; // 3a->3d->4d对接收解调器的设置
			  PN512_s_vRFWriteReg(1, PN512_DEMOD_REG, &ucTemp); //VAx_reg_DemodReg_value_B
                          
                          ucTemp = 0x88;//85->84->86->83->85->80->83->88->84->88
                          PN512_s_vRFWriteReg(1,PN512_GSNON_REG,&ucTemp);//VAx_reg_CWGsN_value_B|VAx_reg_ModGsN_value_B
                          
                        }
                        
                        //13/07/10 1.5cm
                        if(  (_glbPiccSetPara&0x02)==0 )
                        {
                          ucTemp = 0x70; //75->65->85->a5->55->70 VAx_reg_RxThreshold_MinLevel_value_B|VAx_reg_RxThreshold_CollLevel_value_B
			  PN512_s_vRFWriteReg(1,PN512_RXTHRESHOLD_REG,&ucTemp);
			  ucTemp = 0x48; //58->68->58->48->50->52->51->53->54->4d->50->54->64(3cm)->58采用默认值 PN512测外部RF场强灵敏度（注意与接收器的接收灵敏度区分） 12/09/21 48->58
			  PN512_s_vRFWriteReg(1,PN512_RFCFG_REG,&ucTemp);//VAx_reg_RFCfgReg_RxGain_value_B|
                          
			  ucTemp = 0x4d; // 3a->3d->4d对接收解调器的设置
			  PN512_s_vRFWriteReg(1, PN512_DEMOD_REG, &ucTemp); //VAx_reg_DemodReg_value_B
                          
                        ucTemp = 0x88;//85->84->86->83->85->80->83->88->84->88
                        PN512_s_vRFWriteReg(1,PN512_GSNON_REG,&ucTemp);//VAx_reg_CWGsN_value_B|VAx_reg_ModGsN_value_B
                          
                        }
                        
			ucTemp = 0x26; // Pulse width
			// LOW value: #clocksLOW   = (ModWidth % 8)+1.
			// HIGH value: #clocksHIGH = 16 - #clocksLOW.
			PN512_s_vRFWriteReg(1,PN512_MODWIDTH_REG,&ucTemp);
			// set TxLastBits to 0
			ucTemp = 0x00; 
			PN512_s_vRFWriteReg(1,PN512_BITFRAMING_REG, &ucTemp);
			// 冲突后数据都置0
			PN512_s_vRFSetBitMask(PN512_COLL_REG,0x80); // Zero After Coll		 	
			break;
			
		 case 'f':
		 case 'F':
			
			//ucTemp = 0x10;//在s_init中统一配置了
			//PN512_s_vRFWriteReg(1,PN512_CONTROL_REG,&ucTemp); // set PN512 as a Initiator Mode
			
			ucTemp = 0x00;
			PN512_s_vRFWriteReg(1,PN512_BITFRAMING_REG,&ucTemp); // set felica send with a standard 8 bit framing

			ucTemp = (0x38|0x80);//Not MSBFirst,对于felica卡必须配置为MSBFirst,从抓取波形看无影响
			PN512_s_vRFWriteReg(1,PN512_MODE_REG,&ucTemp); // set CRC_PER is 0x00,0x00 for felica,MSBFirst 
			
			// ucTemp = 0xA2;  // FeliCa, 424kBits/s, TxCRCEn
			ucTemp = 0x92;
			PN512_s_vRFWriteReg(1,PN512_TXMODE_REG,&ucTemp); // FeliCa, 212kBits/s, TxCRCEn

			// ucTemp = 0xAA; // FeliCa, 424KBits/s, RxCRCEn, RxNoErr
			ucTemp = 0x9A;//目前配置为单帧接收，但felica卡在polling期间必须设置为多帧接收！
			PN512_s_vRFWriteReg(1,PN512_RXMODE_REG,&ucTemp); // FeliCa, 212KBits/s, RxCRCEn, RxNoErr
			
			ucTemp = 0x00;
			PN512_s_vRFWriteReg(1,PN512_TXAUTO_REG,&ucTemp); // Disable  Force100%ASK

			//ucTemp = 0x10;//在s_init中统一配置了
			//PN512_s_vRFWriteReg(1,PN512_TXSEL_REG,&ucTemp); // TX1，TX2 Modulation signal (envelope) from the internal coder

			ucTemp = 0x84;//Rxwait=6对于felica还需要确认调节
			PN512_s_vRFWriteReg(1,PN512_RXSEL_REG,&ucTemp); // RxWait = 6, Modulation signal from the internal analog part
			
			//ucTemp = PN512_B_gl_RxThreshold;//接收灵敏度
			ucTemp=c_para.card_RxThreshold_val;
			PN512_s_vRFWriteReg(1,PN512_RXTHRESHOLD_REG,&ucTemp);

			ucTemp = c_para.a_conduct_val;
			PN512_s_vRFWriteReg(1,PN512_CWGSP_REG,&ucTemp);//电导系数			
		
			ucTemp = 0x4D; // 采用默认值，对接收解调器的设置
			PN512_s_vRFWriteReg(1, PN512_DEMOD_REG, &ucTemp); 


			ucTemp = 0x00; // felica专用配置
			PN512_s_vRFWriteReg(1, PN512_FELNFC1_REG, &ucTemp);//FelSyncLen：0xB2,0x4D;DataLenMin:0x00(最小包长度0)

			ucTemp = 0x00; // felica专用配置
			PN512_s_vRFWriteReg(1, PN512_FELNFC2_REG, &ucTemp);//DataLenMax:0x00(最大包长度256,须根据实际配置)

			//AT
			//ucTemp = (0x00 |0x10); //disable Parity，此配置对felica无影响，不进行配置采用默认值
			//PN512_s_vRFWriteReg(1, PN512_MANUALRCV_REG, &ucTemp);//enable Parity
 
			//ucTemp = 0x48; //采用默认值 PN512测外部RF场强灵敏度（注意与接收器的接收灵敏度区分）
			ucTemp = 0x60; //采用默认值 PN512测外部RF场强灵敏度（注意与接收器的接收灵敏度区分） 12/09/21 48->60
                        //ucTemp = 0x70;
			PN512_s_vRFWriteReg(1,PN512_RFCFG_REG,&ucTemp);

			ucTemp = c_para.f_modulate_val;//0x17;
			PN512_s_vRFWriteReg(1,PN512_MODGSP_REG,&ucTemp);// 调制深度，felica调制深度（10%）

			ucTemp = 0x00; // TYPEB: RxSOF, RxEOF, TxSOF, TxEOF, TxEGT = 1(2ETU), SOF Width = 11ETU
			PN512_s_vRFWriteReg(1,PN512_TYPEB_REG,&ucTemp);

			ucTemp = 0x26; // Pulse width
			// LOW value: #clocksLOW   = (ModWidth % 8)+1.
			// HIGH value: #clocksHIGH = 16 - #clocksLOW.
			PN512_s_vRFWriteReg(1,PN512_MODWIDTH_REG,&ucTemp);
			break;
		 default:
		 	break;
   	 }

	 DelayMs(6);
}

uchar PN512_s_RF_ucCardTypeCheck(uchar *pCardType)
{
	uchar ucSAK = 0;

	if (PN512_RF_WorkInfo.ucUIDLen == 4)
		ucSAK = PN512_RF_WorkInfo.ucSAK1;
	else if(PN512_RF_WorkInfo.ucUIDLen == 7)
		ucSAK = PN512_RF_WorkInfo.ucSAK2;
	else if(PN512_RF_WorkInfo.ucUIDLen == 10)
		ucSAK = PN512_RF_WorkInfo.ucSAK3;
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


uchar PN512_s_RF_ucWUPA(uchar *pucResp)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData = 0x00;

     // 先将载波调制为TypeA型
     PN512_s_vRFSelectType('A');
	 
	 // 设置定时器
	 //PN512_s_RFSetTimer(RF_DEFAULT); //超时时间延长
     //PN512_s_RFSetTimer(RF_FWTMIN);
	 PN512_s_RFSetTimer(50);
 
	 // 禁止 RxCRC 和 TxCRC , 允许奇偶校验
     ucTempData = 0x00;
     PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
	 PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);
	 // RxNoErr If set to logic 1, a not valid received data stream (less than 4 bits received)
	 // will be ignored. The receiver will remain active.
	 // If the bit RxNoErr in register RxModeReg is set to logic 1, bit RxIRq is only set
	 // to logic 1 when data bytes are available in the FIFO.
	 PN512_s_vRFSetBitMask(PN512_RXMODE_REG, 0x08); //尝试解决无卡时产生接收中断的问题

     // 禁止crypto1单元
     PN512_s_vRFClrBitMask(PN512_STATUS2_REG, 0x08);

//	 ucTempData = 0x83;
//	 PN512_s_vRFWriteReg(1, PN512_TXCONTROL_REG, &ucTempData);// for debug liuxl 20071008
//	 PN512_s_vRFSetBitMask(PN512_TXCONTROL_REG, 0x03);// Tx2RF-En, Tx1RF-En enable
	 
     // set TxLastBits to 7, 短帧格式
     ucTempData = 0x07; 
     PN512_s_vRFWriteReg(1,PN512_BITFRAMING_REG, &ucTempData);

	 // 冲突后数据都置0
     PN512_s_vRFClrBitMask(PN512_COLL_REG,0x80); // Zero After Coll

	 // 准备发送数据: 命令码
     PN512_RF_WorkInfo.ulSendBytes = 1;    // how many bytes to send
	 	 memset((uchar*)PN512_RF_WorkInfo.aucBuffer, 0, sizeof(PN512_RF_WorkInfo.aucBuffer));
     PN512_RF_WorkInfo.aucBuffer[0] = PICC_WUPA; // WUPA命令 

	 ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);

 	 if(ucRet == RET_RF_ERR_TIMEOUT)
     {     	  
     	  PN512_RF_WorkInfo.usErrNo = 0xA1; // timeout err
          return RET_RF_ERR_TIMEOUT;
     }

	 
	 // 处理接收数据
     // 期望接收到两个完整字节: 卡类型识别号(2字节)
     if((ucRet == RET_RF_OK) && (PN512_RF_WorkInfo.lBitsReceived != 16))
     {
     	 PN512_RF_WorkInfo.usErrNo = 0x14; // received data != 16 bits
         ucRet = RET_RF_ERR_TRANSMIT;
     }

     if(ucRet != RET_RF_OK)
     {
          if(ucRet == RET_RF_ERR_MULTI_CARD)
		  {     	  	  
			  ucRet =  RET_RF_ERR_MULTI_CARD;
		  }
		  else
		  {
			  // 通信错误
			  ucRet = RET_RF_ERR_TRANSMIT;
		  }
     }
     else
     {
		// 1. ATQA第一个字节的第B8和B7位必须为00 01 10之一，否则按协议错误处理
		// 2. ATQA第一个字节的第B6为必须为0，否则按协议错误处理
		// 3. ATQA第一个字节的第B5到B1位必须为10000 01000 00100 00010 00001之一，否则按协议错误处理。
		// 4. ATQA第二个字节的第B8到B5位必须为0000，否则按协议错误处理。

          if(((PN512_RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x10) && ((PN512_RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x08)
             && ((PN512_RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x04) && ((PN512_RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x02)
             && ((PN512_RF_WorkInfo.aucBuffer[0] & 0x1f) != 0x01))
          {
               PN512_RF_WorkInfo.usErrNo = 0x10; // ATQA BYTE 1 bit 5~1 err 
               ucRet = RET_RF_ERR_PROTOCOL; // 协议错误
          }
          else if((PN512_RF_WorkInfo.aucBuffer[0] & 0x20) != 0x00)
          {
               PN512_RF_WorkInfo.usErrNo = 0x11; // ATQA BYTE 1 bit 6 err 
               ucRet = RET_RF_ERR_PROTOCOL; // 协议错误
          }
          else if((PN512_RF_WorkInfo.aucBuffer[0] & 0xC0) == 0xC0)
          {
               PN512_RF_WorkInfo.usErrNo = 0x12; // ATQA BYTE 1 bit 8~7 err 
               ucRet = RET_RF_ERR_PROTOCOL; // 协议错误
          }
          else if((PN512_RF_WorkInfo.aucBuffer[1] & 0xf0) != 0x00)
          {
               PN512_RF_WorkInfo.usErrNo = 0x13; // ATQA BYTE 2 err 
               ucRet = RET_RF_ERR_PROTOCOL; // 协议错误
          }
          else
          {
				switch(PN512_RF_WorkInfo.aucBuffer[0] & 0xC0)
				{
					case 0x00:
						 PN512_RF_WorkInfo.ucUIDLen = 4;
						 break;
					case 0x40:
						 PN512_RF_WorkInfo.ucUIDLen = 7;
						 break;
					default:
						 PN512_RF_WorkInfo.ucUIDLen = 10;
						 break;
				}
                memcpy(pucResp, (uchar*)PN512_RF_WorkInfo.aucBuffer, 2);
          }
    }
	 //for debug 避免在detect接口中连续调用时出现错误
//	PN512_s_RFDelay10ETUs(RF_TPDEALY/10);//(RF_FDTMIN/10); // 延时500us再发送下一条命令
	//s_RFDelay100Us(6);// 20071224 延时600us
    return ucRet;
}



uchar PN512_s_RF_ucAnticoll(uchar ucSEL, uchar* pucUID)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData = 0x00;
     uchar ucTempVal = 0x00;
     uchar i = 0;

	 // 设置定时器
     //PN512_s_RFSetTimer(RF_FWTMIN);
	 PN512_s_RFSetTimer(RF_DEFAULT); //超时时间延长
	 
     // 冲突后数据都置0,
     PN512_s_vRFClrBitMask(PN512_COLL_REG,0x80); // Zero After Coll

     // 禁止crypto1单元
     PN512_s_vRFClrBitMask(PN512_STATUS2_REG, 0x08);

	 // 禁止 RxCRC 和 TxCRC
     ucTempData = 0x00;
     PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
	 PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);

     // set TxLastBits to 0, 标准帧格式
     ucTempData = 0x00;   
     PN512_s_vRFWriteReg(1,PN512_BITFRAMING_REG, &ucTempData);
	 // added end

     PN512_RF_WorkInfo.ucAnticol = ucSEL;
	 PN512_RF_WorkInfo.aucBuffer[0] = ucSEL; // SEL
     PN512_RF_WorkInfo.aucBuffer[1] = 0x20;
     PN512_RF_WorkInfo.ulSendBytes = 2;    // how many bytes to send
	 // 执行命令
     ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);
	
	 PN512_s_vRFSetBitMask(PN512_COLL_REG,0x80); // 20080421

	 if(ucRet == RET_RF_ERR_TIMEOUT)  // 超时无响应则直接返回
     {
          PN512_RF_WorkInfo.usErrNo = 0xA2; // timeout err
          return RET_RF_ERR_TIMEOUT;
     }

     if(ucRet == RET_RF_OK)
     {
           if(PN512_RF_WorkInfo.lBitsReceived != 40) // not 5 bytes answered
           {
                ucRet = RET_RF_ERR_TRANSMIT;
           }
           else
           {
                switch(PN512_RF_WorkInfo.ucUIDLen)
				{
				    case 4:
					{
                       if(ucSEL == PICC_ANTICOLL1)
                       {
                             if(PN512_RF_WorkInfo.aucBuffer[0] == 0x88)
                             {
                             		PN512_RF_WorkInfo.usErrNo = 0x20; // UIDLEN = 4, BYTE0=88 err
                                    ucRet = RET_RF_ERR_PROTOCOL; // 协议错误
                             }
                       }
                       break;
					}

					case 7:
					{
                       if(ucSEL == PICC_ANTICOLL1)
                       {
                             if(PN512_RF_WorkInfo.aucBuffer[0] != 0x88)
                             {
                             		PN512_RF_WorkInfo.usErrNo = 0x21; // UIDLEN = 7, BYTE0!=88 err
                                    ucRet = RET_RF_ERR_PROTOCOL; // 协议错误
                             }
                       }

					   if(ucSEL == PICC_ANTICOLL2)
                       {
                             if(PN512_RF_WorkInfo.aucBuffer[0] == 0x88)
                             {
                                    PN512_RF_WorkInfo.usErrNo = 0x22; // UIDLEN = 7, BYTE4=88 err
                                    ucRet = RET_RF_ERR_PROTOCOL; // 协议错误
                             }
                       }
					   break;
                    }
                    default:
					{
                       if(ucSEL == PICC_ANTICOLL1)
                       {
                             if(PN512_RF_WorkInfo.aucBuffer[0] != 0x88)
                             {
                             		PN512_RF_WorkInfo.usErrNo = 0x23; // UIDLEN = 10, BYTE0!=88 err
                                    ucRet = RET_RF_ERR_PROTOCOL; // 协议错误
                             }
                       }

					   if(ucSEL == PICC_ANTICOLL2)
                       {
                             if(PN512_RF_WorkInfo.aucBuffer[0] != 0x88)
                             {
                             		PN512_RF_WorkInfo.usErrNo = 0x24; // UIDLEN = 10, BYTE4!=88 err
                                    ucRet = RET_RF_ERR_PROTOCOL; // 协议错误
                             }
                       }
					   break;
					}
				}

                // 校验BCC
                ucTempVal = 0;
                for(i = 0; i < 4; i++)
                {
                      ucTempVal ^= PN512_RF_WorkInfo.aucBuffer[i];
                }
                if(ucTempVal != PN512_RF_WorkInfo.aucBuffer[4])
                {
                      PN512_RF_WorkInfo.usErrNo = 0x25; // uid bcc err
                      ucRet = RET_RF_ERR_TRANSMIT; // 通信错误
                }
         }

         memcpy(pucUID,(uchar*)PN512_RF_WorkInfo.aucBuffer,5);
     }
     else if(ucRet == RET_RF_ERR_MULTI_CARD)
     {
         ucRet = RET_RF_ERR_MULTI_CARD;
     }
	 else
	 {
		 ucRet = RET_RF_ERR_TRANSMIT;
	 }

	 PN512_RF_WorkInfo.ucAnticol = 0;
	 
	 //for debug 避免在detect接口中连续调用时出现错误
	 //PN512_s_RFDelay10ETUs(RF_TPDEALY/10);*/
//	 PN512_s_RFDelay10ETUs(RF_FDTMIN/10); // 延时500us再发送下一条命令
	 //s_RFDelay100Us(6);// 20071224 延时600us
     return ucRet;
}


uchar PN512_s_RF_ucSelect(uchar ucSEL, uchar* pucUID, uchar* pucSAK)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData = 0x00;

     // 设置定时器
	 PN512_s_RFSetTimer(RF_DEFAULT); //超时时间延长
//     PN512_s_RFSetTimer(RF_FWTMIN);
	 
     // RxCRC,TxCRC, ISO14443A
     ucTempData = 0x80;
     PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
	 PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);

     // 禁止crypto1单元
     PN512_s_vRFClrBitMask(PN512_STATUS2_REG, 0x08);    // disable crypto 1 unit


     // set TxLastBits to 0, 标准帧格式
     ucTempData = 0x00;   
     PN512_s_vRFWriteReg(1,PN512_BITFRAMING_REG, &ucTempData);
	 // added end

     PN512_RF_WorkInfo.ulSendBytes = 7;
     // 导入UID参数
     memcpy((uchar*)(PN512_RF_WorkInfo.aucBuffer+2), pucUID, 5);

	 PN512_RF_WorkInfo.aucBuffer[0] = ucSEL;
     PN512_RF_WorkInfo.aucBuffer[1] = 0x70;       // number of bytes send

	 // 发送命令并接收响应
     ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);

     if(ucRet == RET_RF_ERR_TIMEOUT) // 超时无响应则直接返回
     {
          PN512_RF_WorkInfo.usErrNo = 0xA3; // timeout err
          return RET_RF_ERR_TIMEOUT;
     }

     if(ucRet != RET_RF_OK)
     {
          ucRet = RET_RF_ERR_TRANSMIT; // 返回通信错误
     }
     else
     {
           if(PN512_RF_WorkInfo.lBitsReceived != 8)
           {
           		PN512_RF_WorkInfo.usErrNo = 0x30; // received bits err
               // 一个字节没有收满
               ucRet = RET_RF_ERR_TRANSMIT;
           }
           else
           {
			   // B3位判断UID是否完整
			   switch(PN512_RF_WorkInfo.ucUIDLen)
			   {
			       case 4:
				   {
                       if((PN512_RF_WorkInfo.aucBuffer[0] & 0x04) == 0x04)
					   {
					   		 PN512_RF_WorkInfo.usErrNo = 0x31; // UID err
							 ucRet = RET_RF_ERR_PROTOCOL;
					   }
					   PN512_RF_WorkInfo.ucSAK1 = PN512_RF_WorkInfo.aucBuffer[0];
					   break;
				   }

				   case 7:
				   {
                       if(ucSEL == PICC_ANTICOLL1)
					   {
						   if((PN512_RF_WorkInfo.aucBuffer[0] & 0x04) != 0x04)
						   {
					   		     PN512_RF_WorkInfo.usErrNo = 0x32; // UID err
								 ucRet = RET_RF_ERR_PROTOCOL;
						   }
						   PN512_RF_WorkInfo.ucSAK1 = PN512_RF_WorkInfo.aucBuffer[0];
					   }

					   if(ucSEL == PICC_ANTICOLL2)
					   {
						   if((PN512_RF_WorkInfo.aucBuffer[0] & 0x04) == 0x04)
						   {
					   		     PN512_RF_WorkInfo.usErrNo = 0x33; // UID err
								 ucRet = RET_RF_ERR_PROTOCOL;
						   }
						   PN512_RF_WorkInfo.ucSAK2 = PN512_RF_WorkInfo.aucBuffer[0];
					   }
					   break;
				   }

				   default:
				   {
					   if(ucSEL == PICC_ANTICOLL1)
					   {
						   if((PN512_RF_WorkInfo.aucBuffer[0] & 0x04) != 0x04)
						   {
					   		     PN512_RF_WorkInfo.usErrNo = 0x34; // UID err
								 ucRet = RET_RF_ERR_PROTOCOL;
						   }
						   PN512_RF_WorkInfo.ucSAK1 = PN512_RF_WorkInfo.aucBuffer[0];
					   }

					   if(ucSEL == PICC_ANTICOLL2)
					   {
						   if((PN512_RF_WorkInfo.aucBuffer[0] & 0x04) != 0x04)
						   {
					   		     PN512_RF_WorkInfo.usErrNo = 0x35; // UID err
								 ucRet = RET_RF_ERR_PROTOCOL;
						   }
						   PN512_RF_WorkInfo.ucSAK2 = PN512_RF_WorkInfo.aucBuffer[0];
					   }

					   if(ucSEL == PICC_ANTICOLL3)
					   {
						   if((PN512_RF_WorkInfo.aucBuffer[0] & 0x04) == 0x04)
						   {
					   		     PN512_RF_WorkInfo.usErrNo = 0x36; // UID err
								 ucRet = RET_RF_ERR_PROTOCOL;
						   }
						   PN512_RF_WorkInfo.ucSAK3 = PN512_RF_WorkInfo.aucBuffer[0];
					   }
					   break;
				   }
			   }
           }
      }
	  *pucSAK = PN512_RF_WorkInfo.aucBuffer[0];
	  
	 //for debug 避免在detect接口中连续调用时出现错误
//	 PN512_s_RFDelay10ETUs(RF_TPDEALY/10);//PN512_s_RFDelay10ETUs(RF_FDTMIN/10); // 延时500us再发送下一条命令
	 //s_RFDelay100Us(6);// 20071224 延时600us
      return ucRet;
}


uchar PN512_s_RF_ucGetUID(void)
{
       uchar ucRet = RET_RF_OK;
       uchar ucTempData = 0x00;
       uchar ucUIDTemp[5];
       uchar ucSAK = 0x00;
       uchar ucReSendNo = 0x00;

       memset(ucUIDTemp, 0x00, sizeof(ucUIDTemp));

       for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	   {
		   ucTempData = PICC_ANTICOLL1; // 第一级冲突
		   ucRet = PN512_s_RF_ucAnticoll(ucTempData, ucUIDTemp);
		   if(ucRet != RET_RF_ERR_TIMEOUT)
		   {
				break;
		   }
	   }

       if(ucRet != RET_RF_OK)
       {
		   return ucRet;
       }

	   for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	   {
			ucRet = PN512_s_RF_ucSelect(ucTempData, ucUIDTemp, &ucSAK);
			if(ucRet != RET_RF_ERR_TIMEOUT)
			{
				 break;
			}
	   }

	   if(ucRet != RET_RF_OK)
	   {
		   return ucRet;
	   }

	   // 保存第一级UID参数
       memcpy((uchar*)PN512_RF_WorkInfo.ucUIDCL1, ucUIDTemp, 5);
	   if(PN512_RF_WorkInfo.ucUIDLen == 4)
	   {
            return RET_RF_OK;
	   }

       // 获取第二级UID参数
       for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	   {
		   ucTempData = PICC_ANTICOLL2; // 第二级冲突
		   ucRet = PN512_s_RF_ucAnticoll(ucTempData, ucUIDTemp);
		   if(ucRet != RET_RF_ERR_TIMEOUT)
		   {
			   break;
		   }
	   }

	   if(ucRet != RET_RF_OK)
	   {
		   return ucRet;
	   }

	   for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	   {
			ucRet = PN512_s_RF_ucSelect(ucTempData, ucUIDTemp, &ucSAK);
			if(ucRet != RET_RF_ERR_TIMEOUT)
			{
				 break;
			}
	   }

       if(ucRet != RET_RF_OK)
	   {
		   return ucRet;
	   }
       // 保存第二级UID参数
       memcpy((uchar*)PN512_RF_WorkInfo.ucUIDCL2, ucUIDTemp, 5);

       if(PN512_RF_WorkInfo.ucUIDLen == 7)
	   {
		    return RET_RF_OK;
	   }

       // 获取第三级UID参数
	   for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	   {
			  ucTempData = PICC_ANTICOLL3; // 第三级冲突
			  ucRet = PN512_s_RF_ucAnticoll(ucTempData, ucUIDTemp);
			  if(ucRet != RET_RF_ERR_TIMEOUT)
			  {
				  break;
			  }
	   }

	   if(ucRet != RET_RF_OK)
	   {
			return ucRet;
	   }

       for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	   {
			ucRet = PN512_s_RF_ucSelect(ucTempData, ucUIDTemp, &ucSAK);
			if(ucRet != RET_RF_ERR_TIMEOUT)
			{
				 break;
			}
	   }

       if(ucRet != RET_RF_OK)
	   {
		   return ucRet;
	   }

	   // 保存第三级UID参数
       memcpy((uchar*)PN512_RF_WorkInfo.ucUIDCL3, ucUIDTemp, 5);

	   return RET_RF_OK;
}


uchar PN512_s_RF_ucActPro(uchar* pucOutLen, uchar* pucATSData)
{
       uchar ucRet = RET_RF_OK;
       uchar ucTempData = 0x00;
       uchar ucSAK = 0x00;
       uchar ucResp[2];
       uchar ucReSendNo = 0;

       // 超时情况下，最多发送三次同一条命令
       for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
       {
           // 先进行WUPA操作

           ucRet = PN512_s_RF_ucWUPA(ucResp);
		   if(ucRet != RET_RF_ERR_TIMEOUT)
           {
                 break;
           }
       }

       // 其他错误情况则不重发，直接返回给终端
       if(ucRet != RET_RF_OK)
       {
           PN512_RF_WorkInfo.usErrNo = 0x70; // ActPro WUPA err
		   return ucRet;
       }
       else
       {
             // 先判断获取的ATQA参数是否已保存的ATQA参数一致
             if(memcmp(ucResp, (uchar*)PN512_RF_WorkInfo.ucATQA, 2))
             {
             		PN512_RF_WorkInfo.usErrNo = 0x71; // ActPro ATQA err
                    return RET_RF_ERR_PROTOCOL;
             }
       }

       // 进行第一级SELECT
	   for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	   {
           ucTempData = PICC_ANTICOLL1;
           ucRet = PN512_s_RF_ucSelect(ucTempData, (uchar*)PN512_RF_WorkInfo.ucUIDCL1, &ucSAK);
		   if(ucRet != RET_RF_ERR_TIMEOUT)
           {
                break;
           }
	   }

       if(ucRet != RET_RF_OK)
       {
       	   PN512_RF_WorkInfo.usErrNo |= 0x100; // ActPro SELECT err
		   return ucRet;
	   }
       // UID为4个字节，一次Select操作即可使PICC进入ACTIVATE状态
	   if(PN512_RF_WorkInfo.ucUIDLen == 4)
	   {
		   goto ACTEND;
	   }

       // 进行第二级SELECT
       for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	   {
          ucTempData = PICC_ANTICOLL2;
          ucRet = PN512_s_RF_ucSelect(ucTempData, (uchar*)PN512_RF_WorkInfo.ucUIDCL2, &ucSAK);
          if(ucRet != RET_RF_ERR_TIMEOUT)
          {
                break;
          }
	   }

       if(ucRet != RET_RF_OK)
	   {
	       PN512_RF_WorkInfo.usErrNo |= 0x200; // ActPro SELECT err
		   return ucRet;
	   }
       // UID为7个字节，两次Select操作即可使PICC进入ACTIVATE状态
       if(PN512_RF_WorkInfo.ucUIDLen == 7)
	   {
		   goto ACTEND;
	   }

       // 进行第三级SELECT
       // UID为10个字节，三次Select操作即可使PICC进入ACTIVATE状态
       for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
	   {
          ucTempData = PICC_ANTICOLL3;
          ucRet = PN512_s_RF_ucSelect(ucTempData, (uchar*)PN512_RF_WorkInfo.ucUIDCL3, &ucSAK);
          if(ucRet != RET_RF_ERR_TIMEOUT)
          {
                break;
          }
	   }

       if(ucRet != RET_RF_OK)
	   {
	       PN512_RF_WorkInfo.usErrNo |= 0x300; // ActPro SELECT err
		   return ucRet;
	   }

ACTEND:
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

	   return RET_RF_OK;
}


uchar PN512_s_RF_ucRATS(uchar* pucOutLen, uchar* pucATSData)
{
    uchar ucTempData = 0x00;
    uchar ucRet = RET_RF_OK;
    uchar tucTemp[2];
    uchar i = 0;
    uint  uiFWITemp = 0x00, uiSFGITemp = 0x00;
    ulong ulTemp = 0;
    
    *pucOutLen = 0;

    PN512_RF_WorkInfo.FSC    = 32;
    PN512_RF_WorkInfo.FSD    = 256;
	PN512_RF_WorkInfo.ucFWI  = 4;
    PN512_RF_WorkInfo.ulFWT  = 560; // (256*16/Fc)*2^4 + 3*2^4ETU
    PN512_RF_WorkInfo.ucSFGI = 0;
    PN512_RF_WorkInfo.ulSFGT = 60; // PayPass要求，当SFGI=0时，应至少延时500us
    // 默认不采用CID和NAD
    PN512_RF_WorkInfo.ucCIDFlag = 0;
    PN512_RF_WorkInfo.ucNADFlag = 0;

    tucTemp[0] = 0xE0;
    tucTemp[1] = 0x80;  // FSD = 256Bytes

    // 设置定时器
    PN512_s_RFSetTimer(RF_FWTRATS); // 定时560ETU 
    // RxCRC,TxCRC, ISO14443
    ucTempData = 0x80;
    PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
	PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);

    // 准备发送数据: 命令码
    PN512_RF_WorkInfo.ulSendBytes = 2;     // how many bytes to send
    memcpy((uchar*)PN512_RF_WorkInfo.aucBuffer, tucTemp, PN512_RF_WorkInfo.ulSendBytes);

	// 发送命令并接收响应
    ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);

    if(ucRet == RET_RF_ERR_TIMEOUT)  // 超时无响应则直接返回
    {
          PN512_RF_WorkInfo.usErrNo = 0xA4; // timeout err
          return RET_RF_ERR_TIMEOUT;
    }

	if(ucRet != RET_RF_OK)
	{
		  return RET_RF_ERR_TRANSMIT;
	}
	else
    {
           i = 0;
           ulTemp = PN512_RF_WorkInfo.ulBytesReceived;
           //if(PN512_RF_WorkInfo.ulBytesReceived != PN512_RF_WorkInfo.aucBuffer[0]) // TL字节错误
           if(ulTemp != PN512_RF_WorkInfo.aucBuffer[0]) // TL字节错误
           {
                PN512_RF_WorkInfo.usErrNo = 0x80; // ATS TL err
                ucRet = RET_RF_ERR_TRANSMIT; // 通信错误
				goto RATSEND;
           }
           // 保存ATS参数
           *pucOutLen = PN512_RF_WorkInfo.aucBuffer[0];
           memcpy(pucATSData,(uchar*)PN512_RF_WorkInfo.aucBuffer,PN512_RF_WorkInfo.ulBytesReceived);

           // 如果ATS不包含T0，TA1、TB1或者TC1，则按默认参数设置
           if(PN512_RF_WorkInfo.aucBuffer[0] <= 1) // 只包含TL字节
           {
               goto RATSEND;
           }

           i = 1; // 取T0字节

           // T0字节定义FSC和TA1、TB1、TC1的存在与否
           if((PN512_RF_WorkInfo.aucBuffer[1] & 0x80) != 0x00) // Bit8必须为0
           {
                PN512_RF_WorkInfo.usErrNo = 0x81; // ATS T0 err
			    ucRet = RET_RF_ERR_PROTOCOL; // 协议错误
                goto RATSEND;
           }
           else
           {
                // 卡片可接收最大帧长度
			    PN512_RF_WorkInfo.FSC = PN512_gl_RF_ulFSList[PN512_RF_WorkInfo.aucBuffer[1] & 0x0F];
           }
           // TA1字节定义设备与卡之间的通信速率，默认为106KBits/s
		   // 不管TA1回送任何值，读卡芯片与卡片之间仍按106KBits/s进行通信
           if((PN512_RF_WorkInfo.aucBuffer[1] & 0x10) == 0x10)  // TA1字节
           {
                i++;
				if((PN512_RF_WorkInfo.aucBuffer[i] & 0x08) != 0x00)
				{
				     PN512_RF_WorkInfo.usErrNo = 0x82; // ATS TA1 err
					 ucRet = RET_RF_ERR_PROTOCOL; // 协议错误
                     goto RATSEND;
				}
           }
           // TB1字节定义FWT和SFGT
           if((PN512_RF_WorkInfo.aucBuffer[1] & 0x20) == 0x20) // TB1字节
           {
               i++;
               PN512_RF_WorkInfo.ucFWI = (PN512_RF_WorkInfo.aucBuffer[i] & 0x0F0) >> 4;

			   uiFWITemp = (uint)0x01 << PN512_RF_WorkInfo.ucFWI;
			   PN512_RF_WorkInfo.ulFWT = (ulong)(32 + 3) * uiFWITemp; //(256*16/Fc)*2^FWI + 3*2^FWIETU

			   PN512_RF_WorkInfo.ucSFGI = PN512_RF_WorkInfo.aucBuffer[i] & 0x0F;

			   uiSFGITemp = (uint)0x01 << PN512_RF_WorkInfo.ucSFGI;
			   PN512_RF_WorkInfo.ulSFGT = (ulong)(32 + 3) * uiSFGITemp; //(256*16/Fc)*2^SFGI + 3*2^SFGI ETU;
           }

           if((PN512_RF_WorkInfo.aucBuffer[1] & 0x40) == 0x40)  // TC1
           {
               i++;
               // TC1的高6位必须为000000
               if((PN512_RF_WorkInfo.aucBuffer[i] & 0xFC) != 0x00)
               {
                   PN512_RF_WorkInfo.usErrNo = 0x83; // ATS TC1 err
				   ucRet = RET_RF_ERR_PROTOCOL; // 协议错误
				   goto RATSEND;
               }
               // 不管PICC返回什么，均按不支持NAD和CID处理
               // 判断CID
               if(PN512_RF_WorkInfo.aucBuffer[i] & 0x02)
               {
                   PN512_RF_WorkInfo.ucCIDFlag = 1;
               }
               else
               {
                   PN512_RF_WorkInfo.ucCIDFlag = 0;
               }
               // 判断NAD
               if(PN512_RF_WorkInfo.aucBuffer[i] & 0x01)
               {
                   PN512_RF_WorkInfo.ucNADFlag = 1;
               }
               else
               {
                   PN512_RF_WorkInfo.ucNADFlag = 0;
               }

               ucRet = RET_RF_OK;
           }
    }

RATSEND:

	// liuxl 20070929
	if (ucRet == RET_RF_OK)
	{
		PN512_RF_WorkInfo.ucInProtocol = 1;
		if(c_para.card_buffer_w == 1 && c_para.card_buffer_val)
			PN512_RF_WorkInfo.FSC = c_para.card_buffer_val;
	}
	// end
	
    PN512_s_RFDelay10ETUs(PN512_RF_WorkInfo.ulSFGT/10); // 延时SFGT再发送下一条命令
//    s_RF_DelayUs(100);
    //s_RFDelay100Us(6);// 20071224 延时600us
    return ucRet;
}


void PN512_s_RF_vHALTA(void)
{
    //uchar ucRet = RET_RF_OK;
    uchar ucTempData;

	// set TxLastBits to 0
	ucTempData = 0x00; 
	PN512_s_vRFWriteReg(1,PN512_BITFRAMING_REG, &ucTempData);
	
    // 定时器时间设置
    PN512_s_RFSetTimer(560); // 定时560ETU

    // TxCRC, Parity enable
    ucTempData = 0x80;
    PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);

    // 准备发送数据: 停止命令码, 0
    PN512_RF_WorkInfo.ulSendBytes = 2;
    PN512_RF_WorkInfo.aucBuffer[0] = PICC_HALT ;     // Halt command code
    PN512_RF_WorkInfo.aucBuffer[1] = 0x00;
	// dummy address
	
    // 执行命令   
    //ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);
    PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);
    // 当PCD发送完HLTA后，始终认为PICC已成功接收并正确执行了HLTA命令，而不会去理会
	// PICC会回送任何响应。

    // 复位命令寄存器
  //  ucTempData = 0x00;
  //  PN512_s_vRFWriteReg(1, PN512_COMMAND_REG, &ucTempData);

}



uchar  PN512_s_RF_ucWUPB(uchar* pucOutLen, uchar* pucOut)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData;
	 uint uiFWITemp = 0x00;

	 PN512_s_vRFFlushFIFO();        //empty FIFO

     // 将载波调制为TypeB型
     PN512_s_vRFSelectType('B');
     
	 // RxCRC and TxCRC enable, ISO14443B
     ucTempData = 0x83;
     PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
     ucTempData = 0x83;
	 PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);

 //    ucTempData = 0x83;
//	 PN512_s_vRFWriteReg(1, PN512_TXCONTROL_REG, &ucTempData);// for debug
//	 PN512_s_vRFSetBitMask(PN512_TXCONTROL_REG, 0x03);// Tx2RF-En, Tx1RF-En enable

     // 禁止crypto1单元
     PN512_s_vRFClrBitMask(PN512_STATUS2_REG, 0x08);
     PN512_s_RFSetTimer(RF_FWTWUPB*3);  //RF_DEFAULT);// 超时等待60ETU     
     
     

	//准备发送数据: 命令码
     PN512_RF_WorkInfo.ulSendBytes = 3;    // how many bytes to send
     PN512_RF_WorkInfo.aucBuffer[0] = PICC_APF;
     PN512_RF_WorkInfo.aucBuffer[1] = 0x00;       // AFI = 0x00
     PN512_RF_WorkInfo.aucBuffer[2] = PICC_WUPB;  // TimeSlot = 0, WUPB

     // 执行命令
     ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);

     if(ucRet == RET_RF_ERR_TIMEOUT) // 超时无响应
     {
           PN512_RF_WorkInfo.usErrNo = 0xA5; // timeout err
           return RET_RF_ERR_TIMEOUT;
     }
     if(ucRet != RET_RF_OK)
     {
           ucRet = RET_RF_ERR_TRANSMIT;
     }
     else
     {
		    if(PN512_RF_WorkInfo.ulBytesReceived != 12)
            {
             	  PN512_RF_WorkInfo.usErrNo = 0x50; // received bytes err
                  ucRet = RET_RF_ERR_TRANSMIT;
				  goto WUPBEND;
            }
            if(PN512_RF_WorkInfo.aucBuffer[0] != 0x50)
            {
             	  PN512_RF_WorkInfo.usErrNo = 0x51; // received byte 1 err
				  ucRet = RET_RF_ERR_PROTOCOL;
                  goto WUPBEND;
            }

		    *pucOutLen = PN512_RF_WorkInfo.ulBytesReceived;
            memcpy(pucOut, (uchar*)PN512_RF_WorkInfo.aucBuffer, PN512_RF_WorkInfo.ulBytesReceived);

            PN512_RF_WorkInfo.ucUIDLen = 4;
            // 保存TypeB的UID序列号 第2~5字节
            memcpy((uchar*)PN512_RF_WorkInfo.ucUIDB, (uchar*)(PN512_RF_WorkInfo.aucBuffer + 1), 4);

            // 第6~9字节为Application Data,一般为0000，终端可以不予理会

            // 第10字节为BitRate，PCD与PICC一般只支持106KBit
            // 不管该字节回送任何值，均按106KBits/s进行通信
            if((PN512_RF_WorkInfo.aucBuffer[9] & 0x08) != 0x00)
            {
             	  PN512_RF_WorkInfo.usErrNo = 0x52; // received byte 10 err
				   ucRet = RET_RF_ERR_PROTOCOL;
                   goto WUPBEND;
			}
            // 第11字节返回帧大小和协议 高4位为帧大小，低4位为协议值
			PN512_RF_WorkInfo.FSC = PN512_gl_RF_ulFSList[(PN512_RF_WorkInfo.aucBuffer[10] >> 4) & 0x0F];

            // 低4位表示是否支持ISO14443协议，默认支持
            if((PN512_RF_WorkInfo.aucBuffer[10] & 0x0F) != 0x01)
            {
             	   PN512_RF_WorkInfo.usErrNo = 0x53; // received byte 11 err
                   ucRet = RET_RF_ERR_PROTOCOL;
				   goto WUPBEND;
            }

            // 第12字节包含FWI，CID和NAD等信息
            // 高4位返回FWI
            PN512_RF_WorkInfo.ucFWI = (PN512_RF_WorkInfo.aucBuffer[11] & 0x0F0) >> 4;

			uiFWITemp = (uint)0x01 << PN512_RF_WorkInfo.ucFWI;
			PN512_RF_WorkInfo.ulFWT = (ulong)(32 + 3) * uiFWITemp; //(256*16/Fc)*2^FWI + 3*2^FWI ETU

            // 第3~4位表示ADC，不予考虑
            // 第1~2位表示CID和NAD机制，不管PICC返回什么，均按不支持处理
            // 判断NAD
            if(PN512_RF_WorkInfo.aucBuffer[11] & 0x01)
            {
                 PN512_RF_WorkInfo.ucCIDFlag = 1;
            }
            else
            {
                 PN512_RF_WorkInfo.ucCIDFlag = 0;
            }
            // 判断CID
            if(PN512_RF_WorkInfo.aucBuffer[11] & 0x02)
            {
                 PN512_RF_WorkInfo.ucNADFlag = 1;
            }
            else
            {
                 PN512_RF_WorkInfo.ucNADFlag = 0;
            }
     }

WUPBEND:

//	 PN512_s_RFDelay10ETUs(RF_FDTMIN/10); // 延时500us再发送下一条命令
	 if(ucRet == 0)
	 {
		 if(c_para.card_buffer_w == 1 && c_para.card_buffer_val)
			 PN512_RF_WorkInfo.FSC = c_para.card_buffer_val;
	 }
     return ucRet;
}


uchar PN512_s_RF_ucAttrib(uchar* pucResp)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData = 0x00;

     PN512_s_vRFFlushFIFO();        //empty FIFO

     // RxCRC and TxCRC enable
     ucTempData = 0x83;
     PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
	 PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);

     PN512_s_RFSetTimer(PN512_RF_WorkInfo.ulFWT);  // 设置超时时限

	 PN512_RF_WorkInfo.aucBuffer[0] = PICC_ATTRIB; // Attrib命令码
     // 第2~5字节为PUPI
     PN512_RF_WorkInfo.aucBuffer[1] = PN512_RF_WorkInfo.ucUIDB[0];
     PN512_RF_WorkInfo.aucBuffer[2] = PN512_RF_WorkInfo.ucUIDB[1];
     PN512_RF_WorkInfo.aucBuffer[3] = PN512_RF_WorkInfo.ucUIDB[2];
     PN512_RF_WorkInfo.aucBuffer[4] = PN512_RF_WorkInfo.ucUIDB[3];
     // 第6字节为param1，定义TR0和TR1，及SOF、EOF
     // PAYPASS要求：采用默认TR0和TR1，支持SOF和EOF
     PN512_RF_WorkInfo.aucBuffer[5] = 0x00;
     // 第7字节为param2，定义BitRate和Frame Size
     // PAYPASS要求BitRate为106KBits，Frame Size = 256Bytes
     PN512_RF_WorkInfo.aucBuffer[6] = 0x08;
     // 第8字节为param3，定义PCD是否支持ISO14443-4
     // PAYPASS要求能够支持ISO14443-4
     PN512_RF_WorkInfo.aucBuffer[7] = 0x01;
     // 第9字节为param4，定义了CID
     // PAYPASS要求不支持CID
     PN512_RF_WorkInfo.aucBuffer[8] = 0x00;

     PN512_RF_WorkInfo.ulSendBytes = 9;    // how many bytes to send

     ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);

     if(ucRet == RET_RF_ERR_TIMEOUT)
     {
           // 超时无响应，直接返回
           return RET_RF_ERR_TIMEOUT;
     }
     else if(ucRet != RET_RF_OK)
     {
           // 其他情况按通信错误处理
           ucRet = RET_RF_ERR_TRANSMIT;
     }
     else
     {
           if(PN512_RF_WorkInfo.ulBytesReceived != 1)
           {
           		PN512_RF_WorkInfo.usErrNo = 0x95; // ACTB attrib err
                ucRet = RET_RF_ERR_TRANSMIT;
           }
           else
           {
			    if((PN512_RF_WorkInfo.aucBuffer[0] & 0x0F) != 0x00)
                {
                		PN512_RF_WorkInfo.usErrNo = 0x96; // ACTB attrib err
                       // CID必须为0
                       ucRet = RET_RF_ERR_PROTOCOL;
                }
           }

           *pucResp = PN512_RF_WorkInfo.aucBuffer[0];
     }

	if (ucRet == RET_RF_OK)
	{
		PN512_RF_WorkInfo.ucInProtocol = 1;
	}
	
//     PN512_s_RFDelay10ETUs(RF_FDTMIN/10); // 延时500us再发送下一条命令
     return ucRet;
}


uchar PN512_s_RF_ucHALTB(uchar* pucResp)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData = 0x00;

     PN512_s_vRFFlushFIFO();        // empty FIFO

     // RxCRC and TxCRC enable
     ucTempData = 0x83;
     PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
	 PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);

     PN512_s_RFSetTimer(PN512_RF_WorkInfo.ulFWT);  // 设置超时时限

     // 准备发送数据: 命令码＋PUPI
     PN512_RF_WorkInfo.ulSendBytes = 5;    // how many bytes to send
     PN512_RF_WorkInfo.aucBuffer[0] = PICC_HALT; // HALT命令码
     // 第2~5字节为PUPI
     PN512_RF_WorkInfo.aucBuffer[1] = PN512_RF_WorkInfo.ucUIDB[0];
     PN512_RF_WorkInfo.aucBuffer[2] = PN512_RF_WorkInfo.ucUIDB[1];
     PN512_RF_WorkInfo.aucBuffer[3] = PN512_RF_WorkInfo.ucUIDB[2];
     PN512_RF_WorkInfo.aucBuffer[4] = PN512_RF_WorkInfo.ucUIDB[3];

	 ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);

     if(ucRet == RET_RF_OK)
     {
         if(PN512_RF_WorkInfo.ulBytesReceived != 1)
		 {
               ucRet = RET_RF_ERR_TRANSMIT;
		 }
		 else if(PN512_RF_WorkInfo.aucBuffer[0] == 0x0)
         {
               *pucResp = PN512_RF_WorkInfo.aucBuffer[0];
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

//	 PN512_s_RFDelay10ETUs(RF_FDTMIN/10); // 延时500us再发送下一条命令
     return ucRet;
}


uchar PN512_s_RF_ucActTypeB(uchar* pucOutLen, uchar* pucOut)
{
      uchar ucRet = RET_RF_OK;
      uchar ucTempData = 0x00;
      uchar ucReSendNo = 0;

      // 先执行WUPB命令
      for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
      {
            ucRet = PN512_s_RF_ucWUPB(pucOutLen, pucOut);
            if(ucRet != RET_RF_ERR_TIMEOUT)
            {
                  break;
            }
      }

      if(ucRet != RET_RF_OK)
	  {
           PN512_RF_WorkInfo.usErrNo = 0x90; // ACTB WUPA err
		   return ucRet;
	  }

      // 比较ATQB参数与原先保存的ATQB参数
      if(memcmp(pucOut, (uchar*)PN512_RF_WorkInfo.ucATQB, 12))
	  {
	  	   PN512_RF_WorkInfo.usErrNo = 0x91; // ACTB ATQB err
           return RET_RF_ERR_PROTOCOL;
	  }

      // 接下来执行ATTRIB命令使TypeB卡进入ACTIVATE状态
      for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
      {
           ucRet = PN512_s_RF_ucAttrib(&ucTempData);
           if(ucRet != RET_RF_ERR_TIMEOUT)
           {
                 break;
           }
      }

      if(ucRet != RET_RF_OK)
	  {
	  	   PN512_RF_WorkInfo.usErrNo |= 0x400; // ACTB  err
		   return ucRet;
	  }

      PN512_RF_WorkInfo.ucMifActivateFlag = 1;
	  PN512_RF_WorkInfo.ucCurPCB = 1;

      return RET_RF_OK;
}


uchar PN512_s_RF_ucPOLL(uchar* pucMifType)
{
       uchar ucRet = RET_RF_OK;
       uchar ucTempData = 0x00;
       uchar ucATQA[2];  // 用于保存ATQA参数
       uchar ucATQB[12]; // 用于保存ATQB参数
	   uchar i = 0;
	   

       memset(ucATQA, 0x00, sizeof(ucATQA));
       memset(ucATQB, 0x00, sizeof(ucATQB));

       // 发送WUPA命令，判断感应区是否有TypeA卡
       ucRet = PN512_s_RF_ucWUPA(ucATQA);
	   if(ucRet == RET_RF_OK)
       {
            // 保存获取的ATQA参数
            PN512_RF_WorkInfo.ucATQA[0] = ucATQA[0];
            PN512_RF_WorkInfo.ucATQA[1] = ucATQA[1];

			// 获取UID信息，并判断是否有多卡存在
            ucRet = PN512_s_RF_ucGetUID();
			if(ucRet == RET_RF_OK)
            {
                    // 将该卡置为HALT态
                    PN512_s_RF_vHALTA();
					//del 延时操作放在载波方式选择中执行,需调试
					//PN512_s_RFDelay10ETUs(RF_TPDEALY/10); // 延时500ETU
                    // 接下来判断是否有TypeB卡存在
                    ucRet = PN512_s_RF_ucWUPB(&ucTempData,ucATQB);
					if(ucRet != RET_RF_ERR_TIMEOUT)
                    {                    
                        PN512_RF_WorkInfo.usErrNo = 0x41; // Type A + Type B exist err
                        // 卡有响应，说明感应区内除了TypeA外，还有TypeB卡
                        // 应该返回多卡冲突
						return RET_RF_ERR_MULTI_CARD;
                    }

                    // 否则，说明感应区内只有一张TypeA卡
//                    PN512_s_RFDelay10ETUs(RF_TPDEALY/10); // 延时500ETU
                    
					//根据SAK数据细分卡片类型
					PN512_s_RF_ucCardTypeCheck(pucMifType);
					PN512_RF_WorkInfo.ucMifCardType = *pucMifType;
					PN512_RF_WorkInfo.ucCurType = RF_TYPEA;
					
                    return RET_RF_OK;
            }
            else
            {
//		        PN512_s_RFResetCarrier();
				return ucRet;
            }
       }
       else if(ucRet == RET_RF_ERR_TIMEOUT)
       {
            // 感应区没有TypeA卡响应，说明感应区内没有TypeA卡存在
            // 接下来应该判断是否TypeB卡存在

			//del by liuxl 20071223 延时操作放在载波方式选择中执行,需调试
            //PN512_s_RFDelay10ETUs(10);//RF_TPDEALY/10); // 延时500ETU

            // 接下来判断是否有TypeB卡存在
            ucRet = PN512_s_RF_ucWUPB(&ucTempData,ucATQB);
			if(ucRet == RET_RF_OK)
            {
                   // 保存ATQB参数
                   memcpy((uchar*)PN512_RF_WorkInfo.ucATQB, ucATQB, ucTempData);
                   // 接下来将TypeB卡置为HALT状态
				   for(i = 0; i < 3; i++)
				   {
                        ucRet = PN512_s_RF_ucHALTB(&ucTempData);
						if(ucRet != RET_RF_ERR_TIMEOUT)
						{
							break;
						}
				   }

                   if(ucRet == RET_RF_OK)
                   {
						//del 延时操作放在载波方式选择中执行,需调试
						//PN512_s_RFDelay10ETUs(RF_TPDEALY/10); // 延时500ETU
                        ucRet = PN512_s_RF_ucWUPA(ucATQA);
                        if(ucRet != RET_RF_ERR_TIMEOUT)
                        {                        
             	            PN512_RF_WorkInfo.usErrNo = 0x60; // Type B & Type A exist err
                            // 有TypeA卡响应，则说明存在TypeA，多卡冲突
							return RET_RF_ERR_MULTI_CARD;
                        }

 //                       PN512_s_RFDelay10ETUs(RF_TPDEALY/10); // 延时500ETU
                        // 感应区内只有一张TypeB卡
                        PN512_RF_WorkInfo.ucMifCardType = RF_TYPE_B;
						*pucMifType = PN512_RF_WorkInfo.ucMifCardType;
						PN512_RF_WorkInfo.ucCurType = RF_TYPEB;// 'B';
                        return RET_RF_OK;
                   }
                   else
				   {
                        //将复位PICC操作放至出现传输、协议、超时错误时执行
//		                PN512_s_RFResetCarrier();
                        return ucRet;
				   }
            }
            else if(ucRet == RET_RF_ERR_TIMEOUT)
            {
                 //del延时操作放在载波方式选择中执行,需调试
				//PN512_s_RFDelay10ETUs(RF_TPDEALY/10); // 延时500ETU
            }
            else
            {
                // 将复位PICC操作放至出现传输、协议、超时错误时执行
//		        PN512_s_RFResetCarrier();
            }
			return ucRet;
       }
       else
       {
            // 将复位PICC操作放至出现传输、协议、超时错误时执行
//		    PN512_s_RFResetCarrier();
       }
	   // 其他错误情况，直接返回
	   return ucRet;

}


uchar  PN512_s_RF_ucIfCardExit(void)
{
       uchar ucRet = RET_RF_OK;
       uchar ucTempData[12];
       uchar ucTemp = 0x00;
       uchar ucReSendNo = 0;

       if(PN512_RF_WorkInfo.ucCurType == RF_TYPEA)
       { 
            // 先发送WUPA寻卡，如果有卡响应，则说明卡未离开感应区
            for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
            {
                  ucRet = PN512_s_RF_ucWUPA(ucTempData);
                  if(ucRet != RET_RF_ERR_TIMEOUT)
                  { 
                       break;
                  }

                  if((ucRet == RET_RF_ERR_TIMEOUT) && (ucReSendNo == 2))
                  { 
                        // 无卡响应，则说明卡已离开感应区
                        return RET_RF_OK;
                  }
            }
            // 卡有响应，说明卡未离开感应区
            // 先将卡片置为HALT态
            PN512_s_RF_vHALTA(); 
            return RET_RF_ERR_CARD_EXIST;
       }
       else
       {
            // 先发送WUPB寻卡，如果有卡响应，则说明卡未离开感应区
            for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
            { 
                  ucRet = PN512_s_RF_ucWUPB(&ucTemp, ucTempData);
				  if(ucRet != RET_RF_ERR_TIMEOUT)
                  {
                       break;
                  }
                  if((ucRet == RET_RF_ERR_TIMEOUT) && (ucReSendNo == 2))
                  {
                        // 无卡响应，则说明卡已离开感应区
                        return RET_RF_OK;
                  }
            }
           // 卡有响应，说明卡未离开感应区
           // 先将卡片置为HALT态
//           for(ucReSendNo = 0; ucReSendNo < 3; ucReSendNo++)
           {
                 ucRet = PN512_s_RF_ucHALTB(&ucTemp);
                 if(ucRet != RET_RF_ERR_TIMEOUT)
                 {          
					 return RET_RF_ERR_CARD_EXIST;
					 // break;
                 }				 
                 else //if((ucRet == RET_RF_ERR_TIMEOUT) && (ucReSendNo == 2))  
                 {
                        // 无卡响应，则说明卡已离开感应区
                        return RET_RF_OK;
                 }
           }
       }

	   //return RET_RF_ERR_CARD_EXIST;
}


uchar PN512_s_RF_ucProcess(uint uiSendLen, uint* puiRecLen)
{
       uchar ucRet = RET_RF_OK;
       uchar ucTempData = 0x00;

       // 设置定时器
       PN512_s_RFSetTimer(PN512_RF_WorkInfo.ulFWT);

       if(PN512_RF_WorkInfo.ucCurType == RF_TYPEA)
       {
            ucTempData = 0x80;
            PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
	        PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);
       }
       else
       {
            ucTempData = 0x83;
            PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
	        PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);
       }

        // 准备发送数据: 命令码
        PN512_RF_WorkInfo.ulSendBytes = uiSendLen;    // how many bytes to send
        ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);

        if(ucRet == RET_RF_ERR_TIMEOUT)
		{
			ucRet = RET_RF_ERR_TIMEOUT;
		}
		else if(ucRet != RET_RF_OK)
        {
			ucRet = RET_RF_ERR_TRANSMIT;
        }

        if(ucRet == RET_RF_OK)
        {
		    *puiRecLen = PN512_RF_WorkInfo.ulBytesReceived;
		    if(PN512_RF_WorkInfo.ulBytesReceived == 0)
			{
				// 当接收正确，但是实际接收长度为0时，按传输错误处理
				ucRet = RET_RF_ERR_TRANSMIT;
			}
            // 对于TypeB卡，当PICC早于TR0Min + TR1Min开始回送数据的话
            // 根据PAYPASS要求，可以认定为传输错误，也可以正常接收，
            // 在本程序里，不判断这种情况。
        }

		PN512_s_RFDelay10ETUs(6);//PN512_s_RFDelay10ETUs(RF_FDTMIN/10);  // 命令之间延时500us，以保证最小正向保护时间
        return( ucRet);
}


uchar PN512_s_RF_ucDeselect(void)
{
      uchar ucRet = RET_RF_OK;
      uchar ucTempData = 0x00;
      uchar i = 0;

      if(PN512_RF_WorkInfo.ucCurType == RF_TYPEA)
      {
            ucTempData = 0x80;
            PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
	        PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);
      }
      else
      {
            ucTempData = 0x83;
            PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
	        PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);
      }

      while(1)
      {
          // 设置定时器
          PN512_s_RFSetTimer(RF_FWTDESELECT);

          PN512_RF_WorkInfo.aucBuffer[0] = 0xC2; // 1100 0010

          // 准备发送数据: 命令码
          PN512_RF_WorkInfo.ulSendBytes = 1;   // how many bytes to send
          ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);


          if((ucRet != RET_RF_OK) && (ucRet != RET_RF_ERR_TIMEOUT))
          {
                 ucRet = RET_RF_ERR_TRANSMIT;
          }

          if(ucRet == RET_RF_ERR_TIMEOUT)
		  {
			  ucRet = RET_RF_ERR_TIMEOUT;
		  }

		  if((ucRet == RET_RF_ERR_TRANSMIT) || (ucRet == RET_RF_ERR_TIMEOUT))
          {
			    i++;
                if(i > 2)
                {
                       break;
                }
                // 重发DESELECT命令
                continue;
          }

          if(ucRet == RET_RF_OK)
          {
                if(PN512_RF_WorkInfo.ulBytesReceived != 1)
                {
                       ucRet = RET_RF_ERR_TRANSMIT;
                }
				else if(PN512_RF_WorkInfo.aucBuffer[0] != 0xC2)
				{
					   ucRet = RET_RF_ERR_PROTOCOL;
				}
                break;
          }
      }

      // 将复位PICC操作放至出现传输、协议、超时错误时执行
      if(ucRet != RET_RF_OK)
      {
          PN512_s_RFResetCarrier();
      }

      PN512_RF_WorkInfo.ucMifActivateFlag = 0;
      PN512_RF_WorkInfo.ucHalt = 1;

	  return  ucRet;
}


uchar  PN512_s_RF_ucMifRBlock(uint* puiRecLen)
{
       uchar result = RET_RF_OK;       
	   uchar ucReSendNo = 0x00;
	   
	   // 最多发送NAK两次
       for(ucReSendNo = 0; ucReSendNo < 2; ucReSendNo++)
       {
             // 发送NAK
             PN512_RF_WorkInfo.aucBuffer[0] = PN512_RF_WorkInfo.ucCurPCB | 0xB2;
             result = PN512_s_RF_ucProcess(1, puiRecLen);
             if((result != RET_RF_ERR_TIMEOUT) && (result != RET_RF_ERR_TRANSMIT))
             {              
                    // 如果出现超时或者传输错误，则应重发NAK
                    break;
             }
       }
       return result;      
}


uchar PN512_s_RF_ucMifSBlock( uint* puiRecLen)
{
    uchar ucTempData = 0;
    uchar ucRet = RET_RF_OK;	
    ulong ulTemp1, ulTemp2 = 0;

	if ((PN512_RF_WorkInfo.aucBuffer[0] & 0x30) != 0x30)
	{
		// 不是WTX，错误
		return RET_RF_ERR_PROTOCOL;
	}
	if ((PN512_RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08) 
	{
		// PICC不应带CID字节
		return RET_RF_ERR_PROTOCOL;
	}
	else
	{ 
		ucTempData = PN512_RF_WorkInfo.aucBuffer[1];
		if((ucTempData & 0xC0) != 0x00)
		{
			// 不支持Power Level
			return RET_RF_ERR_PROTOCOL;
		}
		// 获取WTXM整数
		PN512_RF_WorkInfo.ucWTX = ucTempData & 0x3F;
		if(PN512_RF_WorkInfo.ucWTX == 0x00)
		{
			return RET_RF_ERR_PROTOCOL;
		}
		if(PN512_RF_WorkInfo.ucWTX  > 59)
		{
			PN512_RF_WorkInfo.ucWTX = 59;
		}
    
		// 计算临时FWT时间
		ulTemp1 = PN512_RF_WorkInfo.ucWTX;
		ulTemp2 = PN512_RF_WorkInfo.ulFWT;
		//PN512_RF_WorkInfo.ulFWTTemp = PN512_RF_WorkInfo.ucWTX * PN512_RF_WorkInfo.ulFWT;
		PN512_RF_WorkInfo.ulFWTTemp = ulTemp1 * ulTemp2;
	}

	PN512_RF_WorkInfo.aucBuffer[0] = 0xf2;  
	PN512_RF_WorkInfo.aucBuffer[1] = ucTempData  & 0x3F;        

//	if(PN512_RF_WorkInfo.ulFWTTemp > MAXFWT)
//	{
//		 // 设置定时器,最大不超过FWT_MAX
//		 return RET_RF_ERR_PROTOCOL;
//	}
//	else
//	{
		 PN512_s_RFSetTimer(PN512_RF_WorkInfo.ulFWTTemp);
//  }
    if(PN512_RF_WorkInfo.ucCurType == RF_TYPEA)
    {
         ucTempData = 0x80;// TxCRC、RxCRC、Parity Enable
         PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
	     PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);
    }
    else
    {
         ucTempData = 0x83;// TxCRC、RxCRC Enable
         PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
	     PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);
    }
	   
	// 准备发送数据: 命令码
	PN512_RF_WorkInfo.ulSendBytes = 2;     // how many bytes to send
        
	ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);
	
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
			*puiRecLen = PN512_RF_WorkInfo.ulBytesReceived;
			if(PN512_RF_WorkInfo.ulBytesReceived == 0)
			{
				// 当接收正确但是返回实际接收长度为0时，按传输错误处理				
				ucRet = RET_RF_ERR_TRANSMIT;
			}
			
	}

	//PN512_s_RFDelay10ETUs(RF_FDTMIN/10);// 延时500us再发送下一条命令
	//for debug 20071224 should be del
	//PN512_s_RFDelay10ETUs(RF_TPDEALY/10);
    
    return( ucRet); 
}


uchar PN512_s_RF_ucExchange(uint uiSendLen, uchar* pucInData, uint* puiRecLen, uchar* pucOutData)
{	  
    uint  uiSendLeftLen = 0, uiTempLen = 0, uiSendCurrPos = 0,uiCurrRecvLen = 0;
	uchar ucRet = RET_RF_OK;
	uchar tempResult = 0x00;
	uchar i = 0;
    uchar ucResendNo = 0x00;   
	ulong ulTemp = 0;

	*puiRecLen = 0; 

	// 保存要发送的数据长度
	uiSendLeftLen =	uiSendLen;	  
	// 最大发送帧长度包括两个CRC字节和PCB字节
	// PAYPASS要求不包含CID字节和NAD字节
	while(uiSendLeftLen	> (PN512_RF_WorkInfo.FSC - 3))
	{		   
		 uiTempLen = PN512_RF_WorkInfo.FSC - 3;
		 PN512_RF_WorkInfo.ucCurPCB = (PN512_RF_WorkInfo.ucCurPCB + 1) & 0x01;
		 PN512_RF_WorkInfo.aucBuffer[0] = PN512_RF_WorkInfo.ucCurPCB | 0x12;		   
		 memcpy((uchar*)(&PN512_RF_WorkInfo.aucBuffer[1]), pucInData + uiSendCurrPos,	uiTempLen);
		 ucRet = PN512_s_RF_ucProcess(uiTempLen+1, &uiCurrRecvLen);
		 
		 //	如果超时没有接收到数据,发送NAK要求PICC回送数据
		 //	如果接收过程中出现错误，也发送NAK要求PICC回送数据
		 if((ucRet	== RET_RF_ERR_TIMEOUT) ||	(ucRet	== RET_RF_ERR_TRANSMIT))
		 {
			    tempResult = ucRet;
			    ucRet = PN512_s_RF_ucMifRBlock(&uiCurrRecvLen);
				if(ucRet != RET_RF_OK)
				{				
					// 仍然没有接收到有效响应数据，则通知设备
					 return tempResult;
				}
		 }		 
		 
		 if(ucRet != RET_RF_OK)
		 {
			 return ucRet;
		 }

		 if(uiCurrRecvLen >	(PN512_RF_WorkInfo.FSD - 2))
		 {
				// 接收长度超出了FSD
				return RET_RF_ERR_PROTOCOL;
		 }
		 
		 if (uiCurrRecvLen == 0)
		 { 
			    // 数据交换失败
				return RET_RF_ERR_TRANSMIT;
		 }

		 if((PN512_RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
		 {
				// 接收到I_Block, 则按协议错误处理
				return RET_RF_ERR_PROTOCOL;
		 }
		 else if((PN512_RF_WorkInfo.aucBuffer[0] & 0xE2) == 0xA2)
		 {
				// 接收到R_Block
				if((PN512_RF_WorkInfo.aucBuffer[0] & 0x10) == 0x10)
				{
					 // PICC不应返回NAK
					 return RET_RF_ERR_PROTOCOL;
				}
				
				if((PN512_RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
				{
					 // PICC不应返回CID
					 return	RET_RF_ERR_PROTOCOL;
				}
				ulTemp = PN512_RF_WorkInfo.ucCurPCB;
				//if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) == PN512_RF_WorkInfo.ucCurPCB)
				if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) == ulTemp)
				{
					 // 收到R_block(ACK)应答块块号正确, 准备发送下一个信息块
				     uiSendCurrPos += uiTempLen;
				     uiSendLeftLen -= uiTempLen;
				   
				     // 重发计数清零
				     ucResendNo = 0;
				}
				else
			    {   
				     if(ucResendNo++ > 3)
				     {
						 //	PCD重发一个I_Block超过三次
						   return RET_RF_ERR_PROTOCOL;
				     }
				     // 如果块号不正确, 则重发
				     PN512_RF_WorkInfo.ucCurPCB	= (PN512_RF_WorkInfo.ucCurPCB + 1) & 0x01;				  
			   }
		 }
		 else if((PN512_RF_WorkInfo.aucBuffer[0] & 0xC7) == 0xC2)
		 {
				// 接收到S_Block
				// 发送S_Block
				while(1)
				{
						ucRet = PN512_s_RF_ucMifSBlock(&uiCurrRecvLen);
						if((ucRet == RET_RF_ERR_TIMEOUT)	|| (ucRet == RET_RF_ERR_TRANSMIT))
						{
							tempResult = ucRet;
							// 出现无应答或者通信错误，则发送NAK
							ucRet = PN512_s_RF_ucMifRBlock(&uiCurrRecvLen);
							if(ucRet != RET_RF_OK)
							{
								   // 仍然没有接收到有效响应数据，则通知设备
								   return tempResult;
							}
						}						
						
						if(ucRet != RET_RF_OK)
						{
							 return ucRet;
						}

						if(uiCurrRecvLen > (PN512_RF_WorkInfo.FSD - 2))
						{
								// 接收长度超出了FSD
								return RET_RF_ERR_PROTOCOL;
						}

						if (uiCurrRecvLen == 0)
						{  
							    // 数据交换失败
								return RET_RF_ERR_TRANSMIT;
						}
										
						if((PN512_RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
						{
								// 接收到I_Block, 则按协议错误处理
								return RET_RF_ERR_PROTOCOL;
						}
						else if((PN512_RF_WorkInfo.aucBuffer[0]	& 0xE2)	== 0xA2)
						{						
								// 接收到R_Block
								if((PN512_RF_WorkInfo.aucBuffer[0] & 0x10) == 0x10)
								{
									   // PICC不应返回NAK
									   return RET_RF_ERR_PROTOCOL;
								}
								
								if((PN512_RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
								{
									   // PICC不应返回CID
									   return	RET_RF_ERR_PROTOCOL;
								}
								ulTemp = PN512_RF_WorkInfo.ucCurPCB;
								//if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) == PN512_RF_WorkInfo.ucCurPCB)
                                if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) == ulTemp)
								{
								   // 收到R_block(ACK)应答块块号正确, 准备发送下一个信息块
								   uiSendCurrPos +=	uiTempLen;
								   uiSendLeftLen -=	uiTempLen;
								   
								   // 重发计数清零
								   ucResendNo =	0;
								}
								else
								{	 
									 // 如果块号不正确, 重发上个I_BLOCk						  
									 if(ucResendNo++ > 3)
									 {
										   // PCD重发一个I_Block超过三次
										   return RET_RF_ERR_PROTOCOL;
									 }
									 // 如果块号不正确, 则重发
									 PN512_RF_WorkInfo.ucCurPCB	= (PN512_RF_WorkInfo.ucCurPCB + 1) &	0x01;						  
								}
								break;
						 }
						 else if((PN512_RF_WorkInfo.aucBuffer[0] & 0xC7) != 0xC2)
						 {
                               // 接收到其他不明Block
							   return	RET_RF_ERR_PROTOCOL;
						 }
				}                
		 }
		 else
		 {
				// PCB值不正确						  
				return RET_RF_ERR_PROTOCOL;
		 }
	}
	
	ucResendNo = 0; // 重发次数清零
	// 发送最后一帧数据
	while(1)
	{		   
		 PN512_RF_WorkInfo.ucCurPCB = (PN512_RF_WorkInfo.ucCurPCB + 1) & 0x01;
		   
		 memcpy((uchar*)(&PN512_RF_WorkInfo.aucBuffer[1]), pucInData + uiSendCurrPos, uiSendLeftLen);
		 PN512_RF_WorkInfo.aucBuffer[0] = PN512_RF_WorkInfo.ucCurPCB | 0x02;	// PCB附值,当前无NAD和CID		   
		
		 ucRet	= PN512_s_RF_ucProcess(uiSendLeftLen+1, &uiCurrRecvLen); 		
 		 //	如果超时没有接收到数据,发送NAK要求PICC回送数据
		 //	如果接收过程中出现错误，也发送NAK要求PICC回送数据
		 if((ucRet	== RET_RF_ERR_TIMEOUT) || (ucRet	== RET_RF_ERR_TRANSMIT))
		 {	   
			    tempResult = ucRet;
				/*/for debug 070915 for display
				ScrPrint(0,2,1,"S RBlock: 01:%02X ", result);
				getkey();
				// debug end*/
				
			    ucRet = PN512_s_RF_ucMifRBlock(&uiCurrRecvLen);
				
				/*for debug 070915 for display
				ScrPrint(0,2,1,"S RBlock: 02:%02X ", result);
				getkey();
				debug end*/
				if(ucRet != RET_RF_OK)
				{ 
					   // 仍然没有接收到有效响应数据，则通知设备
					   return tempResult;
				}
		 }
		 
		 if(ucRet != RET_RF_OK)
		 {	 
			 return ucRet;
		 }

		 if(uiCurrRecvLen >	(PN512_RF_WorkInfo.FSD - 2))
		 {
				// 接收长度超出了FSD
				return RET_RF_ERR_PROTOCOL;
		 }

		 if (uiCurrRecvLen == 0)
		 { 	  
			    // 数据交换失败			 
			 return RET_RF_ERR_TRANSMIT;
		 }
		 
		 if((PN512_RF_WorkInfo.aucBuffer[0] & 0xE2) == 0xA2)
		 {						
				// 接收到R_Block
				if((PN512_RF_WorkInfo.aucBuffer[0] & 0x10) == 0x10)
				{
					   // PICC不应返回NAK
					   return	RET_RF_ERR_PROTOCOL;
				}
				
				if((PN512_RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
				{
					   // PICC不应返回CID
					   return	RET_RF_ERR_PROTOCOL;
				}
				ulTemp = PN512_RF_WorkInfo.ucCurPCB;
				//if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) == PN512_RF_WorkInfo.ucCurPCB)
                if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) == ulTemp)
				{
					   // PICC回送错误块号
					   return	RET_RF_ERR_PROTOCOL;
				}
				else
				{
					   if(ucResendNo++ > 3)
				       {
						   // PCD重发一个I_Block超过三次
						   return	RET_RF_ERR_PROTOCOL;
				       }
					   // 请求重发
					   PN512_RF_WorkInfo.ucCurPCB = (PN512_RF_WorkInfo.ucCurPCB + 1) & 0x01; 
				}
		 }
		 else if((PN512_RF_WorkInfo.aucBuffer[0] & 0xC7) == 0xC2)
		 {
				// 接收到S_Block
				// 发送S_Block
				while(1)
				{	
					ucRet = PN512_s_RF_ucMifSBlock(&uiCurrRecvLen);
					if((ucRet == RET_RF_ERR_TIMEOUT)	|| (ucRet == RET_RF_ERR_TRANSMIT))
					{
					
						/*for debug  for display
						ScrPrint(0,2,1,"S SBlock: 01:%02X ", result);
						getkey();
						debug end*/
						tempResult = ucRet;
						// 出现无应答或者通信错误，则发送NAK
						ucRet = PN512_s_RF_ucMifRBlock(&uiCurrRecvLen);
				
						/*for debug  for display
						ScrPrint(0,2,1,"S SBlock: 02:%02X ", result);
						getkey();
						debug end*/
						
						if(ucRet != RET_RF_OK)
						{
							   // 仍然没有接收到有效响应数据，则通知设备
							   return tempResult;
						}
					}
					
					if(ucRet != RET_RF_OK)
					{
						 return ucRet;
					}

					if (uiCurrRecvLen == 0)
					{ 
						    // 数据交换失败
							return RET_RF_ERR_TRANSMIT;
					}

					if(uiCurrRecvLen > (PN512_RF_WorkInfo.FSD - 2))
					{
						  // 接收长度超出了FSD
						  return RET_RF_ERR_PROTOCOL;
					}
					  
					if((PN512_RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
					{	
						// 接收到I_Block
						break;
					}
					else if((PN512_RF_WorkInfo.aucBuffer[0]	& 0xE2)	== 0xA2)
					{
						// 接收到R_Block
						if((PN512_RF_WorkInfo.aucBuffer[0] & 0x10) == 0x10)
						{
							   // PICC不应返回NAK
							   return	RET_RF_ERR_PROTOCOL;
						}
						
						if((PN512_RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
						{
							   // PICC不应返回CID
							   return	RET_RF_ERR_PROTOCOL;
						}
						ulTemp = PN512_RF_WorkInfo.ucCurPCB;
				        //if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) == PN512_RF_WorkInfo.ucCurPCB)
                        if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) == ulTemp)
						{
							   // PICC回送错误块号
							   return	RET_RF_ERR_PROTOCOL;
						}
						else
						{
							   if(ucResendNo++ > 3)
							   {
								   // PCD重发一个I_Block超过三次
								   return	RET_RF_ERR_PROTOCOL;
							   }
							   // 请求重发
							   PN512_RF_WorkInfo.ucCurPCB = (PN512_RF_WorkInfo.ucCurPCB +	1) & 0x01; 
						}
						break;
					}
					else if((PN512_RF_WorkInfo.aucBuffer[0] & 0xC7) != 0xC2)
					{
                        // 接收到其他不明Block
						return RET_RF_ERR_PROTOCOL;
					}
				}

				if((PN512_RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
				{	
					// 接收到I_Block
					break;
				}
		 }
		 else if((PN512_RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
		 {
		 	/*for debug  
		 	ScrPrint(0,6,0,"Rec I Block:%02X",uiCurrRecvLen);
			getkey();
			debug end*/
			
				// 接收到I_Block
				break;
		 }
		 else
		 {
				// 接收到其他不支持的PCB
				return RET_RF_ERR_PROTOCOL;
		 }		 
	}
	
 	/*for debug  
 	ScrPrint(0,6,0,"Rec I Block out");
	getkey();
	debug end*/
	// 接收PICC回送的I_Block
	while(1)
	{
		 	/*for debug liuxl 
		 	ScrPrint(0,6,0,"Save In %02X %02X %02X",PN512_RF_WorkInfo.aucBuffer[0],PN512_RF_WorkInfo.ucCurPCB,uiCurrRecvLen);
			getkey();
			debug end*/
			ulTemp = PN512_RF_WorkInfo.ucCurPCB;
		   //if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) != PN512_RF_WorkInfo.ucCurPCB)
            if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) != ulTemp)
		   {
				 // PICC回送错误块号
				 return	RET_RF_ERR_PROTOCOL;
		   }
		   
		   if((PN512_RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
		   {
				  // PICC不应返回CID
				  return RET_RF_ERR_PROTOCOL;
		   }
		   // 长度越界判断， 
		   if(uiCurrRecvLen < 1)
		   {
			   return RET_RF_ERR_TRANSMIT;
		   }
		   //	保存接收到的I_Block数据
		   memcpy(&pucOutData[*puiRecLen], (uchar*)(&PN512_RF_WorkInfo.aucBuffer[1]), uiCurrRecvLen-1);
		   *puiRecLen += uiCurrRecvLen - 1;			 
		   			
		   if((PN512_RF_WorkInfo.aucBuffer[0] & 0x10) == 0x00)
		   {
				 //	接收到最后一个I_Block，则返回
				 break;
		   }
		   // 翻转Block号
		   PN512_RF_WorkInfo.ucCurPCB = (PN512_RF_WorkInfo.ucCurPCB + 1) & 0x01;
		   // PICC发送链接I_Block
		   i = 0;
		   while(1)
		   {
				// 发送ACK获取其他I_Block
				PN512_RF_WorkInfo.aucBuffer[0] = PN512_RF_WorkInfo.ucCurPCB | 0xA2; //	PCB附值,当前无NAD和CID
			    ucRet = PN512_s_RF_ucProcess(1,&uiCurrRecvLen);
				
				if((ucRet == RET_RF_ERR_TIMEOUT) || (ucRet == RET_RF_ERR_TRANSMIT))
				{
						/*for debug  for display
						ScrPrint(0,2,1,"R IBlock: 01:%d %02X ",i, result);
						getkey();
						debug en*/
					   i++;
					   if(i	== 3)
					   {
						    // ACK块最多连续发送三次
							return ucRet;
					   }
					   // 没有响应或者通信错误，则重发ACK
					   continue;
				}				
				
				if(ucRet != RET_RF_OK)
				{
					 return ucRet;
				}

				if (uiCurrRecvLen == 0)
				{  
					    // 数据交换失败
						return RET_RF_ERR_TRANSMIT;
				}

				if(uiCurrRecvLen > (PN512_RF_WorkInfo.FSD - 2))
				{
					  // 接收长度超出了FSD
					  return RET_RF_ERR_PROTOCOL;
				}
				  
				if((PN512_RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
				{
					// I_Block
					break;  
				}
				else if((PN512_RF_WorkInfo.aucBuffer[0] & 0xE2) == 0xA2)
				{	   
					   // 接收到R_Block
					   if((PN512_RF_WorkInfo.aucBuffer[0] & 0x10) == 0x10)
					   {
							// PICC不应返回NAK
							return RET_RF_ERR_PROTOCOL;
					   }
						
					   if((PN512_RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
					   {
							// PICC不应返回CID
							return RET_RF_ERR_PROTOCOL;
					   }

					    ulTemp = PN512_RF_WorkInfo.ucCurPCB;
					   //if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) != PN512_RF_WorkInfo.ucCurPCB)
					   if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) != ulTemp)
                       {
							// PICC回送错误块号
							return RET_RF_ERR_PROTOCOL;
					   }

					   i++;
					   if(i	== 3)
					   {
						   // ACK块最多连续发送三次
						   return RET_RF_ERR_PROTOCOL;
					   }					   
					   continue;
				}
				else if((PN512_RF_WorkInfo.aucBuffer[0] & 0xC7) == 0xC2)
				{
				    // 接收到S_Block
					// 发送S_Block
					while(1)
					{
						ucRet = PN512_s_RF_ucMifSBlock(&uiCurrRecvLen);
						if((ucRet == RET_RF_ERR_TIMEOUT)	|| (ucRet == RET_RF_ERR_TRANSMIT))
						{
					
						/*for debug 070915 for display
						ScrPrint(0,2,1,"R SBlock: 01:%02X ", result);
						getkey();
						debug end*/
							tempResult = ucRet;
							// 出现无应答或者通信错误，则发送NAK
							ucRet = PN512_s_RF_ucMifRBlock(&uiCurrRecvLen);
					
						/*for debug 070915 for display
						ScrPrint(0,2,1,"R SBlock: 02:%02X ", result);
						getkey();
						debug en*/
							if(ucRet != RET_RF_OK)
							{
								   // 仍然没有接收到有效响应数据，则通知设备
								   return tempResult;
							}
						}
						
						if(ucRet != RET_RF_OK)
						{
							 return ucRet;
						}

						if (uiCurrRecvLen == 0)
						{ 
							    // 数据交换失败
								return RET_RF_ERR_TRANSMIT;
						}
						
						if(uiCurrRecvLen > (PN512_RF_WorkInfo.FSD - 2))
						{
							  // 接收长度超出了FSD
							  return RET_RF_ERR_PROTOCOL;
						}
						  
						if((PN512_RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
						{	
							// 接收到I_Block
							break;
						}
						else if((PN512_RF_WorkInfo.aucBuffer[0]	& 0xE2)	== 0xA2)
						{
							// 接收到R_Block
							if((PN512_RF_WorkInfo.aucBuffer[0] & 0x10) == 0x10)
							{
								   // PICC不应返回NAK
								   return	RET_RF_ERR_PROTOCOL;
							}
							
							if((PN512_RF_WorkInfo.aucBuffer[0] & 0x08) == 0x08)
							{
								   // PICC不应返回CID
								   return	RET_RF_ERR_PROTOCOL;
							}

                            ulTemp = PN512_RF_WorkInfo.ucCurPCB;
							//if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) != PN512_RF_WorkInfo.ucCurPCB)
							if((PN512_RF_WorkInfo.aucBuffer[0] & 0x01) != ulTemp)
							{
								   // PICC回送错误块号
								   return	RET_RF_ERR_PROTOCOL;
							}
							else
							{
								   if(i++ > 3)
								   {
									   // PCD重发一个I_Block超过三次
									   return	RET_RF_ERR_PROTOCOL;
								   }
							}
							break;
						}
						else if((PN512_RF_WorkInfo.aucBuffer[0] & 0xC7) != 0xC2)
						{
							// 接收到其他不明Block
							return RET_RF_ERR_PROTOCOL;
						}
					}

					if((PN512_RF_WorkInfo.aucBuffer[0] & 0xE2) == 0x02)
					{	
						// 接收到I_Block
						break;
					}
				}
				else
				{
                     // 接收到其他不明Block
					 return RET_RF_ERR_PROTOCOL;
				}
		   }
	       // 接收到I_Block, 则继续进行接收
		   i = 0; 
	}
	
	// 最终处理
	if ((*puiRecLen > 1) && (ucRet == RET_RF_OK))
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

void PN512_s_RF_vGetDetectAInfo(uchar *SerialInfo, uchar *Other)
{
	switch(PN512_RF_WorkInfo.ucUIDLen)
	{
	    case 4:
			memcpy(SerialInfo+1, (uchar*)PN512_RF_WorkInfo.ucUIDCL1, 4);
			break;
		case 7:
			//modify by skx
			memcpy(SerialInfo+1, (uchar*)(PN512_RF_WorkInfo.ucUIDCL1+1), 3);
			memcpy(SerialInfo+4, (uchar*)(&PN512_RF_WorkInfo.ucUIDCL2), 4);
			break;
		case 10:
			memcpy(SerialInfo+1, (uchar*)(PN512_RF_WorkInfo.ucUIDCL1+1), 3);
			memcpy(SerialInfo+4, (uchar*)(&PN512_RF_WorkInfo.ucUIDCL2+1), 3);
			memcpy(SerialInfo+7, (uchar*)(&PN512_RF_WorkInfo.ucUIDCL3), 4);
			break;
	}

	//对于A型卡，返回len+ errcode[2] +ATQA[2] + SAK1/SAK2/SAK3
	// mifare one 卡无ATS数据，ATS长度为0；
	memcpy(Other+3, (uchar*)PN512_RF_WorkInfo.ucATQA, 2);
	switch(PN512_RF_WorkInfo.ucUIDLen)
	{
	   case 4:				   	  
	   	  Other[0] = 5;
		  Other[5] = PN512_RF_WorkInfo.ucSAK1;
		  break;
	   case 7:	   	  
	   	  Other[0] = 6;
		  Other[5] = PN512_RF_WorkInfo.ucSAK1;
		  Other[6] = PN512_RF_WorkInfo.ucSAK2;
		  break;
	   case 10:	   	  
	   	  Other[0] = 7;
		  Other[5] = PN512_RF_WorkInfo.ucSAK1;
		  Other[6] = PN512_RF_WorkInfo.ucSAK2;
		  Other[7] = PN512_RF_WorkInfo.ucSAK3;
		  break;
	}	
	
	return;
}



void PN512_s_RFDelay10ETUs(ulong nETU)
{
    ulong  TmpETU = 0;
	uchar ucTemp = 0;
	ushort usTimeValue = 0;

	TmpETU = nETU;

    PN512_s_vRFSetBitMask(PN512_CONTROL_REG, 0x80); // 关闭定时器
    ucTemp = 0;
    // TPrescale = 0x00
    // 定时器频率为6.78MHz
	PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
	PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

    while(1)
    {
        // Treload = 0x500, 约为20ETU
		ucTemp = 0x05;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = 0x00;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);

		PN512_s_vRFSetBitMask(PN512_CONTROL_REG, 0x40);  // 启动定时器

		while(1)
		{
		   PN512_s_vRFReadReg(1,PN512_TCOUNTERVAL_H_REG,&ucTemp);
		   usTimeValue = ucTemp << 8;
		   PN512_s_vRFReadReg(1,PN512_TCOUNTERVAL_L_REG,&ucTemp);
		   usTimeValue |= ucTemp;

		   if(usTimeValue <= 0x280)
		   {
		        PN512_s_vRFSetBitMask(PN512_CONTROL_REG, 0x80); // 关闭定时器
		        break;
		   }
		}

		TmpETU--;

		if(TmpETU == 0)
		{
		     break;
		}
    }

}


void PN512_s_RFSetTimer(ulong nETU)
{
    uchar ucTemp = 0;
	uint uiTempClk = 0;

	if(nETU == RF_FWTMIN)
	{
	    // TPrescale = 0x00
        // 定时器频率为6.78MHz
        ucTemp = 0;
	    PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
	    PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload = 0x26A, 约为618CLk
		ucTemp = 0x02;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = 0x6A;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}
	else if(nETU < 1024)
	{
	    // TPrescale = 0x00
        // 定时器频率为6.78MHz
        ucTemp = 0;
	    PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
	    PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload
		uiTempClk = nETU*64 - 1;  // 64Clk为一个ETU
		ucTemp = uiTempClk >> 8;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = uiTempClk & 0x0FF;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}
	else if(nETU < 2048)
	{
	    // TPrescale = 0x01
        // 定时器频率为3.39MHz
        ucTemp = 0;
	    PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
		ucTemp = 0x01;
	    PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload
		uiTempClk = nETU*32 - 1;  // 32Clk为一个ETU
		ucTemp = uiTempClk >> 8;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = uiTempClk & 0x0FF;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}
	else if(nETU < 4096)
	{
	    // TPrescale = 0x03
        // 定时器频率为1.695MHz
        ucTemp = 0;
	    PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
		ucTemp = 0x03;
	    PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload
		uiTempClk = nETU*16 - 1;  // 16Clk为一个ETU
		ucTemp = uiTempClk >> 8;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = uiTempClk & 0x0FF;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}
	else if(nETU < 8192)
	{
		// TPrescale = 0x07
		// 定时器频率为847.5KHz
		ucTemp = 0;
		PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
		ucTemp = 0x07;
		PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload
		uiTempClk = nETU*8 - 1;  // 8Clk为一个ETU
		ucTemp = uiTempClk >> 8;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = uiTempClk & 0x0FF;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}
	else if(nETU < 16384)
	{
		// TPrescale = 0x0F
		// 定时器频率为423.75KHz
		ucTemp = 0;
		PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
		ucTemp = 0x0F;
		PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload
		uiTempClk = nETU*4 - 1;  // 4Clk为一个ETU
		ucTemp = uiTempClk >> 8;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = uiTempClk & 0x0FF;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}
	else if(nETU < 32768)
	{
		// TPrescale = 0x1F
		// 定时器频率为211.875KHz
		ucTemp = 0;
		PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
		ucTemp = 0x1F;
		PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload
		uiTempClk = nETU*2 - 1;  // 2Clk为一个ETU
		ucTemp = uiTempClk >> 8;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = uiTempClk & 0x0FF;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}
	else if(nETU < 65536)
	{
		// TPrescale = 0x3F
		// 定时器频率为105.9375KHz
		ucTemp = 0;
		PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
		ucTemp = 0x3F;
		PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload
		uiTempClk = nETU - 1;  // 1Clk为一个ETU
		ucTemp = uiTempClk >> 8;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = uiTempClk & 0x0FF;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}
	else if(nETU < 131072)
	{
	    // TPrescale = 0x7F
		// 定时器频率为52.96875KHz
		ucTemp = 0;
		PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
		ucTemp = 0x7F;
		PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload
		uiTempClk = nETU/2 - 1;  // 1Clk为2个ETU
		ucTemp = uiTempClk >> 8;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = uiTempClk & 0x0FF;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}
	else if(nETU < 262144)
	{
	    // TPrescale = 0xFF
		// 定时器频率为26.484375KHz
		ucTemp = 0;
		PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
		ucTemp = 0xFF;
		PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload
		uiTempClk = nETU/4 - 1;  // 1Clk为4个ETU
		ucTemp = uiTempClk >> 8;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = uiTempClk & 0x0FF;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}
	else if(nETU < 524288)
	{
	    // TPrescale = 0x1FF
		// 定时器频率为13.2421875KHz
		ucTemp = 0x01;
		PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
		ucTemp = 0xFF;
		PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload
		uiTempClk = nETU/8 - 1;  // 1Clk为8个ETU
		ucTemp = uiTempClk >> 8;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = uiTempClk & 0x0FF;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}
	else if(nETU < 1048576)
	{
	    // TPrescale = 0x3FF
		// 定时器频率为6.62KHz
		ucTemp = 0x03;
		PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
		ucTemp = 0xFF;
		PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload
		uiTempClk = nETU/16 - 1;  // 1Clk为16个ETU
		ucTemp = uiTempClk >> 8;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = uiTempClk & 0x0FF;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}
	else if(nETU < 2095152)
	{
	    // TPrescale = 0x7FF
		// 定时器频率为3.31KHz
		ucTemp = 0x07;
		PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
		ucTemp = 0xFF;
		PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload
		uiTempClk = nETU/32 - 1;  // 1Clk为32个ETU
		ucTemp = uiTempClk >> 8;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = uiTempClk & 0x0FF;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}
	else
	{
	    // TPrescale = 0xFFF
		// 定时器频率为1.655KHz
		ucTemp = 0x0F;
		PN512_s_vRFWriteReg(1,PN512_TMODE_REG, &ucTemp);
		ucTemp = 0xFF;
		PN512_s_vRFWriteReg(1,PN512_TPRESCALER_REG, &ucTemp);

		// TReload
		uiTempClk = nETU/64 - 1;  // 1Clk为64个ETU
		ucTemp = uiTempClk >> 8;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_H_REG, &ucTemp);
		ucTemp = uiTempClk & 0x0FF;
		PN512_s_vRFWriteReg(1,PN512_TRELOAD_L_REG, &ucTemp);
	}


}


void PN512_s_RFOpenCarrier(void)
{	
	uchar ucTemp = 0;
	ucTemp = 0xA3;
	PN512_s_vRFWriteReg(1,PN512_TXCONTROL_REG,&ucTemp); 
}


void PN512_s_RFCloseCarrier(void)
{
	uchar ucTemp = 0;
	ucTemp = 0x30;
	PN512_s_vRFWriteReg(1,PN512_TXCONTROL_REG,&ucTemp);
    PN512_s_RFDelay10ETUs(RF_TRESET/10);
}


void PN512_s_RFResetCarrier(void)
{	uchar ucTemp = 0;
	ucTemp = 0x30; 
	PN512_s_vRFWriteReg(1,PN512_TXCONTROL_REG,&ucTemp); 
    PN512_s_RFDelay10ETUs(RF_TRESET/10);
	ucTemp = 0xA3;
	PN512_s_vRFWriteReg(1,PN512_TXCONTROL_REG,&ucTemp);
}




#ifdef PN512_Debug_Isr
uint gl_LoAlert=0,gl_TxIRQ=0,gl_RxIRQ=0,gl_HiAlert_Idle=0,gl_HiAlert=0,gl_Idle=0,gl_timer=0;
uchar gl_err_flg1=0,gl_err_flg2=0,gl_err_flg3=0,gl_err_flg4=0,gl_loalertNum=0;
uchar gl_err_flg5=0,gl_err_flg6=0,gl_err_flg5_1=0,gl_err_flg6_1=0,gl_err_flg5_2=0,gl_err_flg6_2=0,gl_err_flg5_3=0,gl_err_flg6_3=0;
uchar isr_test_arr[20];
extern volatile unsigned char rf_get_value;
#endif

#ifdef FOR_LINUX_SYS
DECLARE_WAIT_QUEUE_HEAD(pn512_rf_wq);//定义一个等待队列头
irqreturn_t PN512_s_RFIsr(int irq, void *dev_id, struct pt_regs *regs)
#else
void PN512_s_RFIsr(void)
#endif
//
{
	uchar ucTempData;
	uchar ucIrqPending;
	ulong ulTemp;
	ulong i = 0;//for debug 
	//AT skx
	//uint for_spi_test=0;
    ulong ulTemp1 = 0;

	PN512_sHal_MaskCpuInt();//禁止并清CPU中断
	
	PN512_s_vRFSwitchNCS(1);//片选拉高一次
	WaitNuS(100);
	//while( Get_Spi1_Status()&0x01 )  Get_Spi1_Data();
	
	// 读取错误寄存器
	PN512_s_vRFReadReg(1,PN512_ERROR_REG,(uchar*)&PN512_RF_WorkInfo.ucErrFlags);
	
	PN512_RF_WorkInfo.ucErrFlags &= 0x1f;
 
	// 如果是冲突错误, 需要读取冲突位置
	if(((PN512_RF_WorkInfo.ucAnticol == PICC_ANTICOLL1) && ((PN512_RF_WorkInfo.ucErrFlags & 0x08) != 0))
	   || ((PN512_RF_WorkInfo.ucAnticol ==PICC_ANTICOLL2) && ((PN512_RF_WorkInfo.ucErrFlags & 0x08) != 0))
	   || ((PN512_RF_WorkInfo.ucAnticol == PICC_ANTICOLL3) && ((PN512_RF_WorkInfo.ucErrFlags & 0x08) != 0)))
	{
		PN512_s_vRFReadReg(1, PN512_COLL_REG, (uchar*)&PN512_RF_WorkInfo.ucCollPos); 
	}
	
	if((PN512_RF_WorkInfo.ucErrFlags & 0x08) == 0x08)
	{
		// 在产生冲突错误情况下，忽略奇偶校验错误
		PN512_RF_WorkInfo.ucErrFlags &= 0xfd;
	}

	#ifdef PN512_Debug_Isr
		
	 		if(gl_loalertNum==0)
	 				gl_err_flg1=PN512_RF_WorkInfo.ucErrFlags;
	 		if(gl_loalertNum==1)
	 				gl_err_flg2=PN512_RF_WorkInfo.ucErrFlags;
	 		if(gl_loalertNum==2)
	 				gl_err_flg3=PN512_RF_WorkInfo.ucErrFlags;
	 		if(gl_loalertNum==3)
	 				gl_err_flg4=PN512_RF_WorkInfo.ucErrFlags;

	 		gl_loalertNum++;
	#endif	
	
	
	// 持续进行，直到处理完所有中断请求
    while (1)
    {
        PN512_s_vRFReadReg(1,PN512_STATUS1_REG,&ucTempData);
				
        if ((ucTempData & 0x10) == 0)
        {
        	#ifdef PN512_Debug_Isr
		debug_printf(0,0,0,"isr in but not isr happen !,stat:%02x",ucTempData);
		#endif
            // 没有未处理中断
            break;
        }
				// 读取中断请求位，屏蔽掉不需要的中断，并保存
        PN512_s_vRFReadReg(1,PN512_COMMIEN_REG,&ucTempData);
        PN512_s_vRFReadReg(1,PN512_COMMIRQ_REG,&ucIrqPending);

			
        ucIrqPending &= ucTempData;
        PN512_RF_WorkInfo.ucINTSource |= ucIrqPending;

        // 依次处理各个中断请求
        if((ucIrqPending & 0x04) != 0)
        {
					#ifdef PN512_Debug_Isr
					gl_LoAlert++;
					#endif
            // 查看缓冲区剩余长度
            PN512_s_vRFReadReg(1, PN512_FIFOLEVEL_REG, &ucTempData);
            ulTemp = 64 - ucTempData;
			
            // 决定本次要发送数据长度
            ulTemp1 = PN512_RF_WorkInfo.ulBytesSent;
            //if( (PN512_RF_WorkInfo.ulSendBytes - PN512_RF_WorkInfo.ulBytesSent) <= ulTemp)
            if( (PN512_RF_WorkInfo.ulSendBytes - ulTemp1) <= ulTemp)
            {
                // 如果需要发送长度小于缓冲区剩余空间，则全部发送
                ulTemp1 = PN512_RF_WorkInfo.ulBytesSent;
                //ulTemp = PN512_RF_WorkInfo.ulSendBytes - PN512_RF_WorkInfo.ulBytesSent;
                ulTemp = PN512_RF_WorkInfo.ulSendBytes - ulTemp1;
                ucTempData = 0x04;
			
                // 发送完毕, 禁止中断LoAlert
                PN512_s_vRFClrBitMask(PN512_COMMIEN_REG, ucTempData);				
            }
					
            // 发送数据         
			for(i = 0; i<ulTemp; i++) 
			{
				PN512_s_vRFWriteReg(1,PN512_FIFODATA_REG,(uchar*)(PN512_RF_WorkInfo.aucBuffer+PN512_RF_WorkInfo.ulBytesSent));
				PN512_RF_WorkInfo.ulBytesSent++;
			}

			PN512_s_vRFClrBitMask(PN512_BITFRAMING_REG, 0x80);
			//以下命令放在中断外面启动，防止重复启动命令导致PN512状态机混乱接收超时 modify by  skx 2009-05-04
			if(PN512_RF_WorkInfo.ucCmd==PN512_MFAuthent_CMD)//但PN512_MFAuthent_CMD命令必须在中断内启动
			PN512_s_vRFWriteReg(1,PN512_COMMAND_REG,(uchar*)(&PN512_RF_WorkInfo.ucCmd));
					
			if(PN512_RF_WorkInfo.ucCmd == PN512_TRANSCEIVE_CMD)
			{
				// 设置StartSend Bit 
				//在TRANSCEIVE命令下一定需要启动发送位
				//在TRANS命令下不需要启动发送位也会自动发送
				PN512_s_vRFSetBitMask(PN512_BITFRAMING_REG, 0x80);
			}
			
            // 清除中断请求
            ucTempData = 0x04;
            PN512_s_vRFWriteReg(1, PN512_COMMIRQ_REG, &ucTempData);		
        }

        if((ucIrqPending & 0x40 ) != 0)
        {
			#ifdef PN512_Debug_Isr
			gl_TxIRQ++;
			#endif
            // TxIRQ
					// 清除中断请求
            ucTempData = 0x40;
            PN512_s_vRFWriteReg(1,PN512_COMMIRQ_REG,&ucTempData);

            // 允许HiAlert中断请求，准备接收响应
            ucTempData = 0x08;
            PN512_s_vRFSetBitMask(PN512_COMMIEN_REG,ucTempData);
			
        }

        if((ucIrqPending & 0x20) != 0)
        {
			#ifdef PN512_Debug_Isr
			gl_RxIRQ++;
			#endif
            // RxIRQ
						// 没有发生错误, 则可以停止命令
           if (PN512_RF_WorkInfo.ucErrFlags == 0 || PN512_RF_WorkInfo.ucM1Stage == 1)//20080422
            {
				ucTempData = 0;
				PN512_s_vRFWriteReg(1,PN512_COMMAND_REG,&ucTempData);
				//设置空闲中断请求，以表明处理结束
				ucIrqPending |= 0x10;
            }
           else
            {
            	// 有错误产生
                // 保存错误
                PN512_RF_WorkInfo.ucSaveErrState = PN512_RF_WorkInfo.ucErrFlags;
                // 清空FIFO
                PN512_s_vRFFlushFIFO();

                // 没有接收到数据
                PN512_RF_WorkInfo.ulBytesReceived = 0x00;
                // 下面不再处理 RxIRQ请求
                ucIrqPending &= ~0x20;
                // 清除中断请求
                ucTempData = 0x20;
                PN512_s_vRFWriteReg(1,PN512_COMMIRQ_REG,&ucTempData);
            }
        }

        if((ucIrqPending & 0x38) != 0)
        {
			#ifdef PN512_Debug_Isr
			gl_HiAlert_Idle++;
			#endif
    		// HiAlert, Idle or valid RxIRQ
			if((ucIrqPending & 0x28)!=0)
			{		 
				// 获取缓冲区长度
				PN512_s_vRFReadReg(1,PN512_FIFOLEVEL_REG,&ucTempData);
				
				if(ucTempData > 0 && PN512_RF_WorkInfo.ucCurResult == RET_RF_OK)
				{
					for(i = 0; i<ucTempData; i++)
					{				
						PN512_s_vRFReadReg(1,PN512_FIFODATA_REG,
					  (uchar*)(PN512_RF_WorkInfo.aucBuffer+PN512_RF_WorkInfo.ulBytesReceived));
					  PN512_RF_WorkInfo.ulBytesReceived++;				
					}
				}
				
				//  清空RxIRQ和HiAlert请求
				ucTempData = 0x28 & ucIrqPending;
				PN512_s_vRFWriteReg(1,PN512_COMMIRQ_REG,&ucTempData);
			}
        }

        if((ucIrqPending & 0x08) != 0)
        {
			#ifdef PN512_Debug_Isr
			gl_HiAlert++;
			#endif
          // HiAlertIRQ
          PN512_s_vRFReadReg(1,PN512_STATUS2_REG,&ucTempData);
          if((ucTempData & 0x07) == 0x06) // 接收器仍然打开
          {
              //停止定时器
              PN512_s_vRFSetBitMask(PN512_CONTROL_REG, 0x80); // 关闭定时器		
          }
        }
        // 接收错误，则终止运行
		if(PN512_RF_WorkInfo.ucSaveErrState != 0 && PN512_RF_WorkInfo.ucM1Stage == 0)//20080422
		{
			ucTempData = 0;
      		PN512_s_vRFWriteReg(1,PN512_COMMAND_REG,&ucTempData);
      		//设置空闲中断请求，以表明处理结束
			ucIrqPending |= 0x10;
		}//

		if((ucIrqPending & 0x10) != 0)
		{
			#ifdef PN512_Debug_Isr
			gl_Idle++;
			#endif
		    // Idle IRQ
		    ucTempData = 0x01;
		    // 禁止定时器中断
		    PN512_s_vRFClrBitMask(PN512_COMMIEN_REG,ucTempData);
		
		    ucTempData = 0x11;
		    // 清除定时器中断请求和空闲中断请求
		    PN512_s_vRFWriteReg(1,PN512_COMMIRQ_REG,&ucTempData);
		
		    ucIrqPending &= ~0x01;   // 下面不处理定时器中断
		    // 清除掉定时器中断, 产生IDLE中断
		    PN512_RF_WorkInfo.ucINTSource = (PN512_RF_WorkInfo.ucINTSource & ~0x01)|0x10;	
		    #ifdef FOR_LINUX_SYS
                    wake_up_interruptible(&pn512_rf_wq);//进程阻塞等待退出条件
                    #endif		    
		}

    if((ucIrqPending & 0x01) != 0)
    {
		#ifdef PN512_Debug_Isr
		gl_timer++;
		#endif
      // timer IRQ
			// 清除定时器中断请求
		ucTempData = 0x01;
		PN512_s_vRFWriteReg(1,PN512_COMMIRQ_REG,&ucTempData);

      // 超时错误
      if(PN512_RF_WorkInfo.ucCurResult == RET_RF_OK)
      {
		#ifdef PN512_Debug_Isr
			#ifdef FOR_LINUX_SYS
			printk("/*****Isr TimerOut!******/\n");
			#endif
		#endif
		PN512_RF_WorkInfo.ucCurResult = RET_RF_ERR_TIMEOUT;
      }
      #ifdef FOR_LINUX_SYS
      wake_up_interruptible(&pn512_rf_wq);//进程阻塞等待退出条件
      #endif		    
        // otherwise ignore the interrupt
 	}
			
  }// while (1)
 
	PN512_sHal_EnCpuInt();//使能CPU中断
    
        #ifdef FOR_LINUX_SYS
	return IRQ_HANDLED;
        #else
        return ;
        #endif
}

extern void ledall_off(void);
extern void ledall_on(void);
extern void ledred_on(void);
extern void ledred_off(void);
extern void ledgreen_on(void);
extern void ledgreen_off(void);

uchar PN512_s_RFExchangeCmd(uchar ucCmd)
{
	ulong ulTemp = 0;
	uchar ucTempData;		 // 暂存要操作的寄存器数据
	uchar ucWaterLevelBak;	 // 备份FIFO的WaterLevel值
	uchar ucTimerCtl;		 // 命令中用到的定时器控制方式
	uchar ucIntWait;		 // 命令结束时期待发生的中断
	uchar ucIntEnable;		 // 该命令允许发生的中断
	//uchar ucrxMultiple; 	 // 是否要接收多个帧
	
	//ulong timeout=0;//for linux

	//AT skx
	//uint intreg[10];

	// 给工作变量赋初始值
	PN512_RF_WorkInfo.ucCurResult		=	RET_RF_OK;
	PN512_RF_WorkInfo.ucINTSource		=	0;
	PN512_RF_WorkInfo.ulBytesSent		=	0;		// how many bytes already sent
	PN512_RF_WorkInfo.ulBytesReceived	=	0;		// how many bytes received
	PN512_RF_WorkInfo.lBitsReceived 	=	0;		// how many bits received
	PN512_RF_WorkInfo.ucErrFlags		=	0;
	PN512_RF_WorkInfo.ucSaveErrState	=	0x00;
	PN512_RF_WorkInfo.ucCollPos     =   0;
	PN512_RF_WorkInfo.ucCmd = PN512_IDLE_CMD;

	//ucrxMultiple		   = 0x00;
	ucIntEnable 		   = 0x00;
	ucIntWait			   = 0x00;
	ucTimerCtl			   = 0x00;

	#ifdef PN512_Debug_Isr
	gl_loalertNum=0;
	gl_LoAlert=gl_TxIRQ=gl_RxIRQ=gl_HiAlert_Idle=gl_HiAlert=gl_Idle=gl_timer=0;
	gl_err_flg1=gl_err_flg2=gl_err_flg3=gl_err_flg4=0;
	gl_err_flg5=gl_err_flg6=gl_err_flg5_1=gl_err_flg6_1=gl_err_flg5_2=gl_err_flg6_2=gl_err_flg5_3=gl_err_flg6_3=0;
	#endif
	
	// 禁止并清除所有中断源
	ucTempData = 0x7F;
	PN512_s_vRFClrBitMask(PN512_COMMIEN_REG,ucTempData);
	
	PN512_s_vRFWriteReg(1,PN512_COMMIRQ_REG,&ucTempData);
	
	ucTempData = 0x1F;
	PN512_s_vRFClrBitMask(PN512_DIVIEN_REG,ucTempData);	
	PN512_s_vRFWriteReg(1,PN512_DIVIRQ_REG,&ucTempData);
	
   	// 结束当前正在运行的命令
	ucTempData = PN512_IDLE_CMD;
	PN512_s_vRFWriteReg(1,PN512_COMMAND_REG,&ucTempData); 

        // 设置定时器控制方式
        PN512_s_vRFSetBitMask(PN512_CONTROL_REG,0x80);// 关闭定时器		
	// 清空FIFO缓冲区
	PN512_s_vRFFlushFIFO();

	// 设置FIFO警告值
	PN512_s_vRFReadReg(1,PN512_WATERLEVEL_REG,&ucWaterLevelBak);
	ucTempData = 0x10;
	PN512_s_vRFWriteReg(1,PN512_WATERLEVEL_REG,&ucTempData);
		
	// 根据不同命令进行处理
	switch(ucCmd)
	{
	    case PN512_SOFTRESET_CMD: // for debug
		{
			ucTimerCtl	= 0x00; //手动开始, 数据接收完毕时定时器自动停止
			ucIntEnable = 0x18; // HiAlert and IdleIRq
			ucIntWait	= 0x10; // IdleIRq
			break;
		}
		
		case PN512_IDLE_CMD:
		{
			// 空闲
			ucTimerCtl	= 0x00;  //不使用定时器
			ucIntEnable = 0x00;  //不允许中断
			ucIntWait	= 0x00;  //不等待中断
			break;
		}

		case PN512_CONFIG_CMD:
		{
			// 载入配置
			ucTimerCtl	= 0x00; // 手动启动和停止定时器
			ucIntEnable = 0x14; // IdleIRq and LoAlert
			ucIntWait	= 0x10; // IdleIRq
			break;
		}
		case PN512_GRAMID_CMD:
		{
			ucTimerCtl	= 0x80;
			ucIntEnable = 0x10; // IdleIRq and LoAlert
			ucIntWait	= 0x10; // IdleIRq
			break;
		}
		case PN512_CALCRC_CMD:
		{
			//CRC校验
			ucTimerCtl	= 0x00; // 手动启动和停止定时器
			ucIntEnable = 0x11; // LoAlert and TxIRq
			ucIntWait	= 0x10; // TxIRq
			break;
		}
		case PN512_RECEIVE_CMD:
		{
			//接收数据
			//根据接收起始位的位置来计算实际收到位数
			PN512_s_vRFReadReg(1,PN512_BITFRAMING_REG,&ucTempData);
			PN512_RF_WorkInfo.lBitsReceived = -(long)(ucTempData >> 4);

			ucTimerCtl	= 0x00; //手动开始, 数据接收完毕时定时器自动停止
			ucIntEnable = 0x18; // HiAlert and IdleIRq
			ucIntWait	= 0x10; // IdleIRq
			break;
		}
		case PN512_MFAuthent_CMD:
		{
			ucTimerCtl	= 0x80;
			ucIntEnable = 0x14;// changed by liuxl 20071228
			ucIntWait	= 0x10; // IdleIRq
			break;
		}
		case PN512_TRANSMIT_CMD:
		{
			//发送数据
			ucTimerCtl	= 0x00; // 手动启动和停止定时器
			ucIntEnable = 0x14; // LoAlert and IdleIRq
			ucIntWait	= 0x10; // IdleIRq
			break;
		}
		case PN512_TRANSCEIVE_CMD:
		{	
			// 接收数据
			// 根据接收起始位的位置来计算实际收到位数
			PN512_s_vRFReadReg(1,PN512_BITFRAMING_REG,&ucTempData);
			PN512_RF_WorkInfo.lBitsReceived = -(long)(ucTempData >> 4);
    		ucTimerCtl	= 0x80; // 数据发送完毕时定时器自动开始
			ucIntEnable = 0x74; // TxIrq, RxIrq, IdleIRq and LoAlert
			ucIntWait	= 0x10; // IdleIRq			
			break;
		}
		default:
		{
			// 非法命令
			PN512_RF_WorkInfo.ucCurResult = RET_RF_ERR_PARAM;
			break;
		}
	}

	// 刚才没有错误
	if (PN512_RF_WorkInfo.ucCurResult == RET_RF_OK)
	{
		
		//AT
		//if(PN512_RF_WorkInfo.f_ispoll)//如果是felica的polling命令，则启用多帧接收模式
		{
			//PN512_s_vRFSetBitMask(PN512_RXMODE_REG,0X04);//启用多帧模式
		}
		//else
		{
			PN512_s_vRFClrBitMask(PN512_RXMODE_REG,0X04);//恢复为单帧
		}

	    // 设置定时器控制方式
        PN512_s_vRFSetBitMask(PN512_TMODE_REG,ucTimerCtl);
		
        // 如果定时器要人工启动，就启动它
        if ((~ucTimerCtl & 0x80) != 0 )
        {
			PN512_s_vRFSetBitMask(PN512_CONTROL_REG,0x40);// 启动定时器			
        }
      
		PN512_RF_WorkInfo.ucCmd = ucCmd;

		//将上面屏蔽的代码转移到命令发送完成后，避免因为其它中断或者优先级高的任务导致命令发送操作没有执行而使操作失败		
		// 总是允许定时器超时中断,并且等待这种中断
		ucIntEnable |= 0x01;
		ucIntWait	|= 0x01;
 
		
		//如果先启动PN512_MFAuthent_CMD命令则FIFO被锁而导致M1卡认证失败，故PN512_MFAuthent_CMD须在中断中填写FIFO再启动
		if(PN512_RF_WorkInfo.ucCmd!=PN512_MFAuthent_CMD)
		PN512_s_vRFWriteReg(1,PN512_COMMAND_REG,(uchar*)(&PN512_RF_WorkInfo.ucCmd));//启动命令 add by skx 2009-05-04
		
		//首先开CPU中断，再开PN521中断
		//PN512_sHal_EnCpuInt();//使能CPU中断
                
                //AT skx
		//EXTI_DisableITBit(BIT8);//mask BIT8 first
		EXTI_DisableITBit(BIT1);//mask BIT1 first
                
		//EXTI_ReadAllReg(intreg);
                //debug_printf(0,0,0,"first in_1,%08X/%08x/%08x/%08x/%08x",
		//			intreg[0],intreg[1],intreg[2],intreg[3],intreg[4],intreg[5]);
                //PN512_s_vRFReadReg(1,PN512_COMMIEN_REG,&isr_test_arr[0]);
                //debug_printf(0,0,0,"first in,IEN:%02X/%02x",ucIntEnable,isr_test_arr[0]);
		PN512_s_vRFSetBitMask(PN512_COMMIEN_REG,ucIntEnable);//开PN512中断，由于中断导致最后一次CS没有被拉高，须在中断进入后进行一次CS拉高处理
		//EXTI_EnableEventBit(BIT8);//do not need enable eventbit
		PN512_sHal_EnCpuInt();//使能CPU中断
		//EXTI_GenerateSWInterrupt(BIT8);
		EXTI_GenerateSWInterrupt(BIT1);
		
		// 等待期望的中断请求发生，这样就可以结束命令
		// 如果超过10秒没有处理，则返回错误
		
    	//timeout= HZ*10;//10S超时
    	//for debug
    	#ifdef FOR_LINUX_SYS
	    	timeout= HZ;//1S超时
	    	wait_event_interruptible_timeout(pn512_rf_wq, (PN512_RF_WorkInfo.ucINTSource & ucIntWait), timeout);//使进程深度睡眠,直到超时或者条件成立
		#else

                    unsigned int uiBeginTime = GetTimerCount();
			while(1)
			{			
				if((PN512_RF_WorkInfo.ucINTSource & ucIntWait) != 0)
				{		
					break;
				}
                		if(( GetTimerCount()-uiBeginTime )>(100*5))
			        {
				    break;
			        }
				//AT skx
                                
                                /*
				#ifdef PN512_Debug_Isr
				isr_test_arr[10]=0x35;
				PN512_s_vRFWriteReg(1,0x0b,&isr_test_arr[10]);

				PN512_s_vRFReadReg(1,PN512_COMMIEN_REG,&isr_test_arr[1]);
				debug_printf(0,0,0,"*****COMMIEN:%02x***",isr_test_arr[1]);
				
				PN512_s_vRFReadReg(1,0x03,&isr_test_arr[0]);
				debug_printf(0,0,0,"isr out_0,Div:%02x",isr_test_arr[0]);
				
		        	PN512_s_vRFReadReg(1,PN512_COMMIRQ_REG,&isr_test_arr[2]);
		        	debug_printf(0,0,0,"INTEN:%02x",isr_test_arr[2]);
					
					PN512_s_vRFReadReg(1,0x06,&isr_test_arr[3]);
		        	debug_printf(0,0,0,"Err:%02x",isr_test_arr[3]);
					PN512_s_vRFReadReg(1,0X07,&isr_test_arr[4]);
		        	debug_printf(0,0,0,"Stat1:%02x",isr_test_arr[4]);
					PN512_s_vRFReadReg(1,0x08,&isr_test_arr[5]);
		        	debug_printf(0,0,0,"Stat2:%02x",isr_test_arr[5]);
					PN512_s_vRFReadReg(1,0x09,&isr_test_arr[6]);
		        	debug_printf(0,0,0,"FIFOD:%02x",isr_test_arr[6]);
					PN512_s_vRFReadReg(1,0X0A,&isr_test_arr[7]);
		        	debug_printf(0,0,0,"FIFOLen:%02x",isr_test_arr[7]);
					PN512_s_vRFReadReg(1,0x0b,&isr_test_arr[8]);
		        	debug_printf(0,0,0,"FIFO W:%02x",isr_test_arr[8]);
				#endif
				//delay_ms(10);
				//
				if((GetTimerCount() - uiBeginTime) >= 1*1000*10)	// 10s
				{
				  	PN512_RF_WorkInfo.usErrNo = 0xA0; // timeout err
					PN512_RF_WorkInfo.ucCurResult = RET_RF_ERR_TIMEOUT;
					ScrPrint(0,1,0,"cpu not int");
					getkey();
					break;
				}
				*/
			}
		#endif
		 
		PN512_sHal_MaskCpuInt();//禁止并清CPU中断
		

		#ifdef PN512_Debug_Isr
			#ifdef FOR_LINUX_SYS
			{
				printk(KERN_INFO"int:%d/%d/%d/%d/%d/%d/%d\n",gl_LoAlert,gl_TxIRQ,gl_RxIRQ,gl_HiAlert_Idle,gl_HiAlert,gl_Idle,gl_timer);
				printk(KERN_INFO"errFlg:%02x/%02x/%02x/%02x;%d\n",gl_err_flg1,gl_err_flg2,gl_err_flg3,gl_err_flg4,gl_loalertNum);
				printk(KERN_INFO"int:%02x,%02x/%02x,%02x/%02x,%02x/%02x,%02x\n",
					gl_err_flg5,gl_err_flg6,gl_err_flg5_1,gl_err_flg6_1,gl_err_flg5_2,gl_err_flg6_2,gl_err_flg5_3,gl_err_flg6_3);
				getkey();
			}//end
			#else
			{
				debug_printf(0,0,0,"int:%d/%d/%d/%d/%d/%d/%d\n",gl_LoAlert,gl_TxIRQ,gl_RxIRQ,gl_HiAlert_Idle,gl_HiAlert,gl_Idle,gl_timer);
				debug_printf(0,0,0,"errFlg:%02x/%02x/%02x/%02x;%d\n",gl_err_flg1,gl_err_flg2,gl_err_flg3,gl_err_flg4,gl_loalertNum);
				debug_printf(0,0,0,"int:%02x,%02x/%02x,%02x/%02x,%02x/%02x,%02x\n",
					gl_err_flg5,gl_err_flg6,gl_err_flg5_1,gl_err_flg6_1,gl_err_flg5_2,gl_err_flg6_2,gl_err_flg5_3,gl_err_flg6_3);
				//getkey();
			}
			#endif
		#endif
		
		// 命令结束处理
		PN512_s_vRFSetBitMask(PN512_CONTROL_REG, 0x80); // 关闭定时器
		ucTempData = 0x7F;
		PN512_s_vRFClrBitMask(PN512_COMMIEN_REG,ucTempData); // 禁止所有中断
		PN512_s_vRFWriteReg(1,PN512_COMMIRQ_REG,&ucTempData); // 清除所有中断请求位

		// 返回到IDLE模式
		ucTempData = PN512_IDLE_CMD;
		PN512_s_vRFWriteReg(1, PN512_COMMAND_REG,&ucTempData); 

		//收集产生的错误
		ulTemp = PN512_RF_WorkInfo.ucSaveErrState;
		//PN512_RF_WorkInfo.ucErrFlags |= PN512_RF_WorkInfo.ucSaveErrState;
        PN512_RF_WorkInfo.ucErrFlags |= ulTemp;

		// 返回错误
		if(PN512_RF_WorkInfo.ucErrFlags != 0)
		{
			if((PN512_RF_WorkInfo.ucErrFlags & 0x08) != 0)
			{
				// 冲突				
     	  		PN512_RF_WorkInfo.usErrNo = 0x03; // collision err
				PN512_RF_WorkInfo.ucCurResult = RET_RF_ERR_MULTI_CARD;

			}
			else if((PN512_RF_WorkInfo.ucErrFlags & 0x02) != 0)
			{
				// 奇偶校验错
				PN512_RF_WorkInfo.usErrNo = 0x04; // Parity err
				PN512_RF_WorkInfo.ucCurResult = RET_RF_ERR_TRANSMIT;
			}
			else if((PN512_RF_WorkInfo.ucErrFlags & 0x01) != 0)
			{
				// 帧错误
				PN512_RF_WorkInfo.usErrNo = 0x05; // framing err
				PN512_RF_WorkInfo.ucCurResult = RET_RF_ERR_TRANSMIT;
			}
			else if((PN512_RF_WorkInfo.ucErrFlags & 0x10) != 0)
			{
				// FIFO溢出
				PN512_s_vRFFlushFIFO();
				PN512_RF_WorkInfo.usErrNo = 0x06; // FIFO Over err
				PN512_RF_WorkInfo.ucCurResult = RET_RF_ERR_TRANSMIT;
			}
			else if((PN512_RF_WorkInfo.ucErrFlags & 0x04) != 0)
			{
				// CRC错误
				PN512_RF_WorkInfo.usErrNo = 0x07; // CRC err 
				PN512_RF_WorkInfo.ucCurResult = RET_RF_ERR_TRANSMIT;
			}
			else if(PN512_RF_WorkInfo.ucCurResult == RET_RF_OK) //?
			{
				PN512_RF_WorkInfo.usErrNo = 0x08; // other err 
				PN512_RF_WorkInfo.ucCurResult = RET_RF_ERR_TRANSMIT;
			}
		}

		//先获取数据长度再执行错误处理 
		// 如果命令是接收或者是发送接收，必须记录接收的字节数和字数
		if((ucCmd == PN512_TRANSCEIVE_CMD) || (ucCmd== PN512_RECEIVE_CMD))
		{
			// 接收的最后一个字节位数
			PN512_s_vRFReadReg (1, PN512_CONTROL_REG, &ucTempData);
			ucTempData &= 0x07;
			if (ucTempData != 0)
			{
				// 最后一个字节没有接收完
				ulTemp = PN512_RF_WorkInfo.ulBytesReceived;
				//PN512_RF_WorkInfo.lBitsReceived += (PN512_RF_WorkInfo.ulBytesReceived-1) * 8 + ucTempData;
				PN512_RF_WorkInfo.lBitsReceived += (ulTemp-1) * 8 + ucTempData;
			}
			else
			{
				// 最后一个字节接收完
				ulTemp = PN512_RF_WorkInfo.ulBytesReceived;
				//PN512_RF_WorkInfo.lBitsReceived += PN512_RF_WorkInfo.ulBytesReceived * 8;
				PN512_RF_WorkInfo.lBitsReceived += ulTemp * 8;
			}
		}
	}

	PN512_s_vRFWriteReg(1,PN512_WATERLEVEL_REG,&ucWaterLevelBak);
		
	return PN512_RF_WorkInfo.ucCurResult;

}



void PN512_s_vRFSetBitMask (uchar ucRegAddr, uchar ucBitMask)
{
    uchar ucTemp;

    PN512_s_vRFReadReg(1, ucRegAddr, &ucTemp);
    ucTemp |= ucBitMask;

    PN512_s_vRFWriteReg(1, ucRegAddr, &ucTemp);
}


void PN512_s_vRFClrBitMask(uchar ucRegAddr, uchar ucBitMask)
{
    uchar ucTemp;

    PN512_s_vRFReadReg(1, ucRegAddr, &ucTemp); 
    ucTemp &= ~ucBitMask;

    PN512_s_vRFWriteReg(1, ucRegAddr, &ucTemp); 
}


void PN512_s_vRFFlushFIFO(void)
{
    uchar ucTemp;

    uchar ucRegAddr = PN512_FIFOLEVEL_REG;
    PN512_s_vRFReadReg(1, ucRegAddr, &ucTemp);
    ucTemp |= 0x80;
    PN512_s_vRFWriteReg(1, ucRegAddr, &ucTemp);
}


//添加一REQB命令函数
uchar  PN512_s_RF_ucREQB(uchar* pucOutLen, uchar* pucOut)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTempData;
	 uint uiFWITemp = 0x00;
	 

    PN512_RF_WorkInfo.ucSFGI = 0;
    PN512_RF_WorkInfo.ulSFGT = 60; 
    
	 PN512_s_vRFFlushFIFO();        //empty FIFO

     // 将载波调制为TypeB型
     PN512_s_vRFSelectType('B');
	 
	 // RxCRC and TxCRC enable, ISO14443B
	 ucTempData = 0x83;
     PN512_s_vRFWriteReg(1, PN512_TXMODE_REG, &ucTempData);
     ucTempData = 0x83;
	 PN512_s_vRFWriteReg(1, PN512_RXMODE_REG, &ucTempData);
       
     // 禁止crypto1单元
     PN512_s_vRFClrBitMask(PN512_STATUS2_REG, 0x08);

     PN512_s_RFSetTimer(65);  //RF_DEFAULT);// 超时等待60ETU

	  
	 // 准备发送数据: 命令码
	 #define PICC_REQB           0x00        // request ALL
     PN512_RF_WorkInfo.ulSendBytes = 3;    // how many bytes to send
     PN512_RF_WorkInfo.aucBuffer[0] = PICC_APF;
     PN512_RF_WorkInfo.aucBuffer[1] = 0x00;       // AFI = 0x00
     PN512_RF_WorkInfo.aucBuffer[2] = PICC_REQB;  // TimeSlot = 0,  REQB
    

     // 执行命令
     ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);

     if(ucRet == RET_RF_ERR_TIMEOUT) // 超时无响应
     {
           PN512_RF_WorkInfo.usErrNo = 0xA5; // timeout err
           return RET_RF_ERR_TIMEOUT;
     }
     if(ucRet != RET_RF_OK)
     {
           ucRet = RET_RF_ERR_TRANSMIT;
     }
     else
     { 
		    if(PN512_RF_WorkInfo.ulBytesReceived != 12)
            {
             	  PN512_RF_WorkInfo.usErrNo = 0x50; // received bytes err
                  ucRet = RET_RF_ERR_TRANSMIT;
				  goto WUPBEND;
            }
            if(PN512_RF_WorkInfo.aucBuffer[0] != 0x50)
            {
             	  PN512_RF_WorkInfo.usErrNo = 0x51; // received byte 1 err
				  ucRet = RET_RF_ERR_PROTOCOL;
                  goto WUPBEND;
            }

		    *pucOutLen = PN512_RF_WorkInfo.ulBytesReceived;
            memcpy(pucOut, (uchar*)PN512_RF_WorkInfo.aucBuffer, PN512_RF_WorkInfo.ulBytesReceived);

            PN512_RF_WorkInfo.ucUIDLen = 4;
            // 保存TypeB的UID序列号 第2~5字节
            memcpy((uchar*)PN512_RF_WorkInfo.ucUIDB, (uchar*)(PN512_RF_WorkInfo.aucBuffer + 1), 4);

            // 第6~9字节为Application Data,一般为0000，终端可以不予理会

            // 第10字节为BitRate，PCD与PICC一般只支持106KBit
            // 不管该字节回送任何值，均按106KBits/s进行通信
          
            // 第11字节返回帧大小和协议 高4位为帧大小，低4位为协议值
			PN512_RF_WorkInfo.FSC = PN512_gl_RF_ulFSList[(PN512_RF_WorkInfo.aucBuffer[10] >> 4) & 0x0F];

            // 低4位表示是否支持ISO14443协议，默认支持
            
            if((PN512_RF_WorkInfo.aucBuffer[10] & 0x01) != 0x01) 
            {
             	   PN512_RF_WorkInfo.usErrNo = 0x53; // received byte 11 err
                   ucRet = RET_RF_ERR_PROTOCOL;
				   goto WUPBEND;
            }

            // 第12字节包含FWI，CID和NAD等信息
            // 高4位返回FWI
            PN512_RF_WorkInfo.ucFWI = (PN512_RF_WorkInfo.aucBuffer[11] & 0x0F0) >> 4;

			uiFWITemp = (uint)0x01 << PN512_RF_WorkInfo.ucFWI;
			PN512_RF_WorkInfo.ulFWT = (ulong)(32 + 3) * uiFWITemp; //(256*16/Fc)*2^FWI + 3*2^FWI ETU
            // 第3~4位表示ADC，不予考虑
            // 第1~2位表示CID和NAD机制，不管PICC返回什么，均按不支持处理
            // 判断NAD
            if(PN512_RF_WorkInfo.aucBuffer[11] & 0x01)
            {
                 PN512_RF_WorkInfo.ucCIDFlag = 1;
            }
            else
            {
                 PN512_RF_WorkInfo.ucCIDFlag = 0;
            }
            // 判断CID
            if(PN512_RF_WorkInfo.aucBuffer[11] & 0x02)
            {
                 PN512_RF_WorkInfo.ucNADFlag = 1;
            }
            else
            {
                 PN512_RF_WorkInfo.ucNADFlag = 0;
            } 
     }

WUPBEND:
	 if(ucRet == 0)
	 {
	 	if(c_para.card_buffer_w == 1 && c_para.card_buffer_val)
			PN512_RF_WorkInfo.FSC = c_para.card_buffer_val;
	 }
	
     return ucRet;
}


/*********************************************************
以下为felica驱动相关函数
*********************************************************/

uchar PN512_felica_poll(uchar rate,uchar polarity,uchar *cmd,uint slen,uchar *Resp,ulong *rlen)
{
     uchar ucRet = RET_RF_OK;
     uchar ucTemp;

	 

	
	 //初始化工作变量
	 PN512_s_RF_vInitWorkInfo();
	 
     // 先将载波调制为felica默认配置，212Kbps,正向调制
     PN512_s_vRFSelectType('f');
	 if(rate)//表示为424速率
	 {
		 ucTemp = 0xA2;  // FeliCa, 424kBits/s, TxCRCEn
		 PN512_s_vRFWriteReg(1,PN512_TXMODE_REG,&ucTemp); 
		 ucTemp = 0xAA; // FeliCa, 424KBits/s, RxCRCEn, RxNoErr，//目前配置为单帧接收	
		PN512_s_vRFWriteReg(1,PN512_RXMODE_REG,&ucTemp); 
	 }
	 if(polarity)//表示反向调制输出
	 {
		PN512_s_vRFSetBitMask(PN512_TXMODE_REG, 0x08);//发送数据反向
	 }

	 PN512_s_RFOpenCarrier();//打开载波

	 //PN512_RF_WorkInfo.f_ispoll=1;//表示为polling命令，则需要设置为多帧接收模式
	 
	 // 设置定时器
	 PN512_s_RFSetTimer(4100);//超时时间为38MS

     // 禁止crypto1单元
     PN512_s_vRFClrBitMask(PN512_STATUS2_REG, 0x08);

	 // 准备发送数据: 命令码
	
     PN512_RF_WorkInfo.ulSendBytes = slen;    // how many bytes to send
	 memset((uchar*)PN512_RF_WorkInfo.aucBuffer, 0, sizeof(PN512_RF_WorkInfo.aucBuffer));
	//felica的同步信号0x00,0x00,0x00,0x00,0x00,0x00+0xb2,0x4d已经配置好FelNFC1Reg,PN512会自动添加 
	 memcpy((uchar*)PN512_RF_WorkInfo.aucBuffer,cmd,slen);//需要发送的命令
	 
	 ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);

	//if(ucRet )//如果出错则退出
		//return ucRet;
	
	 
	*rlen=PN512_RF_WorkInfo.ulBytesReceived;
	memcpy(Resp,(uchar*)PN512_RF_WorkInfo.aucBuffer,PN512_RF_WorkInfo.ulBytesReceived);//拷贝卡片回送数据
	Resp[*rlen]=PN512_RF_WorkInfo.usErrNo;
	(*rlen)++;

	PN512_RF_WorkInfo.f_ispoll=0;//表示polling命令结束，恢复单帧接收模式

	return ucRet;
}

//for felica
// ucRate: 0-212k 1-424k,default = 0
// ucPol: 0-正向，1-反向,default = 0
uchar PN512_s_RF_ucInit_Felica(uchar ucRate, uchar ucPol)
{	
	uchar ucTemp = 0;
	
	if(PN512_RF_WorkInfo.ucRFOpen == 0)
		return 0x02;// 模块未开启

	PN512_s_RF_vInitWorkInfo();
	
	// 先将载波调制为felica配置
	PN512_s_vRFSelectType('f');
	
	if (ucRate == RF_RATE_424_FELICA) 
	{
		ucTemp = 0xA2;  // FeliCa, 424kBits/s, TxCRCEn
		PN512_s_vRFWriteReg(1,PN512_TXMODE_REG,&ucTemp); 
		ucTemp = 0xAA; // FeliCa, 424KBits/s, RxCRCEn, RxNoErr，//目前配置为单帧接收	
		PN512_s_vRFWriteReg(1,PN512_RXMODE_REG,&ucTemp); 
	}
	if(ucPol == RF_POL_1_FELICA)//表示反向调制输出
	{
		PN512_s_vRFSetBitMask(PN512_TXMODE_REG, 0x08);//发送数据反向
	}
	
	PN512_s_RFOpenCarrier();//打开载波
	
	PN512_RF_WorkInfo.ucCurType = RF_TYPEF;
	return 0;
}

uchar PN512_s_RF_ucExchange_Felica(uint uiCmdLen, uchar* paucCmd, uint* puiRspLen, uchar* paucResp)
{
	uchar ucRet = RET_RF_OK;
		
	// 设置定时器
	PN512_s_RFSetTimer(4100);//超时时间为38MS
	
	// 禁止crypto1单元
	PN512_s_vRFClrBitMask(PN512_STATUS2_REG, 0x08);
	
	// 准备发送数据: 命令码	
	PN512_RF_WorkInfo.ulSendBytes = uiCmdLen;    // how many bytes to send
	memset((uchar*)PN512_RF_WorkInfo.aucBuffer, 0, sizeof(PN512_RF_WorkInfo.aucBuffer));
	//felica的同步信号0x00,0x00,0x00,0x00,0x00,0x00+0xb2,0x4d已经配置好FelNFC1Reg,PN512会自动添加 
	memcpy((uchar*)PN512_RF_WorkInfo.aucBuffer,paucCmd,uiCmdLen);//需要发送的命令
	
	ucRet = PN512_s_RFExchangeCmd(PN512_TRANSCEIVE_CMD);
	if (ucRet == RET_RF_OK)
	{
		*puiRspLen = PN512_RF_WorkInfo.ulBytesReceived;
		memcpy(paucResp, (uchar*)PN512_RF_WorkInfo.aucBuffer,PN512_RF_WorkInfo.ulBytesReceived);//拷贝卡片回送数据
	}
	else
	{
		paucResp[0] = PN512_RF_WorkInfo.usErrNo & 0xFF;
		paucResp[1] = PN512_RF_WorkInfo.usErrNo >> 8;
		*puiRspLen = 2;
	}
	return ucRet;
}

