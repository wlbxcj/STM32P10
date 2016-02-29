#ifndef _UARTPROTOCOL_H_
#define _UARTPROTOCOL_H_

#define COMM_BANKUNION

#ifdef COMM_BANKUNION

#define POSPICC_INIT_M     		0
#define POSPICC_POLL_M     		1
#define POSPICC_NORMAL_M		2
#define POSPICC_DEBUG_M        		3
#define POSPICC_PAY_M        		4
#define POSPICC_MAGAGER_M      		5

//command code
#define CMD_POLL_POLL	        		0x07  //POLL
#define CMD_POLL_ECHO	        		0x08  //echo

#define CMD_DEBUG_ENTER	        		0x10  //设置调试和优化报文模式
#define CMD_DEBUG_SETPARAMETER			0x12  //设置参数

#define CMD_AUTH_COMM_INIT      		0x20  //通讯初始化
#define CMD_AUTH_BOTH           		0x21  //双向认证
#define CMD_AUTH_KEY_GENERATE                   0x22  //密钥生成
#define CMD_AUTH_DISABLE_READER                 0x23  //屏蔽读卡器

#define CMD_TRAN_READYFORPAY    		0x30  //准备支付交易
#define CMD_TRAN_RESET          		0x31  //复位交易
#define CMD_TRAN_DISP_STATUS    		0x32  //显示状态
#define CMD_TRAN_ONLINE         		0x6A  //交易联机后处理
#define CMD_TRAN_UPCARD					0x6B  //UPCARD

#define CMD_MANAGER_ENTER       		0x40  //进入管理模式
#define CMD_MANAGER_GET_CAPA    		0x41  //获取性能
#define CMD_MANAGER_SET_CAPA    		0x42  //设置性能
#define CMD_MANAGER_GET_TIMEDATE		0x43  //获取时间和日期
#define CMD_MANAGER_SET_TIMEDATE		0x44  //设置时间和日期
#define CMD_MANAGER_GET_PARAMETER   	0x45  //获取参数
#define CMD_MANAGER_GET_COMM_BPS    	0x52  //获取串口通信速率
#define CMD_MANAGER_SET_COMM_BPS    	0x53  //设置串口通信速率
#define CMD_MANAGER_RESET_ACQUIRE_KEY	0x54  //重置收单行密钥
#define CMD_MANAGER_RESTORE_READER      0x55  //恢复读卡器
#define CMD_MANAGER_GET_TAG 		    0x56  //获取PBOC标签值
#define CMD_MANAGER_SET_TAG				0x57  //设置PBOC标签值
#define CMD_MANAGER_GET_DISP  			0x58  //获取显示信息
#define CMD_MANAGER_SET_DISP  			0x59  //设置显示信息
#define CMD_MANAGER_GET_CVM				0x5A  //获取CVM性能
#define CMD_MANAGER_SET_CVM				0x5B  //设置CVM性能
#define CMD_MANAGER_SET_CAPK            0x61  //设置PBOC公钥
#define CMD_MANAGER_FIND_CAPK			0x62  //查询PBOC公钥
#define CMD_MANAGER_SET_REVOCLIST       0x63  //设置回收公钥证书
#define CMD_MANAGER_FIND_REVOCLIST      0x64  //查询回收公钥证书
#define CMD_MANAGER_SET_EXCEPTIONLIST   0x65  //设置黑名单
#define CMD_MANAGER_FIND_EXCEPTIONLIST  0x66  //查询黑名单
#define CMD_MANAGER_SET_PBOC_PARAMETER  0x67  //设置PBOC固定参数
#define CMD_MANAGER_SET_AID				0x68  //设置AID相关参数

//RFU
#define CMD_MANAGER_SET_ALL_AID			0x80  //设置所有AID相关参数

//respone code

#define RC_SUCCESS             	0x00  	  //通用应答码指示读卡器成功执行请求命令                   
#define RC_DATA                	0x01  	  //读卡器从非接触卡片获得的数据有效，用来启动交易         
#define RC_POLL_A              	0x02  	  //读卡器确认应答。终端和读卡器之间已双向认证             
#define RC_POLL_P              	0x03  	  //读卡器确认应答。终端和读卡器未进行双向认证               
#define RC_SCHEME_SUPPORTED    	0x04  	  //读卡器支持该支付方案                                       
#define RC_SIGNATURE           	0x05  	  //需要签名                                                   
#define RC_ONLINE_PIN          	0x06  	  //需要联机PIN                                             
#define RC_OFFLINE_PIN         	0x07  	  //需要脱机PIN                                            
#define RC_SECOND_APPLICATION  	0x08  	  //提示读卡器支持第二个应用(非支付应用) 
//add 
#define RC_DECLINE              0x20      //拒绝交易
#define RC_FILE_ERR             0x21      //操作文件错误
#define RC_USE_CONTACT			0x22      //请使用接触式界面
#define RC_READER_ERR    		0x23       //射频模块故障
#define RC_EMV_NO_APP           0x24       //应用候选列表为空
#define RC_ICC_BLOCK			0x25	   //卡被锁
#define RC_EMV_APP_BLOCK		0x26	   //卡应用被锁
#define RC_TRAN_FAILURE			0x27	   //交易错误
#define RC_EXCEED_OFFLINEAMT	0x28	   //超过脱机限额
#define RC_ONLINE				0x29	   //在线交易
#define RC_TERMINATE 			0x2A	   //交易终止
#define RC_ICC_CMD_ERR			0x2B       //IC命令通讯错

