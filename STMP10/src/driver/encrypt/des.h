#ifndef _DES_H
#define _DES_H

#define ENCRYPT     1    /* MODE == encrypt */
#define DECRYPT     0    /* MODE == decrypt */

void Lib_Des(unsigned char *input,unsigned char *output,unsigned char *key,int mode);

#endif
