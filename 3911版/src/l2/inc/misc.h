/*******************************************************************************
* File Name	     : misc.h
* Author             : guohonglv
* Date First Issued  : 2010/11/12
* Description        : 完全引用qpboctest项目的头文件 
* 注意：             ：这里的宏定义OK改了，因为其他的地方用到，但宏定义的值不一样..
********************************************************************************/
#ifndef MISC_H
#define MISC_H

#define TAXPOS_M
//#define EFTPOS_M
#define EMVTEST_M
#define _EMV_TEST_VERSION

#define KF900_MACHINE
//#define KF600_MACHINE

#define QPBOC_M
//#define PAYWAVE_M

//#define PTDDHD_M
#define KF311_M
//#define KF322_M

//13/08/07
#define AS3911_M

#define DEBUG

#define EMVWORK_M


//13/06/27
#define FIRMWARE_MAJOR_VERSION  '2'
#define FIRMWARE_MINOR_VERSION  '1'

#define MAXDATADICTNUM             118

#define MAX_RECNUM_M              290

#define  WORK_STATUS         '0'
#define  TEST_STATUS         '1'
#define  BATCH_STATUS		 '2'

#define MAX_TRANSLOG_M            32 //20090522 add
#define EMV_OPTIONS(sOptions,bOffset)   ( ( (*(sOptions+(bOffset-1)/8 ) ) >> ((bOffset-1)%8) )  &0x01 )
#define EMV_ENABLE_OPTIONS(sOptions,bOffset) (	*(sOptions+(bOffset-1)/8 ) |= 0x01<<((bOffset-1)%8) ) 
#define EMV_DISABLE_OPTIONS(sOptions,bOffset) (	*(sOptions+(bOffset-1)/8 ) &= 0xfe<<((bOffset-1)%8) ) 

//#define EMV_SET_OPTIONS(sOptions,bOffset) 
#define OFFSET_COMMON_CHAR_M   		     						3
#define OFFSET_Multi_LANG_M			     						4
#define OFFSET_Partial_AID_M		     						5
#define OFFSET_Prefer_ORDER_M			 						6
#define OFFSET_Cardholder_CONFIRM_M 	 						7
#define OFFSET_PSE_SELET_M 	        	 						8


#define OFFSET_amount_KNOWN_BEFORE_CVM_M 						11
#define OFFSET_FAILCVM_M				 						12
#define OFFSET_GetDataPINTC_M			 						13
#define OFFSET_PINByPass_M				 						14
#define OFFSET_default_DDOL_M			 						15
#define OFFSET_revocation_ISSUER_PK_M	 						16


#define OFFSET_TerminalRisk_BasedOnAip_M 						19
#define OFFSET_Exception_File_M			 						20
#define OFFSET_TransLog_M				 						21
#define OFFSET_Velocity_Check_M			 						22
#define OFFSET_RandomTrans_SELECT_M		 						23
#define OFFSET_Floor_LimitCheck_M		 	    				24


#define OFFSET_CDA_requestOrnever_2ndGenerateAC_M 				27
#define OFFSET_CDA_requestOrnever_1ndGenerateAC_M				28
#define OFFSET_Detect_CDA_Fail_BeforeTAAnalysis_M   			29
#define OFFSET_SkipOrprocTACOrIAC_defaultForUnableToGoOnline_M 	30
#define OFFSET_Process_DefaultAC_PriorOrAfter_1ndGenerateAC_M	31
#define OFFSET_Terminal_Action_Codes_M							32

#define OFFSET_DefaultTDOL_M									34
#define OFFSET_OnlineCapture_M									35 //1:	OnlineCapture 0:BatchCapture
//#define OFFSET_BatchCapture_M									36 //
#define OFFSET_IssuerReferral_M									37
#define OFFSET_Advice_M											38
#define OFFSET_Force_Acceptance_M								39
#define OFFSET_ForceOnline_M									40

#define OFFSET_Internate_Date_Management_M						42
#define OFFSET_issuer_script_GreaterThan128_M					43
#define OFFSET_AccountType_Select_M								44
#define OFFSET_MagstripeReader_First_M							45
#define OFFSET_ICCAndMagstripe_Reader_Combined_M				46
#define OFFSET_AmountAndPIN_enter_AtTheSame_Keypad_M			47
#define OFFSET_Equipped_With_PINPAD_M							48

//#define FOR_LINUX_SYS

#ifdef FOR_LINUX_SYS



#define TERM_BATCH_FILE_NAME	"./data/emv/batch.log"
#define BATCH_RECORD_LEN		549 //add type 541->549

#define BATCH_MAX_RECORD        300
#define TERM_ADVICE_FILE_NAME	"./data/emv/advice.log"
#define ADVICE_RECORD_LEN       549 //与batch 是一样,只是message type不一样.add type

