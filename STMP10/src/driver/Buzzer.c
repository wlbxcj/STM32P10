#include "Buzzer.h"
#include "Toolkit.h"

//void Buzzer_Init(void)
void Buzzer_Init(u16 nFre)
{
  /* Peripherals InitStructure define */
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef        TIM_OCInitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  //TIM_Cmd(TIM3, ENABLE); //12/10/28
  TIM_DeInit(TIM4);
  
  TIM_OCStructInit(&TIM_OCInitStructure);  
  /* Configure PB.00 buzzer output (TIM3_OC3) */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;//TIM4的TIM_Channel_3的PWM输出为GPIO_Pin_8
  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  //GPIO_ResetBits(GPIOB, GPIO_Pin_0);//CLEAR PB.0 PIN TO 0 
  //GPIO_ResetBits(GPIOB, GPIO_Pin_8);//CLEAR PB.0 PIN TO 0 
  //GPIO_SetBits(GPIOB, GPIO_Pin_8);//CLEAR PB.0 PIN TO 0 

  
  // TIM3 used for PWM genration 
  TIM_TimeBaseStructure.TIM_Prescaler = 0x47; // TIM3CLK = 1 MHz 72000000/1000000-1
  //TIM_TimeBaseStructure.TIM_Period = 0x013D;   // PWM frequency : 3.15KHz ARR =1000000/317
  TIM_TimeBaseStructure.TIM_Period = 1000000/nFre;   // PWM frequency : 3.15KHz ARR
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  // TIM3's Channel3 in PWM1 mode 
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_Channel = TIM_Channel_3;
  //TIM_OCInitStructure.TIM_Channel = TIM_Channel_1;
  
  //TIM_OCInitStructure.TIM_Pulse = 0x9E;  // Duty cycle: 50%  CRR3 捕获翻转后TO ARR
  //TIM_OCInitStructure.TIM_Pulse = 0x50;  // Duty cycle: 50%  CRR3 捕获翻转后TO ARR
  //TIM_OCInitStructure.TIM_Pulse = nFre/2;  // Duty cycle: 50%  CRR3 捕获翻转后TO ARR
  TIM_OCInitStructure.TIM_Pulse = 1000000/nFre/2;  // Duty cycle: 50%  CRR3 捕获翻转后TO ARR
  
  
  TIM_OCInit(TIM4, &TIM_OCInitStructure);
  //TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);  
  TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);  
 
  
  // Disable TIM3 
  //TIM_Cmd(TIM3, DISABLE); //CR1'S CEN TO 0 
  TIM_Cmd(TIM4, ENABLE); //

  //GPIO_SetBits(GPIOB, GPIO_Pin_8); //12/10/18
  
#if 0  
  //GPIO_ResetBits(GPIOB, GPIO_Pin_0);  
  GPIO_ResetBits(GPIOB, GPIO_Pin_8);
#endif
  
  //TIM_CtrlPWMOutputs(TIM3,ENABLE);				   //使能定时器3的PWM输出

    
}

/* FALSE : ring */
void Buzzer_Control(bool Control)
{
    if (Control)
        //GPIO_SetBits(GPIOB, GPIO_Pin_0);
        GPIO_SetBits(GPIOB, GPIO_Pin_8);
    else
        //GPIO_ResetBits(GPIOB, GPIO_Pin_0);
        GPIO_ResetBits(GPIOB, GPIO_Pin_8);
}

void Buzzer_Ring(u16 Fre)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  u8 i;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB, GPIO_Pin_8);
  //test

  for(i=0;i<100;i++)
  {
    GPIO_ResetBits(GPIOB, GPIO_Pin_8);
    Lib_DelayMs(1);
  GPIO_SetBits(GPIOB, GPIO_Pin_8);
    Lib_DelayMs(1);
    
    
  }
  
}

void Buzzer_Off(void)
{
    TIM_Cmd(TIM4, DISABLE); //
  
#if 0  
  GPIO_InitTypeDef GPIO_InitStructure;
  //for kf311
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOB, GPIO_Pin_8);
#endif
  
}


void   Lib_Beef(unsigned char mode,int DlyTimeMs)
{
    static int BeepFreqTab[8]={0,100,200,400,800,1600,2000,2500}; 
    int nFrequency;
    
    if (0 >= DlyTimeMs)
    {
        return ;
    }  
    nFrequency = BeepFreqTab[mode%8]+500;
    //nFrequency = BeepFreqTab[mode%8];
    //Buzzer_Ring(nFrequency);
    Buzzer_Init(nFrequency);
    delay_ms(DlyTimeMs);
    TIM_Cmd(TIM4, DISABLE); //

}

void  Lib_Beep(void)
{
    Lib_Beef(3, 100);//100
}

