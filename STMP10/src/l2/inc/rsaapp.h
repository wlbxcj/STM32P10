#define MAX_RSA_MODULUS_LEN 248
#define STRUCT_PACK __attribute__((packed))

#pragma pack(1) 
typedef struct {
  unsigned short int bits;                     /* length in bits of modulus */
  unsigned char modulus[MAX_RSA_MODULUS_LEN ];  /* modulus */
  unsigned char exponent[3]; /* public exponent */
}R_RSA_PUBLIC_KEY;
#pragma pack() 

extern int RSAPublicKeyDecrypt(unsigned char *output, unsigned int *outputLen, unsigned char *input, unsigned int inputLen, R_RSA_PUBLIC_KEY *publicKey);

extern void Hash(unsigned char  * DataIn,unsigned int DataLen,unsigned char  * DataOut);
