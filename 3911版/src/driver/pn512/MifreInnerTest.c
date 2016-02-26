#include "Mifre_TmDef.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

extern void debug_printf(char * str ,...);

#define  s_RF_Init	PN512_s_RF_Init
#define  s_vRFWriteReg  PN512_s_vRFWriteReg
#define  s_vRFReadReg   PN512_s_vRFReadReg

extern void ledall_off(void);
extern void ledall_on(void);
extern void ledred_on(void);
extern void ledred_off(void);
extern void ledgreen_on(void);
extern void ledgreen_off(void);
extern void Buzzer_Off(void);
extern void Buzzer_Ring(unsigned int Fre);

//test RC531
//#undef  PN512_GSNON_REG
//#define PN512_GSNON_REG   0x15
void RF_test_spi(void)
{
  uchar ucTemp = 0x00;
  uchar buf[10];
  uchar buftwo[10];
  ulong ulLoop = 0;
  ulong ulOK = 0;
  ulong ulFail = 0;
  unsigned long r1=0,e1=0;
  s_RF_Init();
  ledall_off();
	while(1)
	{
          ulLoop = 0;
          ulOK = 0;
          ulFail = 0;		
            while(1)
            {
                    ulLoop++; 	 
                    buf[0] = (uchar)(ulLoop & 0x0FF);//0x62;
                    ucTemp=buf[0];
			ucTemp+=0x04;
                    s_vRFWriteReg(1, PN512_GSNON_REG, buf);
		    s_vRFWriteReg(1, PN512_GSNON_REG+1, &ucTemp);
                    //s_vRFWriteReg(1, PN512_GSNON_REG+2, &ucTemp);
                    
                    s_vRFReadReg(1, PN512_GSNON_REG, buftwo);
                    
                    
                    if( !memcmp(buftwo,buf,1))
                    {
                            r1++;
                            //if(ulLoop==1)
                            {
                            	
                                ledred_off();
                                ledgreen_on();
                                debug_printf(0,0,0,"wr_rd_OK!\n");
                                debug_printf(0,0,0,"Dt: wr[%02X] rd[%02X] ", buf[0], buftwo[0]);
                            }
                            				
                    
                    }
                    else
                    {
                            e1++;
                            //if(ulLoop==1)
                            {
                              ledred_on();
			      ledgreen_off();
                              debug_printf(0,0,0,"wr_rd_err!\n");
                              debug_printf(0,0,0,"Dt: wr[%02X] rd[%02X] ", buf[0], buftwo[0]);
                            }
                    }
                    debug_printf(0,0,0,"Loop[%d],R[%d],E[%d]\n ",ulLoop,r1,e1);	
                    
                    delay_ms(50);
                    ledall_off();
                    delay_ms(50);
            }
	}

}

void RF_TypeAB_test(unsigned char which_api)
{
	unsigned char ret;
	uchar ucMode=0;
	uchar CardType;
	uchar SerialInfo[50];
	uchar Other[50];
	APDU_SEND  ApduSend;
	APDU_RESP  ApduRecv;
        long total=0,r1=0,e1=0,r2=0,e2=0,r3=0,e3=0;
	uchar bInitFlag=0;//12/09/21
	memcpy(ApduSend.Command,"\x00\xA4\x04\x00",4);
	ApduSend.Lc = 0x0E;
	memset(ApduSend.DataIn,0,sizeof(ApduSend.DataIn));
	//strcpy((char *)ApduSend.DataIn,(char *)"1PAY.SYS.DDF01");
	strcpy((char *)ApduSend.DataIn,(char *)"2PAY.SYS.DDF01");
	ApduSend.Le = 256;
	
	ledall_off();
	while(1)
	{
		total++;
		ret=PiccOpen();
		if(ret)
		{
                  e1++;           
                  debug_printf(0,0,0,"PiccOpen fail !,ret:%02x\n",ret);
                  goto END;
		}
		else
		{
			r1++;
			//ledred_off();
			//ledgreen_on();
			debug_printf(0,0,0,"PiccOpen OK !\n");
		}
                
		memset(SerialInfo,0,sizeof(SerialInfo));
		memset(Other,0,sizeof(Other));	
while(1)//12/09/21   
{
		ret=PiccDetect(ucMode,&CardType,SerialInfo,NULL,Other);
		if(ret)
		{
				e2++;
				ledred_on();
				ledgreen_off();
				debug_printf(0,0,0,"PiccDetect fail !,ret:%02x\n",ret);
				debug_printf(0,0,0,"Other:{%02x,%02x,%02x,%02x}\n",
							Other[0],Other[1],Other[2],Other[3]);
				//goto END;//12/09/21
		}
		else
		{
				r2++;
				ledred_off();
				ledgreen_on();
				debug_printf(0,0,0,"PiccDetect OK !\n");
				debug_printf(0,0,0,"CardType:[%c]\n",CardType);
				debug_printf(0,0,0,"SerialLen:%d,SerialNo:{%02x,%02x,%02x,%02x}\n",
							SerialInfo[0],SerialInfo[1],SerialInfo[2],SerialInfo[3],SerialInfo[4]);
                                break; //12/09/21
		}
      }	//12/09/21	
		memset(&ApduRecv,0,sizeof(ApduRecv));
		ret = PiccIsoCommand(0, &ApduSend, &ApduRecv);
		if(ret)
		{
				e3++;
				debug_printf(0,0,0,"PiccIsoCommand fail !,ret:%02X\n",ret);
				goto END;
		}
		else
		{
                                Buzzer_Ring(4200); 
				r3++;
				debug_printf(0,0,0,"PiccIsoCommand OK !\n");
				debug_printf(0,0,0,"SW:[%02X / %02X],LEN:%d",ApduRecv.SWA,ApduRecv.SWB,ApduRecv.LenOut);
				debug_printf(0,0,0,"RSP:[%02X %02X %02X %02X %02X]",ApduRecv.DataOut[0],
				ApduRecv.DataOut[1],ApduRecv.DataOut[2],ApduRecv.DataOut[3],ApduRecv.DataOut[4]);
		}
		
		END:
		PiccClose();
                bInitFlag = 0;
		debug_printf(0,0,0,"T[%d],P[%d/%d],D[%d/%d],I[%d/%d]\n",
						total,r1,e1,r2,e2,r3,e3);
                delay_ms(50);
                ledall_off();
                Buzzer_Off();
                //delay_ms(50);
                delay_ms(3000);
                
                
	}//end while(1)
			
}

void mifre_test(unsigned char test_case)
{
	switch(test_case)
	{
		case 1:
			debug_printf(0,0,0,"in case 1:spi_test\n");
			RF_test_spi();
			break;
		case 2:
			debug_printf(0,0,0,"in case 2:api_test\n");
			RF_TypeAB_test(1);
			break;
		case 3:
			
			break;	
	}	
}