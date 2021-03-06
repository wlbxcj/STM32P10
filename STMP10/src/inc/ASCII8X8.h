#include "Toolkit.h"

#if 0
u8  ASCII8X8[0x60*8] = 
{
/*0x00~0x1F*/
/*    special    char    */
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,    /*    space    */
        0x00,0x00,0x5F,0x00,0x00,0x00,0x00,0x00,    /*    !    1*/
        0x00,0x0A,0x06,0x00,0x00,0x0A,0x06,0x00,    /*    "    2*/
        0x00,0x20,0x74,0x2E,0x74,0x2E,0x04,0x00,    /*    #    3*/
        
        //0x00,0x00,0x24,0x4A,0xFF,0x52,0x24,0x00,    /*    $    4*/
        0x00,0x24,0x2A,0x7F,0x2A,0x12,0x00,0x00,
        //0x80,0x4E,0x2A,0x1E,0x78,0x54,0x72,0x01,    /*    %    5*/
        0x00,0x4E,0x2A,0x1E,0x78,0x54,0x72,0x00,
        0x00,0x70,0x8E,0xB9,0x56,0xB0,0x80,0x00,    /*    &    6*/
        0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,    /*    `    7*/
        
        /*    special    char    */
        //0x00,0x1C,0x22,0x41,0x41,0x00,0x00,0x00,    /*    (    8*/
        0x00,0x00,0x1C,0x22,0x41,0x00,0x00,0x00,
        //0x00,0x41,0x41,0x22,0x1C,0x00,0x00,0x00,    /*    )    9*/
        0x00,0x00,0x41,0x22,0x1C,0x00,0x00,0x00,
        0x00,0x24,0x18,0x7E,0x18,0x24,0x00,0x00,    /*    *    10*/
        0x00,0x08,0x08,0x3E,0x08,0x08,0x00,0x00,    /*    +    11*/
        
        0x00,0x00,0x40,0x58,0x38,0x00,0x00,0x00,    /*    ,    12*/
        0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x00,    /*    -    13*/
        0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00,    /*    .    14*/
        //0x00,0x40,0x20,0x18,0x06,0x01,0x00,0x00,    /*    /    15*/
        0x00,0x40,0x20,0x10,0x08,0x04,0x02,0x00,
        
        /*    number         */
        0x00,0x3e,0x51,0x49,0x45,0x3e,0x00,0x00,    /*     0    16*/
        0x00,0x00,0x42,0x7f,0x40,0x00,0x00,0x00,    /*    1    17*/
        0x00,0x42,0x61,0x51,0x49,0x46,0x00,0x00,    /*    2    18*/
        0x00,0x21,0x41,0x45,0x4b,0x31,0x00,0x00,    /*    3    19*/
        0x00,0x18,0x14,0x12,0x7f,0x10,0x00,0x00,    /*    4    20*/
        
        0x00,0x27,0x45,0x45,0x45,0x39,0x00,0x00,    /*    5    21*/
        0x00,0x3c,0x4a,0x49,0x49,0x30,0x00,0x00,    /*    6    22*/
        0x00,0x01,0x01,0x79,0x05,0x03,0x00,0x00,    /*    7    23*/
        0x00,0x36,0x49,0x49,0x49,0x36,0x00,0x00,    /*    8    24*/
        0x00,0x06,0x49,0x49,0x29,0x1e,0x00,0x00,    /*    9    25*/
        
        /*    sepecial    char    */
        0x00,0x00,0x66,0x66,0x00,0x00,0x00,0x00,    /*    :    26*/
        0x00,0x00,0x40,0x32,0x00,0x00,0x00,0x00,    /*    ;    27*/
        0x00,0x08,0x14,0x22,0x41,0x00,0x00,0x00,    /*    <    28*/
        0x00,0x24,0x24,0x24,0x24,0x24,0x24,0x00,    /*    =    29*/
        0x00,0x41,0x22,0x14,0x08,0x00,0x00,0x00,    /*    >    30*/
        0x00,0x06,0x01,0x51,0x09,0x06,0x00,0x00,    /*    ?    31*/
        0x3C,0x42,0x59,0x55,0x1D,0x22,0x1E,0x00,    /*    @    32*/
        
        /*    CAPITAL letter    uppercase    */
        0x00,0x7E,0x11,0x11,0x11,0x7E,0x00,0x00,    /*    A    33*/
        0x00,0x7F,0x49,0x49,0x49,0x36,0x00,0x00,    /*    B    34*/
        0x00,0x1C,0x22,0x41,0x41,0x41,0x00,0x00,    /*    C    35*/
        0x00,0x7F,0x41,0x41,0x22,0x1C,0x00,0x00,    /*    D    36*/
        0x00,0x7F,0x49,0x49,0x49,0x41,0x00,0x00,    /*    E    37*/
        0x00,0x7F,0x09,0x09,0x09,0x01,0x00,0x00,    /*    F    38*/
        0x00,0x1C,0x22,0x41,0x49,0x39,0x08,0x00,    /*    G    39*/
        
        0x00,0x7F,0x08,0x08,0x08,0x7F,0x00,0x00,    /*    H    40*/
        0x00,0x41,0x41,0x7F,0x41,0x41,0x00,0x00,    /*    I    41*/
        0x20,0x41,0x41,0x3F,0x01,0x01,0x00,0x00,    /*    J    42*/
        0x00,0x41,0x7F,0x59,0x34,0x43,0x41,0x00,    /*    K    43*/
        0x00,0x7F,0x40,0x40,0x40,0x40,0x00,0x00,    /*    L    44*/
        0x00,0x7F,0x07,0x78,0x07,0x7F,0x00,0x00,    /*    M    45*/
        0x00,0x7F,0x07,0x18,0x30,0x7F,0x00,0x00,    /*    N    46*/
        
        0x00,0x1C,0x22,0x41,0x41,0x22,0x1C,0x00,    /*    O    47*/
        //0x00,0x7E,0x09,0x09,0x09,0x06,0x00,0x00,    /*    P    48*/
        0x00,0x01,0x7F,0x09,0x09,0x09,0x06,0x00,
        0x00,0x1C,0x22,0x51,0x71,0x22,0x5C,0x00,    /*    Q    49*/
        0x00,0x7F,0x09,0x19,0x29,0x46,0x00,0x00,    /*    R    50*/
        0x00,0x26,0x45,0x49,0x49,0x31,0x00,0x00,    /*    S    51*/
        0x00,0x01,0x01,0x7f,0x01,0x01,0x00,0x00,    /*    T    52*/
        
        0x00,0x3F,0x40,0x40,0x40,0x3F,0x00,0x00,    /*    U    53*/
        //0x01,0x07,0x38,0x40,0x38,0x07,0x01,0x00,    /*    V    54*/
        0x00,0x07,0x38,0x40,0x38,0x07,0x00,0x00,
        0x01,0x1F,0x70,0x0F,0x70,0x1F,0x01,0x00,    /*    W    55*/
        //0x00,0x41,0x63,0x1C,0x1C,0x63,0x41,0x00,    /*    X    56*/
        0x00,0x41,0x67,0x18,0x18,0x67,0x41,0x00,
        0x00,0x03,0x04,0x78,0x04,0x03,0x00,0x00,    /*    Y    57*/
        0x00,0x61,0x51,0x49,0x45,0x43,0x00,0x00,    /*    Z    58*/
        
        /*    sepecial    char    */
        0x00,0x00,0x7F,0x41,0x41,0x00,0x00,0x00,    /*    [    59*/
        0x00,0x02,0x04,0x08,0x10,0x20,0x00,0x00,    /*    \    60*/
        0x00,0x00,0x41,0x41,0x7F,0x00,0x00,0x00,    /*    ]    61*/
        0x00,0x00,0x10,0x08,0x04,0x08,0x10,0x00,    /*    ^    62*/
        0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x00,    /*    _    63*/
        0x00,0x00,0x05,0x03,0x00,0x00,0x00,0x00,    /*    '    64*/
        
        /*    lowercase  letter    */    
        0x00,0x24,0x52,0x4A,0x4A,0x7C,0x40,0x00,    /*    a    65*/
        0x00,0x7F,0x48,0x48,0x48,0x30,0x00,0x00,    /*    b    66*/
        0x00,0x3C,0x42,0x42,0x42,0x26,0x00,0x00,    /*    c    67*/
        0x00,0x30,0x48,0x48,0x48,0x7F,0x00,0x00,    /*    d    68*/
        0x00,0x3C,0x4A,0x4A,0x4A,0x2C,0x00,0x00,    /*    e    69*/
        0x00,0x08,0x08,0x7E,0x09,0x09,0x00,0x00,    /*    f    70*/
        0x00,0x2E,0x55,0x55,0x53,0x21,0x00,0x00,    /*    g    71*/
        
        0x00,0x7F,0x08,0x04,0x04,0x78,0x00,0x00,    /*    h    72*/
        0x00,0x44,0x44,0x7D,0x40,0x40,0x00,0x00,    /*    i    73*/
        0x00,0x40,0x44,0x44,0x3D,0x00,0x00,0x00,    /*    j    74*/
        0x00,0x7F,0x18,0x24,0x42,0x00,0x00,0x00,    /*    k    75*/
        0x00,0x3F,0x40,0x40,0x40,0x30,0x00,0x00,    /*    l    76*/
        0x00,0x7C,0x02,0x7E,0x02,0x7C,0x00,0x00,    /*    m    77*/
        0x00,0x7E,0x04,0x02,0x02,0x7C,0x00,0x00,    /*    n    78*/
        
        0x00,0x3C,0x42,0x42,0x42,0x3C,0x00,0x00,    /*    o    79*/
        //0x00,0x01,0x7F,0x09,0x09,0x09,0x06,0x00,    /*    p    80*/
        0x00,0x00,0x7F,0x09,0x09,0x06,0x00,0x00,
        0x00,0x06,0x09,0x09,0x09,0x7F,0x00,0x00,    /*    q    81*/
        0x00,0x42,0x7E,0x44,0x42,0x02,0x00,0x00,    /*    r    82*/
        0x00,0x64,0x4A,0x4A,0x52,0x26,0x00,0x00,    /*    s    83*/
        0x00,0x04,0x04,0x3F,0x44,0x44,0x00,0x00,    /*    t    84*/
        
        0x00,0x3E,0x40,0x40,0x40,0x7E,0x40,0x00,    /*    u    85*/
        0x00,0x00,0x0E,0x30,0x40,0x30,0x0E,0x00,    /*    v    86*/
        0x00,0x1E,0x60,0x1E,0x60,0x1E,0x00,0x00,    /*    w    87*/
        0x00,0x42,0x66,0x58,0x1A,0x66,0x42,0x00,    /*    x    88*/
        0x00,0x43,0x4C,0x30,0x0C,0x03,0x00,0x00,    /*    y    89*/
        0x00,0x46,0x62,0x5A,0x46,0x62,0x00,0x00,    /*    z    90*/
        
        /*    sepecial    char    */
        0x00,0x00,0x08,0x77,0x41,0x00,0x00,0x00,    /*    {    91*/
        0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00,    /*    |    92*/
        0x00,0x00,0x41,0x77,0x08,0x00,0x00,0x00,    /*    }    93*/
        0x00,0x10,0x08,0x10,0x20,0x20,0x10,0x00,    /*    ~    94*/
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00     /* space   95 */   
          
};
#endif

