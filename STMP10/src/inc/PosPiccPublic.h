#ifndef _POSPICCPUBLIC_H_
#define _POSPICCPUBLIC_H_

int PosPiccGetPara(unsigned char bFlag,unsigned char *pbLen,unsigned char *pBuf);
int PosPiccSetPara(int nParaLen,unsigned char *psPara);

#endif