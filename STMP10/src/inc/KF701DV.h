
/*  引脚定义     */
//sbit SIO1	=P1^2 ;  //ID卡IO引脚
//sbit SCLK1	=P1^7  ; //ID卡同步时钟SCLK引脚
//sbit VPP	=P3^3 ; //ID卡VPP引脚
//sbit FUSE	=P1^6 ; //ID卡FUSE引脚
//sbit PGM	=P1^3 ; //ID卡PGM引脚     
//sbit SIO2	=P1^5 ; //SAM卡IO引脚
//
//sbit SIO	=P3^2;
//sbit SCLK	=P3^4;
//
//sbit AB23	=P4^0;//HXW V1.1 2004-10-9 
//sbit AB24	=P4^1;//HXW V1.1 2004-10-9

/* 库里用到的全局变量  */
//uchar   bdata  r_w_byte;
//sbit    HBIT=r_w_byte^7;
//sbit    LBIT=r_w_byte^0;
//
//uchar bdata wrdata;
//sbit  LWR=wrdata^0;
//sbit  HWR=wrdata^7;

//cpu card
//bit     logic_mode_id;     //大卡座cpu卡的全局位变量,正向反向逻辑约定
//bit     logic_mode_sam;     //小卡座cpu卡的全局位变量
//bit     re_TS_f;
 uint    g_ui_wait;     //定时器T0用到的全局变量,每中断一次减1,直到0
 uchar   re_wait_time, guard_time,TIME;
 uint  G_UI_TIME;    //等待时间开关，0时关闭定时中断，否则定时时间为5ms×G_UI_TIME
 uint   HAVE_SEND_WT;  //是否发送了WT命令，根据它可以在发送OK时判断是否需要延时。

//volatile uchar xdata HC574A_PORT _at_ 0x8000;
//volatile uchar xdata HC574B_PORT _at_ 0x9000;
//
//uchar bdata hc574a_byte;
//sbit AB15	=hc574a_byte^0;
//sbit AB16	=hc574a_byte^1;
//sbit AB17	=hc574a_byte^2;
//sbit AB18	=hc574a_byte^3;
//sbit AB19	=hc574a_byte^4;
//sbit AB20	=hc574a_byte^5;
//HXW V1.1 2004-10-9 sbit BEEP	=hc574a_byte^6;
//HXW V1.1 2004-10-9 sbit LED	=hc574a_byte^7;
//sbit AB21	=hc574a_byte^6;//HXW V1.1 2004-10-9 
//sbit AB22	=hc574a_byte^7;//HXW V1.1 2004-10-9
//
//uchar bdata hc574b_byte;
//sbit ICC_PWR1	=hc574b_byte^0;
////HXW V1.1 2004-10-9 sbit ICC_PWR2	=hc574b_byte^1;
//sbit BEEP	=hc574b_byte^1;//HXW V1.1 2004-10-9
//sbit ICC_RST1	=hc574b_byte^2;
//sbit ICC_RST2	=hc574b_byte^3;
//sbit ICC1_SEL1	=hc574b_byte^4;		//ID卡时钟信号的开启端,0--ON,1--OFF
//sbit ICC1_SEL0	=hc574b_byte^5;		//ID卡时钟信号的选择端，0--异步时钟，1--同步时钟
//sbit ICC2_SEL1	=hc574b_byte^6;		//SAM卡时钟信号的开启端,0--ON,1--OFF
//sbit LED	=hc574b_byte^7;//HXW V1.1 2004-10-9
//