const unsigned char Ch8ModAddr[]={
			0x00,0x00,0x00,0x00,0x00,0x00,    /*   */
			0x00,0x00,0x9e,0x00,0x00,0x00,    /* ! 0x21*/
			0x00,0x0e,0x00,0x0e,0x00,0x00,    /* " 0x22*/
			0x28,0xfe,0x28,0xfe,0x28,0x00,    /* # 0x23*/
			0x48,0x54,0xfe,0x54,0x24,0x00,    /* $ 0x24*/
			0x46,0x26,0x10,0xc8,0xc4,0x00,    /* % 0x25*/
			0x6c,0x92,0xaa,0x44,0xa0,0x00,    /* & 0x26*/
			0x00,0x0a,0x06,0x00,0x00,0x00,    /* ' 0x27*/
			0x00,0x38,0x44,0x82,0x00,0x00,    /* ( 0x28*/
			0x00,0x00,0x82,0x44,0x38,0x00,    /* ) 0x29*/
			0x28,0x10,0x7c,0x10,0x28,0x00,    /* * 0x2a*/
			0x10,0x10,0xfe,0x10,0x10,0x00,    /* + 0x2b*/
			0x00,0xa0,0x60,0x00,0x00,0x00,    /* , 0x2c*/
			0x10,0x10,0x10,0x10,0x10,0x00,    /* - 0x2d*/
			0x00,0xc0,0xc0,0x00,0x00,0x00,    /* . 0x2e*/
			0x40,0x20,0x10,0x08,0x04,0x00,    /* / 0x2f*/
			0x7c,0xa2,0x92,0x8a,0x7c,0x00,    /* 0 0x30*/
			0x00,0x84,0xfe,0x80,0x00,0x00,    /* 1 0x31*/
			0x84,0xc2,0xa2,0x92,0x8c,0x00,    /* 2 0x32*/
			0x42,0x82,0x8a,0x96,0x62,0x00,    /* 3 0x33*/
			0x30,0x28,0x24,0xfe,0x20,0x00,    /* 4 0x34*/
			0x4e,0x8a,0x8a,0x8a,0x72,0x00,    /* 5 0x35*/
			0x78,0x94,0x92,0x92,0x60,0x00,    /* 6 0x36*/
			0x02,0xe2,0x12,0x0a,0x06,0x00,    /* 7 0x37*/
			0x6c,0x92,0x92,0x92,0x6c,0x00,    /* 8 0x38*/
			0x0c,0x92,0x92,0x52,0x3c,0x00,    /* 9 0x39*/
			0x00,0x00,0xcc,0xcc,0x00,0x00,    /* : 0x3a*/
			0x00,0x00,0xa6,0x66,0x00,0x00,    /* ; 0x3b*/
			0x10,0x28,0x44,0x82,0x00,0x00,    /* < 0x3c*/
			0x48,0x48,0x48,0x48,0x48,0x00,    /* = 0x3d*/
			0x00,0x82,0x44,0x28,0x10,0x00,    /* > 0x3e*/
			0x04,0x02,0xa2,0x12,0x0c,0x00,    /* ? 0x3f*/
			0x64,0x92,0xf2,0x82,0x7c,0x00,    /* @ 0x40*/
			0xfc,0x22,0x22,0x22,0xfc,0x00,    /* A 0x41*/
			0xfe,0x92,0x92,0x92,0x6c,0x00,    /* B 0x42*/
			0x7c,0x82,0x82,0x82,0x44,0x00,    /* C 0x43*/
			0xfe,0x82,0x82,0x44,0x38,0x00,    /* D 0x44*/
			0xfe,0x92,0x92,0x92,0x82,0x00,    /* E 0x45*/
			0xfe,0x12,0x12,0x12,0x12,0x02,    /* F 0x46*/
			0x7c,0x82,0x82,0x92,0xf4,0x00,    /* G 0x47*/
			0xfe,0x10,0x10,0x10,0xfe,0x00,    /* H 0x48*/
			0x00,0x82,0xfe,0x82,0x00,0x00,    /* I 0x49*/
			0x40,0x80,0x82,0x7e,0x02,0x00,    /* J 0x4a*/
			0xfe,0x10,0x28,0x44,0x82,0x00,    /* K 0x4b*/
			0xfe,0x80,0x80,0x80,0x80,0x00,    /* L 0x4c*/
			0xfe,0x04,0x18,0x04,0xfe,0x00,    /* M 0x4d*/
			0xfe,0x08,0x10,0x20,0xfe,0x00,    /* N 0x4e*/
			0x7c,0x82,0x82,0x82,0x7c,0x00,    /* O 0x4f*/
			0xfe,0x12,0x12,0x12,0x0c,0x00,    /* P 0x50*/
			0x7c,0x82,0xa2,0x42,0xbc,0x00,    /* Q 0x51*/
			0xfe,0x12,0x32,0x52,0x8c,0x00,    /* R 0x52*/
			0x8c,0x92,0x92,0x92,0x62,0x00,    /* S 0x53*/
			0x02,0x02,0xfe,0x02,0x02,0x00,    /* T 0x54*/
			0x7e,0x80,0x80,0x80,0x7e,0x00,    /* U 0x55*/
			0x3e,0x40,0x80,0x40,0x3e,0x00,    /* V 0x56*/
			0x7e,0x80,0x70,0x80,0x7e,0x00,    /* W 0x57*/
			0xc6,0x28,0x10,0x28,0xc6,0x00,    /* X 0x58*/
			0x0e,0x10,0xe0,0x10,0x0e,0x00,    /* Y 0x59*/
			0xc2,0xa2,0x92,0x8a,0x86,0x00,    /* Z 0x5a*/
			0x00,0xfe,0x82,0x82,0x00,0x00,    /* [ 0x5b*/
			0x08,0x10,0x20,0x40,0x80,0x00,    /* \ 0x5c*/
			0x00,0x82,0x82,0xfe,0x00,0x00,    /* ] 0x5d*/
			0x08,0x04,0x02,0x04,0x08,0x00,    /* ^ 0x5e*/
			0x80,0x80,0x80,0x80,0x80,0x00,    /* _ 0x5f*/
			0x02,0x04,0x08,0x00,0x00,0x00,    /* ` 0x60*/
			0x40,0xa8,0xa8,0xa8,0xf0,0x00,    /* a 0x61*/
			0xfe,0x90,0x88,0x88,0x70,0x00,    /* b 0x62*/
			0x70,0x88,0x88,0x88,0x40,0x00,    /* c 0x63*/
			0x70,0x88,0x88,0x90,0xfe,0x00,    /* d 0x64*/
			0x70,0xa8,0xa8,0xa8,0x30,0x00,    /* e 0x65*/
			0x10,0xfc,0x12,0x12,0x04,0x00,    /* f 0x66*/
			0x18,0xa4,0xa4,0xa4,0x7c,0x00,    /* g 0x67*/
			0xfe,0x10,0x08,0x08,0xf0,0x00,    /* h 0x68*/
			0x00,0x88,0xfa,0x80,0x00,0x00,    /* i 0x69*/
			0x40,0x80,0x88,0x7a,0x00,0x00,    /* j 0x6a*/
			0xfe,0x20,0x50,0x88,0x00,0x00,    /* k 0x6b*/
			0x00,0x82,0xfe,0x80,0x00,0x00,    /* l 0x6c*/
			0xf8,0x08,0x30,0x08,0xf8,0x00,    /* m 0x6d*/
			0xf8,0x10,0x08,0x08,0xf0,0x00,    /* n 0x6e*/
			0x70,0x88,0x88,0x88,0x70,0x00,    /* o 0x6f*/
			0xf8,0x28,0x28,0x28,0x10,0x00,    /* p 0x70*/
			0x10,0x28,0x28,0x30,0xf8,0x00,    /* q 0x71*/
			0xf8,0x10,0x08,0x08,0x10,0x00,    /* r 0x72*/
			0x90,0xa8,0xa8,0xa8,0x40,0x00,    /* s 0x73*/
			0x08,0x7e,0x88,0x80,0x40,0x00,    /* t 0x74*/
			0x78,0x80,0x80,0x40,0xf8,0x00,    /* u 0x75*/
			0x38,0x40,0x80,0x40,0x38,0x00,    /* v 0x76*/
			0x78,0x80,0x60,0x80,0x78,0x00,    /* w 0x77*/
			0x88,0x50,0x20,0x50,0x88,0x00,    /* x 0x78*/
			0x98,0xa0,0xa0,0xa0,0x78,0x00,    /* y 0x79*/
			0x88,0xc8,0xa8,0x98,0x88,0x00,    /* z 0x7a*/
			0x00,0x10,0x6c,0x82,0x00,0x00,    /* { 0x7b*/
			0x00,0x00,0xfe,0x00,0x00,0x00,    /* | 0x7c*/
			0x00,0x82,0x6c,0x10,0x00,0x00,    /* } 0x7d*/
			0x08,0x04,0x08,0x08,0x04,0x00,    /* ~ 0x7e*/
			0xfe,0xfe,0xfe,0xfe,0xfe,0x00     /*  */
};


