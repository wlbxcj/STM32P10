/*
 * Author: 
 *  Owner: KaiFa Technology Corporation.
 *   Date: 2008.01.02
 */
 
#ifndef SCC_SUPPORT_H
#define SCC_SUPPORT_H


#define BUF_LEN         300 //512->300
#define MAX_BUF_LEN     512
#define MAX_CARD_NUM    6

#define VCC_3V 					0
#define VCC_1_8V 				1
#define VCC_5V 					2

extern unsigned char   slotnumber;

#pragma pack(1)
struct sc_s {
   int           id;            // SC interface number (0-4)
   int	         fd;            // File handle for SC device ( e.g. "/dev/sc0")
} ;
extern struct sc_s scs[6];

struct scc_cmd
{
    unsigned char   data[BUF_LEN];
    unsigned int   len;	// Length of "data" in byte
    unsigned char   name[64];
};
extern struct scc_cmd packed;
#pragma pack() 

//extern int      scc_filedesc;	// file descriptor for POSIX standard I/O operation 

//函 数 名: int      scc_init(void)
//功    能:	打开dev file
//输入参数:	无
//输出参数: 
//返 回 值:	成功返回0,失败返回错误码
//注    释:     程序启动时调用一次可以




extern int      scc_init(void);	/* SmartCard initialization */
//函 数 名: void     scc_fini(void)
//功    能:	关闭dev file
//输入参数:	无
//输出参数: 
//返 回 值:	无
//注    释:     

extern void     scc_fini(void);

/*
//函 数 名: int      scc_select_slot(unsigned char slot)
//功    能:	选择IC卡的slot
//输入参数:	slot:0-6
                    0-用户卡
                    1~6 SAM卡
//输出参数:     
//返 回 值:	成功返回0,失败返回错误码
//注    释:     
*/
extern int      scc_select_slot(unsigned char slot);	/* Choose a slot to operate */

/*
//函 数 名: unsigned char inserted_card(void)
//功    能:	检查卡是否到位
//输入参数:	无

//输出参数:     
//返 回 值:	0:有卡插入
                1:无卡插入
//注    释:  仅对用户卡   
*/

extern unsigned char inserted_card(void);

/*
//函 数 名: int      scc_reset(unsigned char *voltage, unsigned char *atr, unsigned int *len)
//功    能:	卡上电复位
//输入参数: *voltage:电压值,范围0-2
                      0:3V
                      1:1.8V
                      2:5V
                    缺省为5V
//输出参数:     *atr:复位串,最大buffer长度为512
                *len:复位串的长度
//返 回 值:	成功返回0,失败返回错误码
                
//注    释:     
*/
extern int      scc_reset(unsigned char *voltage, unsigned char *atr, unsigned int *len);	/* Reset and get the ATR */

/*
//函 数 名: int SendCmdT0_APDU(struct scc_cmd *command, unsigned char *rbuf,unsigned int *rlen)
//功    能:	卡命令,支持T0,T1协议
//输入参数: *command:卡命令
//输出参数:     *rbuf:返回数据(SW1,SW2,Data)
                *len:返回数据长度
//返 回 值:	成功返回0,失败返回错误码
                
//注    释:  

struct scc_cmd
{
    unsigned char   data[BUF_LEN];//卡命令:case 1:CLA，INS，P1，P2
                                           case 2:CLA，INS，P1，P2,Le
                                           case 3:CLA，INS，P1，P2,LC Data
                                           case 4:CLA，INS，P1，P2,LC Data Le
                                           
    unsigned int   len;	// Length of "data" in byte
    unsigned char   name[64]; //可不用填入数据
}__attribute__ ((packed));


  举    例:

  	struct scc_cmd tIccCmd;
    
	unsigned char OutBuf[1024];
	unsigned int nLen;
	//memcpy(ApduSend.Cmd,"\x00\x24\x00\x00",4);
	
	memcpy(tIccCmd.data,"\x00\x24\x00\x00",4);
	tIccCmd.data[0] = cla;
	tIccCmd.data[3] = p2;
	tIccCmd.data[4] = lc;
	memcpy(&tIccCmd.data[5],pindata,lc);
	tIccCmd.len = lc+5;
	if(scc_command_APDU(&tIccCmd,OutBuf,&nLen)!=0)
		return 0xff;
		
	status[0] = OutBuf[0];
	status[1] = OutBuf[1];
   
*/
/*为了兼容，它已包括T1,它与scc_command_APDU一样，支持T0,T1协议*/
extern int SendCmdT0_APDU(struct scc_cmd *command, unsigned char *rbuf,unsigned int *rlen);

/*
//函 数 名: int scc_command_APDU(struct scc_cmd *command, unsigned char *rbuf, unsigned int *rlen)
//功    能:	卡命令,支持T0,T1协议,等同SendCmdT0_APDU
*/
extern int scc_command_APDU(struct scc_cmd *command, unsigned char *rbuf, unsigned int *rlen);

/*
//函 数 名: int scc_close(void)
//功    能:	卡下电,关闭当前所选slot(scc_select_slot)
//输入参数:     无
//输出参数:     无
                
//返 回 值:	无
                
//注    释:     成功返回0,失败返回错误码
*/
extern int scc_close(void);

#endif /*  SCC_SUPPORT_H  */
