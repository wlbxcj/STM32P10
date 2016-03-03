#ifndef _DES_H
#define _DES_H

#define ENCRYPT     1    /* MODE == encrypt */
#define DECRYPT     0    /* MODE == decrypt */

void Lib_Des(unsigned char *input,unsigned char *output,unsigned char *key,int mode);
void Lib_Des24(unsigned char *input,unsigned char *output,unsigned char *deskey,int mode);
void s_DesMac24(const unsigned char *pbyDataIn24, const unsigned char *pbyKey24, unsigned char *pbyMac8);
void s_DesMac(unsigned char *pbuInData, unsigned char *pbyKey, unsigned char *pbyOutData);

#endif