typedef struct 
{
  unsigned short nHz;
  unsigned char  sDot[18];
} HZ_T;

// 字模取样规则：纵向取模，字节倒序。总共会生成24字节，将末尾12字节合并为6字节，规则为(data[n] | (data[n+1] << 8))
HZ_T tHz12X12[]=
{
  
0xb0ac,0x02,0x02,0x02,0x32,0xcf,0x02,0x82,0x4f,0x32,0x02,0x02,0x00,0x44,0x24,0x12,0x21,0x44,0x04,//艾
0xb0b4,0x44,0x24,0xff,0x14,0x26,0xe2,0x3a,0x23,0xa2,0x62,0x26,0x00,0x44,0x07,0x44,0x35,0x21,0x04,//按
0xb0c9,0xfe,0x82,0xfe,0x00,0xfe,0x22,0x22,0x3e,0x22,0x3f,0x02,0x00,0x00,0x00,0x43,0x44,0x44,0x07,//吧
0xb0ce,0x44,0x44,0xff,0x24,0x00,0xe4,0xbf,0x24,0xa5,0x66,0x04,0x00,0x44,0x27,0x45,0x12,0x42,0x04,//拔
0xb0dc,0xff,0x01,0xfd,0x01,0xff,0x10,0x7c,0x87,0x84,0x7c,0x04,0x00,0x34,0x10,0x42,0x12,0x21,0x04,//败
0xb0fb,0x00,0xff,0x49,0xff,0x10,0xfc,0x4b,0x4a,0x7a,0x02,0xfe,0x00,0x16,0x74,0x70,0x54,0x55,0x06,//胞
0xb0fc,0x10,0x08,0xfc,0x4b,0x4a,0x4a,0x7a,0x82,0x82,0x7e,0x00,0x00,0x00,0x43,0x44,0x44,0x44,0x07,//包
0xb1a7,0x44,0x24,0xff,0x24,0x10,0xfc,0x4b,0x4a,0x7a,0x82,0x7e,0x00,0x44,0x07,0x70,0x44,0x44,0x07,//抱
0xb1b1,0x08,0x08,0x08,0xff,0x00,0x00,0xff,0x10,0x10,0x08,0x88,0x00,0x22,0x71,0x00,0x47,0x44,0x07,//北
0xb1bb,0x44,0x25,0xf6,0xac,0x00,0xfc,0xa4,0x3f,0xa4,0x64,0x0c,0x00,0x00,0x47,0x52,0x12,0x42,0x04,//被
0xb1e0,0xd8,0xb4,0x53,0x48,0xfe,0xaa,0xea,0xab,0xea,0xaa,0xee,0x00,0x22,0x25,0x07,0x03,0x43,0x07,//编
0xb1ea,0xc4,0x34,0xff,0x24,0x12,0xd2,0x12,0xf2,0x52,0x93,0x12,0x00,0x00,0x27,0x41,0x74,0x00,0x03,//标
0xb2bb,0x81,0x81,0x41,0x21,0x11,0xfd,0x03,0x11,0x21,0x41,0x81,0x00,0x00,0x00,0x70,0x00,0x00,0x00,//不
0xb2d9,0x44,0xff,0x24,0xbc,0xa8,0xbf,0xc5,0xbd,0xaf,0xa8,0xb8,0x00,0x74,0x44,0x12,0x07,0x21,0x04,//操
0xb2e2,0x89,0xf2,0x00,0xff,0x01,0xf9,0xff,0x00,0xfc,0x00,0xff,0x00,0x70,0x44,0x12,0x42,0x41,0x07,//测
0xb2e5,0x44,0x24,0xff,0x14,0xea,0xaa,0x0a,0xff,0x09,0xa9,0xe8,0x00,0x44,0x07,0x27,0x32,0x22,0x07,//插
0xb2e9,0x22,0x22,0xf2,0x5a,0x56,0x5f,0x52,0x56,0xfa,0x12,0x22,0x00,0x44,0x55,0x55,0x55,0x45,0x04,//查
0xb3a2,0x08,0x4c,0x55,0x56,0xd4,0x57,0x54,0x56,0x55,0x44,0x0c,0x00,0x40,0x56,0x44,0x54,0x46,0x00,//尝
0xb3a4,0x20,0x20,0xff,0x20,0x30,0x68,0xa4,0x22,0x21,0x20,0x20,0x00,0x00,0x47,0x22,0x10,0x42,0x04,//长
0xb3ac,0xd4,0x14,0xff,0x54,0x44,0x10,0xe9,0x27,0x21,0x29,0xef,0x00,0x27,0x43,0x44,0x55,0x55,0x05,//超
0xb3c9,0x00,0xfc,0x24,0x24,0xe4,0x04,0x7f,0x84,0x45,0x36,0x04,0x00,0x34,0x21,0x41,0x12,0x42,0x07,//成
0xb3d6,0x44,0x44,0xff,0x24,0xd2,0x52,0x52,0x5f,0xf2,0x52,0x52,0x00,0x44,0x07,0x10,0x44,0x07,0x00,//持
0xb3f5,0x44,0x24,0xf7,0x2c,0x50,0x02,0xc2,0x3e,0x02,0x02,0xfe,0x00,0x00,0x07,0x24,0x41,0x64,0x01,//初
0xb3f6,0x00,0xde,0x10,0x10,0x10,0xff,0x10,0x10,0x10,0xde,0x00,0x00,0x70,0x44,0x74,0x44,0x74,0x00,//出
0xb4a5,0xfc,0xab,0xfa,0xae,0xf8,0x00,0xf8,0x88,0xff,0x88,0xf8,0x00,0x07,0x47,0x07,0x44,0x27,0x06,//触
0xb4a6,0x40,0x30,0x4f,0x84,0x64,0x1c,0x00,0xff,0x08,0x10,0x20,0x00,0x44,0x12,0x22,0x74,0x44,0x04,//处
0xb4bf,0xd8,0xb4,0x9b,0x00,0x7a,0x42,0x42,0xff,0x42,0x7a,0x02,0x00,0x44,0x22,0x00,0x70,0x44,0x07,//纯
0xb4c5,0x22,0xfe,0x12,0xf2,0x64,0xdd,0x66,0x04,0x66,0xdd,0x64,0x00,0x70,0x32,0x56,0x06,0x56,0x06,//磁
0xb4ce,0x81,0xc2,0x34,0x00,0x10,0x0c,0xf7,0x84,0x04,0x14,0x0c,0x00,0x70,0x40,0x12,0x00,0x21,0x04,//次
0xb4ed,0x58,0xf7,0x54,0x40,0x14,0xd4,0x5f,0x54,0x5f,0xd4,0x14,0x00,0x70,0x12,0x70,0x55,0x75,0x00,//错
0xb4f3,0x08,0x08,0x08,0x08,0xc8,0x3f,0xc8,0x08,0x08,0x0c,0x08,0x00,0x44,0x12,0x00,0x10,0x42,0x04,//大
0xb4fd,0x48,0x24,0xf3,0x48,0x52,0xd2,0x5f,0x52,0xf2,0x52,0x50,0x00,0x00,0x07,0x00,0x45,0x07,0x00,//待
0xb5c8,0x28,0xa6,0xab,0xae,0xaa,0xbc,0xaa,0xab,0xee,0xaa,0xa2,0x00,0x00,0x10,0x02,0x44,0x07,0x00,//等
0xb5e7,0x00,0xfc,0x94,0x94,0x94,0xff,0x94,0x94,0x94,0xfe,0x04,0x00,0x10,0x00,0x30,0x44,0x44,0x06,//电
0xb6c1,0x11,0xf2,0x00,0x88,0xaa,0xda,0xaf,0xfa,0x8a,0x8a,0x98,0x00,0x70,0x02,0x44,0x12,0x42,0x00,//读
0xb6c8,0x00,0xfe,0x0a,0x8a,0xbe,0xaa,0xab,0xaa,0xbe,0x0a,0x0a,0x00,0x16,0x44,0x34,0x32,0x44,0x04,//度
0xb6cb,0x74,0x85,0x76,0x04,0xd6,0x54,0xf4,0x57,0xd4,0x54,0xd6,0x00,0x13,0x11,0x07,0x03,0x43,0x07,//端
0xb6d4,0x02,0x0a,0x92,0x62,0x9e,0x08,0x28,0xc8,0x08,0xff,0x08,0x00,0x12,0x00,0x03,0x40,0x74,0x00,//对
0xb6e0,0x00,0x28,0x28,0xa4,0x56,0x7b,0x52,0x4a,0x46,0xc2,0x40,0x00,0x54,0x45,0x25,0x12,0x01,0x00,//多
0xb6ee,0xa6,0xda,0xab,0xaa,0xde,0x01,0xfd,0x05,0xf7,0x05,0xfd,0x00,0x70,0x44,0x07,0x24,0x21,0x04,//额
0xb6fb,0x10,0x08,0x84,0x67,0x04,0xfc,0x04,0x24,0x54,0x8e,0x04,0x00,0x12,0x40,0x74,0x00,0x00,0x03,//尔
0xb7a2,0x08,0x0e,0x88,0x78,0x2f,0xe8,0x28,0xa9,0x6a,0x08,0x08,0x00,0x24,0x45,0x22,0x21,0x42,0x04,//发
0xb7a8,0x08,0xd1,0x06,0x20,0x24,0x24,0xe4,0x3f,0xa4,0x24,0x24,0x00,0x71,0x00,0x56,0x24,0x32,0x06,//法
0xb7c7,0x04,0x24,0x24,0x24,0xff,0x00,0xff,0x24,0x24,0x24,0x04,0x00,0x11,0x11,0x07,0x17,0x11,0x01,//非
0xb7d1,0x20,0x2e,0xea,0x5a,0x4f,0xca,0x7f,0x4a,0xea,0x2e,0x18,0x00,0x44,0x45,0x12,0x22,0x45,0x00,//费
0xb7dd,0x20,0xf8,0x07,0x10,0x28,0xe7,0x20,0x23,0xec,0x10,0x20,0x00,0x70,0x40,0x12,0x44,0x07,0x00,//份
0xb7f1,0x11,0x11,0xc9,0x49,0x45,0x7f,0x45,0x45,0xc9,0x09,0x11,0x00,0x00,0x27,0x22,0x22,0x07,0x00,//否
0xb8c3,0x10,0xf1,0x06,0x00,0x22,0x32,0xae,0x63,0x22,0x9a,0x42,0x00,0x30,0x12,0x54,0x22,0x21,0x04,//该
0xb8d0,0xc0,0x3e,0x0a,0x7a,0x5a,0x7a,0x8a,0x5f,0x22,0x5b,0xe2,0x00,0x34,0x70,0x54,0x46,0x16,0x02,//感
0xb8db,0x89,0x72,0x80,0x52,0xf2,0x5f,0x52,0x52,0xff,0x52,0x92,0x00,0x07,0x00,0x57,0x55,0x45,0x06,//港
0xb9a6,0x02,0x02,0xfe,0x82,0x82,0x08,0x88,0x7f,0x08,0x08,0xf8,0x00,0x11,0x40,0x24,0x41,0x44,0x03,//功
0xb9ab,0x20,0x10,0x08,0x07,0xc0,0x30,0x83,0x0c,0x10,0x20,0x20,0x00,0x40,0x56,0x44,0x32,0x06,0x00,//公
0xb9ca,0x04,0xe4,0x24,0x3f,0xe4,0x24,0x18,0x67,0x84,0x7c,0x04,0x00,0x70,0x22,0x47,0x24,0x21,0x04,//故
0xb9d8,0x40,0x44,0x44,0x45,0xc6,0x7c,0xc4,0x46,0x45,0x44,0x40,0x00,0x44,0x12,0x00,0x10,0x42,0x04,//关
0xb9d9,0x04,0x06,0xfa,0xaa,0xaa,0xab,0xaa,0xaa,0xba,0x02,0x06,0x00,0x00,0x47,0x44,0x44,0x07,0x00,//官
0xb9fd,0x11,0x16,0xf0,0x04,0x14,0x64,0x04,0x04,0xff,0x04,0x04,0x00,0x24,0x21,0x44,0x55,0x45,0x04,//过
0xbaba,0x88,0x91,0x62,0x18,0x02,0x1e,0x62,0x82,0x72,0x0f,0x02,0x00,0x70,0x40,0x24,0x01,0x21,0x04,//汉
0xbac5,0x20,0x20,0x2f,0xe9,0xa9,0xa9,0xa9,0xa9,0xaf,0x20,0x20,0x00,0x00,0x00,0x40,0x44,0x03,0x00,//号
0xbacb,0xc4,0x34,0xff,0x24,0x02,0xb2,0x6a,0x27,0x92,0x4a,0x02,0x00,0x00,0x07,0x45,0x12,0x21,0x04,//核
0xbaee,0x10,0xfc,0x03,0xa4,0x9d,0x95,0xf5,0x95,0x97,0x94,0x84,0x00,0x70,0x40,0x24,0x11,0x42,0x04,//侯
0xbaf2,0x10,0xfc,0x03,0xfc,0x80,0xb5,0x9d,0xf5,0x97,0x94,0x84,0x00,0x70,0x10,0x24,0x01,0x21,0x04,//候
0xbaf3,0x00,0x00,0xfe,0x0a,0xc9,0x49,0x49,0x49,0x49,0xc9,0x08,0x00,0x34,0x00,0x27,0x22,0x72,0x00,//后
0xbba4,0x44,0x44,0xff,0x24,0x00,0xfc,0x24,0x25,0x26,0x24,0x3c,0x00,0x44,0x47,0x03,0x00,0x00,0x00,//护
0xbbaa,0x88,0x84,0xbe,0x91,0x90,0xc8,0xbf,0xa8,0xa4,0xa2,0xb8,0x00,0x00,0x00,0x70,0x00,0x00,0x00,//华
0xbbaf,0x20,0x10,0xfc,0x83,0x80,0x40,0xff,0x20,0x10,0x0c,0x00,0x00,0x00,0x07,0x00,0x47,0x44,0x07,//化
0xbbd3,0x44,0x44,0xff,0x24,0x4b,0x79,0x4d,0xf9,0x49,0x49,0x4b,0x00,0x44,0x07,0x11,0x71,0x11,0x01,//挥
0xbbf2,0x04,0xf4,0x54,0x54,0xf4,0x04,0x7f,0x84,0x45,0x36,0x04,0x00,0x22,0x12,0x55,0x12,0x21,0x07,//或
0xbbfa,0x84,0x64,0xff,0x24,0x44,0xfe,0x02,0x02,0xff,0x02,0x00,0x00,0x01,0x47,0x12,0x00,0x47,0x07,//机
0xbcbc,0x44,0x44,0xff,0x24,0x00,0x64,0xa4,0x3f,0xa4,0x64,0x04,0x00,0x44,0x07,0x44,0x12,0x42,0x04,//技
0xbccc,0xd8,0xb6,0x51,0x48,0xff,0x12,0x94,0xff,0x58,0x96,0x10,0x00,0x26,0x12,0x57,0x74,0x54,0x04,//继
0xbce4,0xfc,0x01,0x02,0xf8,0x29,0x29,0x29,0xf9,0x01,0xff,0x00,0x00,0x07,0x10,0x11,0x11,0x74,0x00,//间
0xbcfc,0x4c,0xfb,0x4a,0xd2,0x1e,0xf2,0xaa,0xff,0xaa,0xbe,0x88,0x00,0x70,0x42,0x23,0x74,0x44,0x04,//键
0xbcfe,0x10,0xf8,0x07,0x50,0x4e,0x48,0x48,0xff,0x48,0x4c,0x48,0x00,0x70,0x00,0x00,0x70,0x00,0x00,//件
0xbdbb,0x22,0x12,0x0a,0x76,0x82,0x03,0x82,0x76,0x0a,0x12,0x02,0x00,0x44,0x24,0x12,0x22,0x44,0x04,//交
0xbdd3,0x44,0x44,0xff,0x24,0x52,0xd6,0x7a,0x53,0xda,0x56,0x52,0x00,0x44,0x07,0x54,0x25,0x43,0x04,//接
0xbde7,0x80,0x80,0x5f,0x35,0xd5,0x1f,0x15,0xf5,0x55,0x9f,0x80,0x00,0x40,0x24,0x01,0x70,0x00,0x00,//界
0xbdf0,0x10,0x90,0x88,0x94,0x92,0xf1,0x92,0x94,0x88,0x90,0x10,0x00,0x44,0x65,0x74,0x64,0x45,0x04,//金
0xbdf8,0x10,0xf1,0x02,0x24,0x24,0xff,0x24,0x24,0xff,0x24,0x24,0x00,0x16,0x62,0x45,0x44,0x47,0x04,//进
0xbea9,0x02,0x02,0x7a,0xca,0x4a,0xcb,0x4a,0xca,0x7a,0x02,0x02,0x00,0x24,0x41,0x74,0x00,0x61,0x00,//京
0xbeb3,0x04,0xff,0x84,0x08,0xfa,0xae,0xaa,0xab,0xae,0xfa,0x0a,0x00,0x11,0x44,0x24,0x01,0x47,0x06,//境
0xbedc,0x44,0x24,0xff,0x14,0x00,0xfe,0x92,0x92,0x92,0xf3,0x02,0x00,0x44,0x07,0x70,0x44,0x44,0x04,//拒
0xbedd,0x44,0x24,0xff,0x14,0xff,0xa5,0xa5,0xfd,0xa5,0xa7,0x20,0x00,0x44,0x27,0x71,0x44,0x74,0x00,//据
0xbef8,0x98,0xd4,0xb3,0x4c,0x10,0xfc,0x4b,0x7a,0x4e,0x4a,0x78,0x00,0x44,0x22,0x70,0x44,0x44,0x07,//绝
0xbefc,0x03,0x09,0x69,0x59,0x4f,0xf9,0x49,0x49,0x49,0x49,0x03,0x00,0x11,0x11,0x71,0x11,0x11,0x01,//军
0xbfa8,0x10,0x10,0x10,0x10,0xff,0x52,0x52,0x92,0x92,0x12,0x10,0x00,0x00,0x00,0x07,0x00,0x10,0x00,//卡
0xbfaa,0x20,0x22,0x22,0xfe,0x22,0x22,0x22,0xfe,0x22,0x23,0x22,0x00,0x44,0x12,0x00,0x70,0x00,0x00,//开
0xbfc6,0x8a,0x6a,0xfe,0x29,0x49,0x80,0x92,0xa4,0x80,0xff,0x40,0x00,0x01,0x07,0x00,0x00,0x70,0x00,//科
0xbfc9,0x02,0xfa,0x8a,0x8a,0x8a,0xfa,0x02,0x02,0xfe,0x03,0x02,0x00,0x10,0x00,0x10,0x44,0x07,0x00,//可
0xbfe9,0x08,0xff,0x08,0x48,0x44,0x44,0xff,0x44,0x44,0x7c,0x40,0x00,0x13,0x55,0x12,0x10,0x42,0x04,//块
0xc0ed,0x12,0x12,0xfe,0x12,0x3f,0xa5,0xa5,0xff,0xa5,0xa5,0x3f,0x00,0x22,0x51,0x44,0x74,0x44,0x04,//理
0xc1aa,0x02,0xfe,0x2a,0xfe,0x82,0x29,0x2a,0xfc,0x2a,0x29,0x28,0x00,0x12,0x71,0x24,0x01,0x21,0x04,//联
0xc1ac,0x11,0xf6,0x00,0x82,0x9a,0x96,0x93,0xfa,0x92,0x92,0x82,0x00,0x16,0x42,0x44,0x74,0x44,0x04,//连
0xc1d9,0xfc,0x00,0xff,0x00,0x10,0xcc,0x47,0xcc,0x54,0x44,0xc4,0x00,0x01,0x07,0x70,0x32,0x22,0x07,//临
0xc2e7,0xcc,0xaa,0x59,0x04,0x48,0xc4,0x6b,0x52,0x5a,0xe6,0x40,0x00,0x22,0x11,0x70,0x22,0x72,0x00,//络
0xc2eb,0x21,0xff,0x11,0xf1,0x80,0xa1,0xbd,0xa1,0xa1,0x3f,0xe0,0x00,0x30,0x31,0x00,0x40,0x44,0x03,//码
0xc3b4,0x40,0x20,0x10,0x08,0x84,0x43,0x20,0x98,0x00,0x00,0x00,0x00,0x00,0x32,0x22,0x22,0x63,0x00,//么
0xc3dc,0x44,0x56,0x4a,0x3a,0x62,0x57,0x4a,0x76,0x02,0x12,0x26,0x00,0x00,0x47,0x74,0x44,0x74,0x00,//密
0xc3e6,0x01,0xf9,0x09,0x09,0xfd,0xab,0xa9,0xf9,0x09,0x09,0xf9,0x00,0x70,0x44,0x47,0x74,0x44,0x07,//面
0xc4a3,0xc4,0x34,0xff,0x24,0x01,0x7d,0x57,0xd5,0x57,0x7d,0x01,0x00,0x00,0x07,0x55,0x13,0x53,0x05,//模
0xc4cf,0x02,0xfa,0x0a,0x5a,0x6a,0xcf,0x6a,0x5a,0x0a,0xfa,0x02,0x00,0x70,0x10,0x71,0x51,0x74,0x00,//南

0xc5fa,0x44,0x44,0xff,0x24,0x00,0xff,0x10,0x00,0xff,0x10,0x08,0x00,0x44,0x07,0x70,0x12,0x47,0x07,//批
0xc6ac,0x00,0x00,0xff,0x48,0x48,0x48,0x4f,0xc8,0x08,0x08,0x08,0x00,0x24,0x01,0x00,0x70,0x00,0x00,//片
0xc6b5,0x90,0x5e,0xf0,0x1f,0xd4,0x11,0xfd,0x05,0xf7,0x05,0xfd,0x00,0x44,0x12,0x44,0x12,0x10,0x06,//频
0xc6da,0x82,0xff,0xaa,0xaa,0xff,0x82,0x00,0xff,0x49,0x49,0xff,0x00,0x24,0x01,0x21,0x34,0x40,0x07,//期
0xc6e4,0x02,0x02,0x02,0xff,0x2a,0x2a,0x2a,0xff,0x02,0x02,0x02,0x00,0x55,0x13,0x11,0x11,0x53,0x01,//其
0xc6f7,0xa0,0xaf,0xa9,0xe9,0xaf,0x30,0xaf,0xa9,0xf9,0xaf,0xa0,0x00,0x70,0x44,0x07,0x47,0x74,0x00,//器
0xc7ae,0x58,0x4f,0xfa,0x4a,0x42,0x28,0x3f,0xe8,0x25,0xd6,0x14,0x00,0x00,0x23,0x45,0x22,0x21,0x07,//钱
0xc7b8,0xaa,0xff,0xaa,0xfe,0xab,0xfa,0x28,0x07,0xf4,0x04,0x0c,0x00,0x72,0x70,0x21,0x34,0x30,0x04,//歉
0xc7bf,0x79,0x49,0xc9,0x0f,0xe0,0xa7,0xa5,0xfd,0xa5,0xa7,0xe0,0x00,0x44,0x07,0x44,0x74,0x44,0x06,//强
0xc7e5,0x09,0xd2,0x20,0x22,0xea,0xaa,0xbf,0xaa,0xaa,0xea,0x22,0x00,0x71,0x00,0x27,0x22,0x72,0x00,//清
0xc7eb,0x10,0xf1,0x02,0x00,0x2a,0xea,0xaa,0xbf,0xaa,0xea,0x2a,0x00,0x70,0x12,0x70,0x22,0x72,0x00,//请
0xc7f8,0xff,0x01,0x01,0x09,0x91,0x61,0x51,0x8d,0x01,0x01,0x00,0x00,0x47,0x55,0x44,0x44,0x45,0x04,//区
0xc8a1,0x01,0xff,0x29,0xa9,0xff,0x81,0x7e,0x82,0x42,0x3e,0x00,0x00,0x11,0x01,0x47,0x12,0x21,0x04,//取
0xc8a5,0x20,0x24,0x24,0x24,0xa4,0x7f,0x24,0xa4,0x26,0x24,0x20,0x00,0x00,0x56,0x44,0x44,0x65,0x00,//去
0xc8a8,0x84,0x64,0xff,0x24,0x44,0x1e,0x62,0x82,0x62,0x1e,0x00,0x00,0x01,0x47,0x24,0x01,0x21,0x04,//权
0xc8b7,0x22,0xfa,0x16,0xf2,0x0a,0xfc,0xab,0xfa,0xae,0xaa,0xf8,0x00,0x30,0x31,0x34,0x70,0x40,0x07,//确
0xc8cb,0x00,0x00,0x00,0x80,0x60,0x1f,0x60,0x80,0x00,0x00,0x00,0x00,0x44,0x12,0x00,0x10,0x42,0x04,//人
0xc8ce,0x20,0x10,0xfc,0x23,0x24,0x24,0x24,0xfe,0x22,0x22,0x20,0x00,0x00,0x07,0x44,0x74,0x44,0x04,//任
0xc8cf,0x10,0xf1,0x06,0x00,0x00,0x00,0xc0,0x3f,0xc0,0x00,0x00,0x00,0x30,0x52,0x12,0x00,0x30,0x04,//认
0xc8d5,0x00,0x00,0xff,0x11,0x11,0x11,0x11,0x11,0x11,0xff,0x00,0x00,0x00,0x27,0x22,0x22,0x72,0x00,//日
0xc8eb,0x00,0x00,0x00,0xc1,0x31,0x0e,0x30,0xc0,0x00,0x00,0x00,0x00,0x24,0x01,0x00,0x00,0x21,0x04,//入
0xc9bd,0x00,0xf8,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0xf8,0x00,0x00,0x30,0x22,0x32,0x22,0x72,0x00,//山
0xc9d4,0x8a,0x6a,0xfe,0x29,0x00,0xfa,0xac,0xaf,0xa8,0xac,0xfa,0x00,0x01,0x07,0x70,0x00,0x44,0x07,//稍
0xc9e4,0x80,0xfe,0xab,0xaa,0xfe,0x08,0x28,0xc8,0x08,0xff,0x08,0x00,0x24,0x41,0x07,0x40,0x74,0x00,//射
0xc9ed,0x80,0x80,0xfe,0xaa,0xab,0xaa,0xaa,0xaa,0xfe,0x20,0x00,0x00,0x44,0x24,0x12,0x44,0x07,0x00,//身
0xc9ee,0x08,0xf1,0x1a,0x40,0x56,0xca,0x46,0xf2,0xc6,0x4a,0x56,0x00,0x71,0x20,0x01,0x70,0x10,0x02,//深
0xcaa7,0x20,0x30,0x2e,0x24,0xa4,0x7f,0xa4,0x24,0x24,0x24,0x20,0x00,0x44,0x12,0x00,0x10,0x42,0x04,//失
0xcab1,0xfe,0x22,0x22,0xfe,0x04,0x14,0x64,0x04,0xff,0x04,0x04,0x00,0x13,0x31,0x00,0x44,0x07,0x00,//时
0xcabc,0x84,0xfc,0x07,0xfc,0x04,0x18,0xd4,0x53,0x50,0x54,0xd8,0x00,0x24,0x11,0x02,0x27,0x22,0x07,//始
0xcabd,0x24,0x24,0x24,0xe4,0x24,0x24,0x7f,0x84,0x05,0x06,0x04,0x00,0x44,0x34,0x22,0x10,0x42,0x07,//式
0xcac7,0x40,0x40,0xc0,0x5f,0x55,0xd5,0x55,0x55,0x5f,0x40,0x40,0x00,0x24,0x11,0x72,0x55,0x55,0x04,//是
0xcad0,0x02,0xf2,0x12,0x12,0x12,0xff,0x12,0x12,0x12,0xf2,0x02,0x00,0x30,0x00,0x70,0x20,0x32,0x00,//市
0xcad4,0x11,0xf6,0x00,0x04,0x24,0xe4,0x24,0x24,0xff,0x05,0x06,0x00,0x70,0x12,0x32,0x11,0x21,0x07,//试
0xcad6,0x42,0x4a,0x4a,0x4a,0x4a,0xfe,0x49,0x49,0x49,0x49,0x40,0x00,0x00,0x40,0x74,0x00,0x00,0x00,//手
0xcada,0x44,0x24,0xff,0x14,0x32,0x5e,0xd2,0x56,0xd9,0x57,0x31,0x00,0x44,0x07,0x44,0x12,0x42,0x04,//授
0xcadc,0x20,0x32,0x16,0x5a,0xd2,0x5e,0x51,0xd9,0x55,0x11,0x30,0x00,0x44,0x24,0x12,0x21,0x44,0x04,//受
0xcae4,0x34,0x2f,0xf4,0xa4,0xe8,0xa4,0xea,0x09,0xea,0x04,0xe8,0x00,0x11,0x07,0x27,0x07,0x41,0x07,//输
0xcaf5,0x08,0x08,0x88,0x48,0x28,0xff,0x28,0x49,0x8a,0x08,0x08,0x00,0x11,0x00,0x70,0x00,0x10,0x01,//术
0xcafd,0xa5,0x96,0xcc,0xbf,0x96,0xa5,0x18,0x67,0x84,0x7c,0x04,0x00,0x54,0x22,0x43,0x24,0x21,0x04,//数
0xcba2,0x80,0x7f,0xe5,0x25,0xfd,0x25,0xe7,0x00,0xfe,0x00,0xff,0x00,0x01,0x03,0x27,0x03,0x45,0x07,//刷
0xcbbe,0x08,0x09,0xe9,0x29,0x29,0x29,0xed,0x09,0x01,0xff,0x00,0x00,0x00,0x11,0x11,0x41,0x74,0x00,//司
0xcbc0,0x42,0x22,0x5e,0x92,0x72,0x02,0xfe,0x22,0x22,0x13,0x12,0x00,0x24,0x01,0x00,0x43,0x44,0x07,//死
0xcbf8,0x58,0xf7,0x54,0x54,0x00,0xfe,0x08,0xef,0x0c,0xfa,0x00,0x00,0x30,0x52,0x44,0x12,0x22,0x04,//锁
0xcbfb,0x20,0x10,0xff,0x20,0xfc,0x10,0x10,0xff,0x88,0xf8,0x00,0x00,0x00,0x07,0x43,0x54,0x44,0x07,//他
0xcbfc,0x08,0x06,0x02,0xf2,0x42,0x43,0x22,0x22,0x22,0x0a,0x06,0x00,0x00,0x70,0x44,0x44,0x74,0x00,//它
0xcca8,0x00,0x10,0xd8,0x54,0x52,0x51,0x50,0x54,0xd8,0x10,0x00,0x00,0x00,0x47,0x44,0x44,0x07,0x00,//台
0xcce5,0x20,0x10,0xfc,0x83,0x44,0x24,0x14,0xff,0x24,0x44,0x84,0x00,0x00,0x07,0x11,0x71,0x11,0x00,//体
0xcda8,0x11,0xf6,0x00,0x01,0xfd,0x57,0xfd,0x57,0x55,0xfd,0x00,0x00,0x16,0x42,0x45,0x45,0x55,0x04,//通
0xcdcb,0x11,0xf6,0x00,0x00,0xff,0x15,0x55,0x55,0x95,0x5f,0x00,0x00,0x16,0x22,0x55,0x45,0x54,0x04,//退
0xcdd1,0x00,0xff,0x49,0xff,0x00,0x3d,0xe6,0x24,0xe6,0x3d,0x00,0x00,0x16,0x74,0x24,0x01,0x47,0x07,//脱
0xcdf8,0x00,0xff,0x49,0x31,0x4d,0x81,0x49,0x31,0xcd,0x01,0xff,0x00,0x70,0x00,0x00,0x00,0x44,0x07,//网
0xcdfe,0x00,0xfc,0x54,0xf4,0x54,0xd4,0x04,0x7f,0x84,0x77,0x04,0x00,0x16,0x24,0x21,0x24,0x21,0x07,//威
0xcec4,0x04,0x04,0x04,0x3c,0xc5,0x06,0xc4,0x3c,0x04,0x06,0x04,0x00,0x44,0x24,0x12,0x22,0x44,0x04,//文
0xced5,0x44,0x44,0xff,0x24,0xff,0x55,0x75,0xd5,0x55,0x75,0x57,0x00,0x44,0x47,0x53,0x75,0x55,0x05,//握
0xcede,0x10,0x11,0x11,0x91,0x71,0x1f,0xf1,0x11,0x11,0x11,0x10,0x00,0x44,0x12,0x00,0x47,0x44,0x07,//无
0xcef3,0x10,0xf1,0x06,0x80,0xa0,0xaf,0xa9,0xe9,0xa9,0xaf,0xa0,0x00,0x70,0x52,0x24,0x01,0x21,0x04,//误
0xcfa2,0x00,0x00,0xfe,0xaa,0xab,0xaa,0xaa,0xaa,0xfe,0x00,0x00,0x00,0x34,0x70,0x54,0x46,0x16,0x02,//息
0xcfb5,0x00,0x02,0x4a,0xca,0x6e,0xda,0x49,0xc9,0x65,0x41,0x00,0x00,0x24,0x41,0x74,0x00,0x21,0x04,//系
0xcfde,0xff,0x81,0x99,0xe7,0x00,0xff,0x25,0xe5,0x25,0xbf,0x40,0x00,0x07,0x00,0x70,0x24,0x21,0x04,//限
0xcfdf,0xd8,0xb4,0x53,0x4c,0x20,0x28,0xff,0x28,0xa9,0x56,0x14,0x00,0x22,0x51,0x24,0x12,0x42,0x07,//线
0xcffb,0x08,0xd1,0x3a,0x00,0xfe,0xa8,0xaf,0xa8,0xac,0xfa,0x00,0x00,0x71,0x00,0x07,0x40,0x74,0x00,//消
0xd0c4,0x80,0x70,0x00,0xfc,0x00,0x01,0x0e,0x00,0x80,0x30,0xc0,0x00,0x00,0x70,0x44,0x44,0x07,0x00,//心
0xd0c5,0x20,0xf8,0x07,0x02,0xaa,0xaa,0xaa,0xab,0xaa,0xaa,0x02,0x00,0x70,0x00,0x27,0x22,0x72,0x00,//信
0xd0d0,0x48,0x24,0xf3,0x08,0x09,0x09,0x09,0x09,0xf9,0x09,0x08,0x00,0x00,0x07,0x40,0x44,0x07,0x00,//行
0xd0e8,0x2c,0xa5,0xad,0xad,0xe5,0xaf,0xa5,0xad,0xad,0xa5,0x2c,0x00,0x70,0x00,0x07,0x07,0x74,0x00,//需
0xd0f8,0x98,0xd4,0xb3,0x18,0x8a,0xda,0xaa,0xff,0x8a,0x8a,0x98,0x00,0x22,0x22,0x44,0x12,0x42,0x00,//续
0xd1a1,0x11,0xf6,0x00,0x18,0x16,0xf4,0x1f,0xf4,0x14,0x14,0x90,0x00,0x16,0x62,0x45,0x54,0x55,0x05,//选
0xd1af,0x10,0xf1,0x02,0x08,0x04,0xfb,0x4a,0x4a,0xfa,0x02,0xfe,0x00,0x30,0x12,0x10,0x51,0x45,0x03,//询
0xd1b6,0x10,0x11,0xf2,0x00,0x21,0x21,0xfd,0x21,0x21,0xff,0x00,0x00,0x00,0x27,0x01,0x07,0x30,0x06,//讯
0xd1d0,0x42,0xfe,0x12,0xf2,0x00,0x21,0xff,0x21,0x21,0xff,0x21,0x00,0x10,0x51,0x24,0x01,0x70,0x00,//研
0xd1d3,0x02,0xda,0x16,0xf2,0x00,0xf2,0x02,0xfe,0x12,0x11,0x11,0x00,0x24,0x21,0x54,0x55,0x55,0x05,//延
0xd1d4,0x02,0x02,0xaa,0xaa,0xaa,0xab,0xaa,0xaa,0xaa,0x02,0x02,0x00,0x00,0x47,0x44,0x44,0x07,0x00,//言
0xd2aa,0x41,0x5d,0x55,0xd5,0x7f,0x55,0x5f,0xd5,0x55,0x5d,0x41,0x00,0x44,0x55,0x25,0x23,0x44,0x00,//要
0xd2d1,0x00,0xfa,0x12,0x12,0x12,0x12,0x12,0x12,0x1f,0x02,0x80,0x00,0x70,0x44,0x44,0x44,0x44,0x07,//已
0xd2d7,0x80,0x80,0x5f,0x75,0xd5,0x55,0x55,0xd5,0x5f,0xc0,0x00,0x00,0x20,0x52,0x24,0x41,0x34,0x00,//易
0xd2e2,0x08,0x0a,0xfa,0xae,0xaa,0xab,0xaa,0xae,0xfa,0x0a,0x08,0x00,0x34,0x70,0x54,0x46,0x16,0x06,//意
0xd2eb,0x08,0xf9,0x02,0x90,0x51,0x53,0x4d,0xe9,0x55,0x53,0x10,0x00,0x30,0x01,0x11,0x71,0x11,0x01,//译
0xd2f8,0x58,0xf7,0x54,0x54,0x00,0xff,0x29,0xe9,0x29,0xbf,0x40,0x00,0x70,0x12,0x70,0x02,0x21,0x04,//银
0xd3a6,0x00,0xfe,0x02,0x32,0xc2,0x1b,0xe2,0x02,0xc2,0x3a,0x02,0x00,0x16,0x44,0x44,0x74,0x44,0x04,//应
0xd3c3,0x00,0xff,0x49,0x49,0x49,0xff,0x49,0x49,0x49,0xff,0x00,0x00,0x16,0x00,0x70,0x40,0x74,0x00,//用
0xd3d0,0x42,0x22,0x12,0xfa,0x4e,0x4b,0x4a,0x4a,0xfa,0x02,0x02,0x00,0x00,0x70,0x11,0x51,0x07,0x00,//有
0xd3e0,0x10,0x50,0x48,0xd4,0x52,0xf1,0x52,0xd4,0x48,0x50,0x10,0x00,0x24,0x41,0x74,0x00,0x61,0x00,//余
0xd3eb,0x80,0x80,0xa0,0xbf,0xa4,0xa4,0xa4,0xa4,0x24,0xf6,0x24,0x00,0x00,0x00,0x00,0x44,0x34,0x00,//与
0xd3ef,0x11,0xf2,0x00,0x21,0xa5,0xbd,0xa7,0xa5,0xbd,0xa1,0x21,0x00,0x70,0x12,0x47,0x44,0x74,0x00,//语
0xd4da,0x82,0x42,0xf2,0x0a,0x47,0x42,0xfa,0x42,0x42,0x42,0x02,0x00,0x00,0x47,0x44,0x47,0x44,0x04,//在
0xd4f1,0x44,0x44,0xff,0x24,0x55,0x53,0x4d,0xe9,0x55,0x53,0x10,0x00,0x44,0x07,0x11,0x71,0x11,0x01,//择
0xd5cf,0xff,0x81,0x9d,0xe3,0x0a,0xfa,0xae,0xab,0xae,0xfa,0x0a,0x00,0x07,0x20,0x22,0x72,0x22,0x02,//障
0xd5d2,0x44,0x44,0xff,0x24,0x24,0x08,0x7f,0x88,0x49,0x2a,0x08,0x00,0x44,0x07,0x24,0x11,0x42,0x07,//找
0xd5d5,0x00,0xff,0x49,0xff,0x10,0xf9,0x97,0x91,0x99,0xf7,0x00,0x00,0x24,0x20,0x04,0x42,0x20,0x04,//照
0xd5df,0x10,0x12,0x92,0xd2,0x7f,0x52,0x5a,0x56,0xd2,0x19,0x10,0x00,0x11,0x70,0x55,0x55,0x07,0x00,//者
0xd5e2,0x10,0xf1,0x02,0x04,0x14,0x14,0xa5,0x66,0x9c,0x04,0x04,0x00,0x12,0x21,0x55,0x44,0x54,0x04,//这
0xd5fd,0x02,0x02,0xf2,0x02,0x02,0xfe,0x22,0x22,0x22,0x23,0x02,0x00,0x44,0x47,0x74,0x44,0x44,0x04,//正
0xd6a4,0x11,0xf6,0x00,0x02,0xf2,0x02,0x02,0xfe,0x22,0x23,0x22,0x00,0x70,0x42,0x47,0x74,0x44,0x04,//证
0xd6a7,0x04,0x04,0x14,0x74,0x94,0x1f,0x94,0x54,0x34,0x06,0x04,0x00,0x44,0x24,0x12,0x22,0x44,0x04,//支
0xd6b9,0x00,0x00,0xf8,0x00,0x00,0x00,0xff,0x10,0x10,0x10,0x10,0x00,0x44,0x47,0x44,0x47,0x44,0x04,//止
0xd6c6,0x18,0xd6,0x54,0xff,0x54,0x56,0xd4,0x00,0xfc,0x00,0xff,0x00,0x30,0x70,0x20,0x03,0x44,0x07,//制
0xd6d0,0x00,0x7c,0x44,0x44,0x44,0xff,0x44,0x44,0x44,0x7c,0x00,0x00,0x00,0x00,0x70,0x00,0x00,0x00,//中
0xd6d5,0x4c,0x6a,0x59,0x24,0x48,0x44,0xab,0x92,0x2a,0x46,0x40,0x00,0x22,0x11,0x00,0x22,0x05,0x00,//终
0xd6d8,0x04,0x05,0xf5,0x55,0x55,0xff,0x55,0x55,0xf5,0x05,0x04,0x00,0x54,0x55,0x75,0x55,0x55,0x04,//重
0xd7bc,0x42,0xf4,0x0e,0x10,0xfc,0x97,0x94,0xff,0x94,0x94,0x94,0x00,0x30,0x00,0x47,0x74,0x44,0x04,//准
0xd7d3,0x20,0x21,0x21,0x21,0x21,0xf9,0x29,0x25,0x23,0x31,0x20,0x00,0x00,0x40,0x74,0x00,0x00,0x00,//子
0xd7e3,0x00,0x00,0xdf,0x11,0x11,0xf1,0x51,0x51,0x5f,0x40,0x00,0x00,0x24,0x11,0x32,0x44,0x44,0x04,//足
0xd7f7,0x10,0x08,0xfc,0x13,0x08,0x04,0xff,0x24,0x24,0x24,0x04,0x00,0x00,0x07,0x00,0x17,0x11,0x01,//作
0xd7f8,0x20,0x90,0x8e,0x90,0xa0,0xff,0xa0,0x90,0x8e,0x90,0x20,0x00,0x44,0x44,0x74,0x44,0x44,0x04,//坐
0xdbda,0x08,0x08,0xff,0x88,0x08,0xff,0x00,0xfe,0x00,0x00,0xff,0x00,0x11,0x44,0x12,0x30,0x00,0x07,//圳




};