#define TERM_LOG_FILE_NAME	    "./data/emv/trans.log"
#else
#define TERM_BATCH_FILE_NAME	".\\data\\emv\\batch.log"
#define BATCH_RECORD_LEN		549 //add type 541->549

#define BATCH_MAX_RECORD        300
#define TERM_ADVICE_FILE_NAME	".\\data\\emv\\advice.log"
#define ADVICE_RECORD_LEN       549 //与batch 是一样,只是message type不一样.add type

#define TERM_LOG_FILE_NAME	    ".\\data\\emv\\trans.log"


#endif

#define LOG_RECORD_LEN		sizeof(T_EMVTRANSREC) //	132




#define OFFSET_AMEX_M											66
#define OFFSET_JCB_M											67
#define OFFSET_VISANET_M										68
#define OFFSET_VSDC_M											69
#define OFFSET_ETEC_M											70
#define OFFSET_PBOC_M											71


/*
unsigned char Options(unsigned char *sOptions,unsigned char bOffset)
{
	return ( (*(sOptions+(bOffset-1)/8 )>> ((bOffset-1)%8) ) &0x01 );

}
*/

//FILE
#ifdef FOR_LINUX_SYS

#define FILE_CTRL_M      		"./data/emv/ctrl.ini"
//#define  TERM_LOG_FILE_NAME		"pos.log"
#define  TERM_REV_FILE_NAME		"./data/emv/rev.log"   //冲正
#define  REVERSAL_FILE			"./data/emv/dup_file"  
//#define  TERM_AIDLIST			"termaid.lst"	// tommy for CUP ,在T_EMV_APP_PARAMETER中已加onlinePIN 09/06/29
#define  ICC_SCRIPT_FILE		"./data/emv/iccscript"		// POS_COM tommy for ICC
#define  FILE_POS_PARAM_M       "./data/emv/posparam.ini"

#define  FIEL_EXCEPTION_M	    "./data/emv/exception"  

#define EMV_REVOCLIST_FILE_M   "./data/emv/emvrevoclist"


#define  FILE_TRACE_DEBUG_M     "./data/emv/trace.txt"

#define  FILE_TERMAPP_M 		"./data/emv/termAppFile"   //非linux set".//data//emv"?

#define QPBOC_EXCEPTION_FILE_M	 "./data/emv/qpbocexception"  

#define EMV_TERM_CAPK_FILE_M   "./data/emv/emvtermcapk" 

#else

#define FILE_CTRL_M      		".\\data\\emv\\ctrl.ini"
//#define  TERM_LOG_FILE_NAME		"pos.log"
#define  TERM_REV_FILE_NAME		".\\data\\emv\\rev.log"   //冲正
#define  REVERSAL_FILE			".\\data\\emv\\dup_file"  
//#define  TERM_AIDLIST			"termaid.lst"	// tommy for CUP ,在T_EMV_APP_PARAMETER中已加onlinePIN 09/06/29
#define  ICC_SCRIPT_FILE		".\\data\\emv\\iccscript"		// POS_COM tommy for ICC
#define  FILE_POS_PARAM_M       ".\\data\\emv\\posparam.ini"

#define  FIEL_EXCEPTION_M	    ".\\data\\emv\\exception"  

#define EMV_REVOCLIST_FILE_M   ".\\data\\emv\\emvrevoclist"


#define  FILE_TRACE_DEBUG_M     ".\\data\\emv\\trace.txt"

#define  FILE_TERMAPP_M 		".\\data\\emv\\termAppFile"   //非linux set".//data//emv"?

#define QPBOC_EXCEPTION_FILE_M	 ".\\data\\emv\\qpbocexception"  

#define EMV_TERM_CAPK_FILE_M   ".\\data\\emv\\emvtermcapk" 

#endif

#define TPDU_LEN_M      		11
#define MSGTYPE_LEN_M   		5
#define BITMAP_LEN_M    		8
#define PAN_LEN_M       		22
#define PROCCODE_LEN_M  		7
#define AMOUNT_LEN_M   		13
#define SYSTRACE_LEN_M  		7
#define TIME_LEN_M      		7
#define DATE_LEN_M            9
#define EXPDATE_LEN_M   		5
#define ENTRYMODE_LEN_M 		4
#define NII_LEN_M       		5
#define VERSION_LEN_M         3
#define TRACK2_LEN_M    		38
#define TRACK3_LEN_M          105
#define REFNO_LEN_M     		13 			    
#define AUTHCODE_LEN_M  		7
#define RESPCODE_LEN_M  		3
#define TERMINAL_LEN_M  		9
#define MERCHANT_LEN_M  		16
#define MONEYCODE_LEN_M 		4
#define PINDATA_LEN_M       	17
#define BALAMOUNT_LEN_M     	30
#define ICC_DATA_LEN_M			500
#define PRIVATEUSE1_LEN_M     160  /* 20 */
#define PRIVATEUSE2_LEN_M   	31
#define PRIVATEUSE62_LEN_M    30   /* 20 */
#define PRIVATEUSE3_LEN_M   	61