#define RC_CARD_EXPIRED			0x2C       //过期卡

#define RC_FAILURE             	0xFF  	  //常规错误，请求报文存在错误                                  
#define RC_ACCESS_NOT_PERFORMED	0xFE  	  //打开管理模式的访问控制未执行                             
#define RC_ACCESS_FAILURE      	0xFD  	  //打开管理模式的访问控制错误                               
#define RC_AUTH_FAILURE       	0xFC  	  //双向认证失败                                           
#define RC_AUTH_NOT_PERFORMED  	0xFB  	  //未双向认证                                               
#define RC_DDA_AUTH_FAILURE    	0xFA  	  //DDA认证失败                                              
#define RC_INVALID_COMMAND     	0xF9  	  //命令码错误                                                  
#define RC_INVALID_DATA        	0xF8  	  //请求报文的数据域错误                                       
#define RC_INVALID_PARAM       	0xF7  	  //无此参数                                               
#define RC_INVALID_KEYINDEX    	0xF6  	  //在未产生Asession或AEK时，终端请求读卡器产生Msession密钥   
#define RC_INVALID_SCHEME      	0xF5  	  //读卡器不支持终端激活的方案                                
#define RC_MORE_CARDS          	0xF3  	  //多卡                                                       
#define RC_NO_CARD             	0xF2  	  //未出示非接触卡                                         
#define RC_NO_PBOC_TAGS        	0xF1  	  //读卡器不支持该标签                                          
#define RC_NO_PARAMETER        	0xF0  	  //无此参数                                                  
#define RC_POLL_N              	0xEF  	  //读卡器确认应答，读卡器未准备好                           
#define RC_Other_AP_CARDS      	0xEE  	  //其他亚太地区国家的Visa Wave卡                            
#define RC_US_CARDS            	0xED  	  //老版美国非接触卡                                             
#define RC_NO_PIN              	0xEC  	  //未输入PIN                                                   
#define RC_NO_SIG              	0xEB  	  //触摸屏未获得签名                                        

//parameter
#define P_MSG_TIMEOUT     0x0001   //毫秒级的超时时间，读卡器必须在缺省时间内对终端发送的报文（除准备支付交易报文）作出应答。默认的值为500ms或0x01F4
#define P_SALE_TIMEOUT    0x0002   //毫秒级的超时时间，是终端发送准备支付交易报文后的超时等待时间。缺省值为15000ms或0x3A98
#define P_POLL_MSG        0x0003   //终端发送下一个POLL报文已检测读卡器是否存在的秒级超时时间。默认的值为30s或0x001E
#define P_BUF_TIMEOUT     0x0004   //若终端没有响应读卡器的准备支付交易报文的应答报文，这个参数指示读卡器清除缓冲区的等待时间。缺省值为5000ms或0x1388
#define P_ENCRYPTION      0x0005   //0x00 TDEA已禁止，在终端和读卡器之间都是以明文的形式发送数据，只用于测试和调试。?	0x01 TDEA已开启
#define P_DISPLAY         0x0006   //0x00 终端应显示所有读卡器的应答码，只用于测试和调式.	0x01 终端应将读卡器的应答码转变成提示信息并显示出来
#define P_MAX_BUF_SIZE    0x0007   //读卡器给请求报文和命令报文分配的最大的缓冲区。缺省值为1024字节或0x0400
#define P_DOUBLE_DIP      0x0008   //该值为读卡器允许同一张卡片连续进行两次交易之间的间隔时间.缺省值为5000ms或0x1388
#define P_READER_INDEX    0x0009   //读卡器索引号
#define P_LANGUAGE        0x000A   //该参数指出了读卡器支持的语言类型和显示信息的语言类型.标签 5F2D + 长度 + 语言类型 + 状态(01：激活，00: 禁止
#define P_DISPLAY_S_MSG   0x000B   //显示短提示信息的毫秒级超时时间 缺省值为2000ms或0x07D0
#define P_DISPLAY_L_MSG   0x000C   //显示长提示信息的毫秒级超时时间.缺省值为5000ms或0x1388
#define P_DISPLAY_SS_MSG  0x000D   //在触摸屏上签名的毫秒级超时时间.缺省值为10000ms或0x2710
#define P_DISPLAY_SR_MSG  0x000E   //在收据上签字的毫秒级超时时间.缺省值为5000ms或0x1388
#define P_DISPLAY_PIN_MSG 0x000F   //PIN输入的毫秒级超时等待时间.缺省值为10000ms或0x2710
#define P_DISPLAY_E_MSG   0x0010   //显示错误信息的毫秒级超时时间.缺省值为3000ms或0x0BB8


#endif

unsigned char GetXorCheck(unsigned char *, unsigned int );
//s16 PortRecvPack(unsigned char *);
unsigned short Protocol_Pro(unsigned char* ,signed short );
unsigned short Uart_Protocol(void);

#endif