#define FORADD_M              0
#define BACKADD_M             1

#define MSGMAC_LEN_M   		17

#define BCD_M          		0x01
#define ASC_M               0x02
#define BIN_M               0x04
#define LLVAR_M	    		0x10
#define LLLVAR_M      		0x20
#define NUMBERIC_M     		0x40
#define STRING_M      		0x80

/*输入方式*/
#define ALPHA_INPUT	0				/*输入字母*/
#define PASS_INPUT	1				/*输入口令*/
#define NUM_INPUT	2				/*输入数字*/
#define AMOUNT_INPUT	3			/*输入金额*/
#define MAX_ALPHA       5

#define  OK_MIS           0  //guohonglv
#undef  NULL
#define NULL 0

//#define TRUE  1
//#define FALSE 0

#ifdef TAXPOS_M
	#define ENTER SUM
	#define UP    DATE
	#define DOWN  ROLLPAPER
	#define ALPHA SUBTOTAL  //小计
#endif

#ifdef TAXPOS_M
	#define MAX_LINE_M  4 //8->4
	#define MAX_COL_M   16 //24->16
#else
	#define MAX_LINE_M  4 //8->4
	#define MAX_COL_M   16 //24->16
#endif



//for disp

#define LEFT   0xF0
#define MIDDLE 0xF1
#define RIGHT  0xF2
#define CLS_LINE	0x01				   
#define UNCLS_LINE	0x00

#define UNCLEAR_LCD 0		//不清屏
#define CLEAR_LCD 1			//清屏标志

#define MAX_NUM_PER_LINE	16 //最大每行显示字节数 30->20->16
#define MAX_SCR_LINE_NUM    8  //最大每屏行数

#define REC_APP_M                1
#define REC_CAPK_M               2
#define REC_EMV_PARA_M           3
#define REC_EMV_REVOCATIONLIST_M 4
#define REC_EXCEPTION_M			 5   //在应用中处理

#define DE55_MUST_SET	1	// 必须存在
#define DE55_OPT_SET	2	// 可选择存在
#define DE55_COND_SET	3	// 根据条件存在

#define  PARAM_OPEN		'1'
#define  PARAM_CLOSE	'0'

#define  E_FILE_OPEN	18 /*打开文件错*/
#define  E_FILE_SEEK	19 /*定位文件错*/
#define  E_FILE_READ	20 /*读文件错*/ 
#define  E_FILE_WRITE	21 /*写文件错*/

#define  NO_DISP          36

#define  E_NEED_FALLBACK	51 /*需要FALLBACK*/
#define  E_NEED_INSERT		52 // 需要插卡交易
#define  E_EMV_REVERSAL		53	// 需要IC卡冲正

//EMV return 
#define E_EMV_DOWNPARA_M    0xF0

#define NO_SWIPE_INSERT		0x00		// 没有刷卡/插卡
#define  E_ERR_SWIPE      12  /*刷卡错误*/

#define CARD_SWIPED			0x01	// 刷卡
#define CARD_INSERTED		0x02	// 插卡
#define CARD_KEYIN			0x04	// 手输卡号
#define FALLBACK_SWIPED		0x08	// 刷卡(FALLBACK)
#define	MODE_PIN_INPUT			0x10	// online PIN input
#define MODE_OFF_PIN			0x20	// for AMEX
#define CONTACTLESS_ENTER       0x40    

#define ICC_USER	0


#define  E_TRANS_CANCEL   31    /*交易被取消  updated 2008/02/27 jiaxf*/
#define  E_TRANS_FAIL     2    /*交易失败    */   
#define  E_NO_TRANS       3   /* 无交易*/

#define  E_MEM_ERR        13  /*文件操作失败*/

#define TRAN_AMOUNT		0


#define  ERR_INFO_NUM  	65  /*返回码个数*/


#define  TRACK2_LEN        100 
#define  TRACK3_LEN        107

#define LEN_POS_STAN			6

#define ENC_M 1
#define DEC_M 0

#define APPROVE_M   0
#define DECLINE_M   1

#define BIN      0x01
#define ANS		 0x02
#define AN		 0x04
#define CN       0x08
#define NUM	     0x10

#define ONLINE_M    0xc8

#define AppListStoredNum  11

#define	CMD_CODE_MAG_SWIPE	  0xA0	//swipe mag card
#define	CMD_CODE_ICC_INSERT	  0xA1	//insert icc card

#define WAITTIMEFORDETECT     500  //500 of 10ms

#define printf trace_debug_printf

#endif
