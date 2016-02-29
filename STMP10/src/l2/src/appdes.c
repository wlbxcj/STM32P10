
#define BLOCK_SIZE	64
#define HALF_BLOCK	32
#define EP_SIZE 	48
#define LOOPCOUNT	16
#define QUARTET 	 4
#define SEXTET		 6
#define OCTET		 8
#define LEFTP		 0
#define RIGHTP		 1
#define PC1_SIZE	56
#define PC2_SIZE	48	/* equal to EP_SIZE */
#define HALF_PC1	28
#define ENCIPHER	 1
#define DECIPHER	 0
#define PSIZE		 8	/* 8 character & terminal */


#define UBYTE unsigned char

unsigned char gsIMEKmdkTalk[16]; //IMEKmdk 认证会话密钥
unsigned char gsIAEKmdkTalk[16]; //IAEKmdk 认证会话密钥

const UBYTE shift_left[LOOPCOUNT] = {
	1, 1, 2, 2, 2, 2, 2, 2,
	1, 2, 2, 2, 2, 2, 2, 1
};

const UBYTE shift_right[LOOPCOUNT] = {
	0, 1, 2, 2, 2, 2, 2, 2,
	1, 2, 2, 2, 2, 2, 2, 1
};

const UBYTE ip_table[BLOCK_SIZE] = {
	58, 50, 42, 34, 26, 18, 10, 2,
	60, 52, 44, 36, 28, 20, 12, 4,
	62, 54, 46, 38, 30, 22, 14, 6,
	64, 56, 48, 40, 32, 24, 16, 8,
	57, 49, 41, 33, 25, 17, 9, 1,
	59, 51, 43, 35, 27, 19, 11, 3,
	61, 53, 45, 37, 29, 21, 13, 5,
	63, 55, 47, 39, 31, 23, 15, 7
};

const UBYTE ep_table[EP_SIZE] = {
	32, 1, 2, 3, 4, 5,
	4, 5, 6, 7, 8, 9,
	8, 9, 10, 11, 12, 13,
	12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21,
	20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29,
	28, 29, 30, 31, 32, 1
};

const UBYTE sbox_table[OCTET][QUARTET][16] = {
	{
	 {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7},
	 {0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8},
	 {4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0},
	 {15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13}
	 },
	{
	 {15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10},
	 {3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5},
	 {0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15},
	 {13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9}
	 },
	{
	 {10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8},
	 {13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1},
	 {13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7},
	 {1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12}
	 },
	{
	 {7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15},
	 {13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9},
	 {10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4},
	 {3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14}
	 },
	{
	 {2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9},
	 {14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6},
	 {4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14},
	 {11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3}
	 },
	{

	 {12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11},
	 {10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8},
	 {9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6},
	 {4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13}
	 },

	{
	 {4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1},
	 {13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6},
	 {1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2},
	 {6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12}
	 },
	{
	 {13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7},
	 {1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2},
	 {7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8},
	 {2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11}
	 }
};

const UBYTE p_table[HALF_BLOCK] = {
	16, 7, 20, 21,
	29, 12, 28, 17,
	1, 15, 23, 26,
	5, 18, 31, 10,
	2, 8, 24, 14,
	32, 27, 3, 9,
	19, 13, 30, 6,
	22, 11, 4, 25
};

const UBYTE pc1_table[PC1_SIZE] = {
	57, 49, 41, 33, 25, 17, 9,
	1, 58, 50, 42, 34, 26, 18,
	10, 2, 59, 51, 43, 35, 27,
	19, 11, 3, 60, 52, 44, 36,

	63, 55, 47, 39, 31, 23, 15,
	7, 62, 54, 46, 38, 30, 22,
	14, 6, 61, 53, 45, 37, 29,
	21, 13, 5, 28, 20, 12, 4
};

const UBYTE pc2_table[PC2_SIZE] = {
	14, 17, 11, 24, 1, 5,
	3, 28, 15, 6, 21, 10,
	23, 19, 12, 4, 26, 8,
	16, 7, 27, 20, 13, 2,

	41, 52, 31, 37, 47, 55,
	30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53,
	46, 42, 50, 36, 29, 32
};


void stringcpy(UBYTE destination[], UBYTE source[], int max)
{
	int i;

	for (i = 0; i < max; i++)
		destination[i] = source[i];
}

void straight_permutation(UBYTE lookup[], UBYTE destination[], int max)
{
	int i, content, count = 0;
/*	UBYTE *source; */
	UBYTE source[64];

/*
	source = (UBYTE*) malloc(max);
*/
	stringcpy(source, destination, max);

	for (i = 0; i < max; i++) {
		content = lookup[count];
		destination[count++] = source[content - 1];
	}
/*
	free(source);
*/
}

void invert_permutation(UBYTE lookup[], UBYTE destination[], int max)
{
	int i, content, count = 0;
/*	UBYTE *source; */
	UBYTE source[64];

/*
	source = (UBYTE*) malloc(max);
*/
	stringcpy(source, destination, max);

	for (i = 0; i < max; i++) {
		content = lookup[count];
		destination[content - 1] = source[count++];
	}
/*
	free(source);
*/
}

void expansion_permutation(UBYTE lookup[], UBYTE source[], UBYTE destination[])
{
	int i, count, content;

	for (i = 0, count = 0; i < EP_SIZE; i++) {
		content = lookup[count];
		destination[count++] = source[content - 1];
	}
}

void dataxor(UBYTE destination[], UBYTE source[], int max)
{
	int i;

	for (i = 0; i < max; i++)
		destination[i] ^= source[i];
}

void confusion(UBYTE destination[], UBYTE source[], int num)
{
	int i, row, column = 0, result;

	row = (source[0] << 1) + source[5];
	for (i = 1; i < 5; i++) {
		column <<= 1;
		column += source[i];
	}
	result = sbox_table[num][row][column];
	for (i = 0; i < QUARTET; i++) {
		destination[i] = (result & 8) >> 3;
		result <<= 1;
	}
}

void permuted_choice(UBYTE lookup[], UBYTE destination[], UBYTE source[], int max)
{
	int i, content;

	for (i = 0; i < max; i++) {
		content = lookup[i];
		destination[i] = source[content - 1];
	}
}

void shift_left_regs(UBYTE c_reg[], UBYTE d_reg[], int num)
{
	int i, j, shift_cnt, msb;
	UBYTE temp_c, temp_d;

	msb = HALF_PC1 - 1;

	shift_cnt = shift_left[num];

	for (i = 0; i < shift_cnt; i++) {
		temp_c = c_reg[0];
		temp_d = d_reg[0];
		for (j = 0; j < msb; j++) {
			c_reg[j] = c_reg[j + 1];
			d_reg[j] = d_reg[j + 1];
		}
		c_reg[msb] = temp_c;
		d_reg[msb] = temp_d;
	}
}

void shift_right_regs(UBYTE c_reg[], UBYTE d_reg[], int num)
{
	int i, j, shift_cnt, lsb;
	UBYTE temp_c, temp_d;

	lsb = HALF_PC1 - 1;

	shift_cnt = shift_right[num];
	for (i = 0; i < shift_cnt; i++) {
		temp_c = c_reg[lsb];
		temp_d = d_reg[lsb];
		for (j = 0; j < lsb; j++) {
			c_reg[lsb - j] = c_reg[lsb - j - 1];
			d_reg[lsb - j] = d_reg[lsb - j - 1];
		}
		c_reg[0] = temp_c;
		d_reg[0] = temp_d;
	}
}

void hex2bin(UBYTE bin[], UBYTE hex[], int max)
{
	int i, j, count = 0;
	UBYTE hexa[64];

/*
	hexa = (UBYTE *) malloc(max);
*/
	stringcpy(hexa, hex, max);

	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++) {
			bin[count++] = (hexa[i] & 0x80) >> 7;
			hexa[i] <<= 1;
		}
/*
	free(hexa);
*/
}

void bin2hex(UBYTE hex[], UBYTE bin[])
{
	int i, j, count = 0;

	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			hex[i] = (hex[i] << 1) + bin[count++];
}

/*-----------------------------------------------------------------------
 * Name:           des
 * Description:    Encrypted/DeEncrypted
 * Parameters:     None
 * Return Value:   mode :0 - DeEncrypted
 *                       1 - Encrypted 
 * Notes:          None.
 *-----------------------------------------------------------------------*/
void desdata(unsigned char plaintext[], unsigned char origin_key[], int mode)
{
	int i, j;

	union {
		UBYTE full[BLOCK_SIZE];
		UBYTE half[2][HALF_BLOCK];
	} info;

	union {
		UBYTE full[HALF_BLOCK];
		UBYTE sbox[OCTET][QUARTET];
	} temp;

	union {
		UBYTE full[PC1_SIZE];
		UBYTE half[2][HALF_PC1];
	} key;

	union {
		UBYTE right[EP_SIZE];
		UBYTE sbox[OCTET][SEXTET];
	} exp;

	UBYTE chosen_key[EP_SIZE];

	hex2bin(info.full, origin_key, BLOCK_SIZE);
	permuted_choice((UBYTE *) pc1_table, (UBYTE *) key.full, (UBYTE *) info.full, PC1_SIZE);
	hex2bin(info.full, plaintext, BLOCK_SIZE);
	straight_permutation((UBYTE *) ip_table, (UBYTE *) info.full, BLOCK_SIZE);

	/* *****  main loop     ***** */

	for (i = 0; i < LOOPCOUNT; i++) {
		expansion_permutation((UBYTE *) ep_table, (UBYTE *) info.half[RIGHTP], (UBYTE *) exp.right);

		/* ****  key seletion  **** */

		if (mode == ENCIPHER)
			shift_left_regs(key.half[0], key.half[1], i);
		else
			shift_right_regs(key.half[0], key.half[1], i);

		permuted_choice((UBYTE *) pc2_table, (UBYTE *) chosen_key, (UBYTE *) key.full, PC2_SIZE);
		dataxor(exp.right, chosen_key, EP_SIZE);

		for (j = 0; j < OCTET; j++)
			confusion(temp.sbox[j], exp.sbox[j], j);

		straight_permutation((UBYTE *) p_table, (UBYTE *) temp.full, HALF_BLOCK);
		dataxor(temp.full, info.half[LEFTP], HALF_BLOCK);
		stringcpy(info.half[LEFTP], info.half[RIGHTP], HALF_BLOCK);
		stringcpy(info.half[RIGHTP], temp.full, HALF_BLOCK);
		bin2hex(plaintext, info.full);
	}
	stringcpy(temp.full, info.half[LEFTP], HALF_BLOCK);
	stringcpy(info.half[LEFTP], info.half[RIGHTP], HALF_BLOCK);
	stringcpy(info.half[RIGHTP], temp.full, HALF_BLOCK);
	invert_permutation((UBYTE *) ip_table, (UBYTE *) info.full, BLOCK_SIZE);
	bin2hex(plaintext, info.full);
}

/*-----------------------------------------------------------------------
 * Name:           des
 * Description:    Encrypted/DeEncrypted
 * Parameters:     plaintext(16Byte)
                  origin_key(16Byte)  
 * Return Value:   mode :0 - DeEncrypted
 *                       1 - Encrypted 
 * Notes:          None.
 *-----------------------------------------------------------------------*/

void TripleDes(unsigned char plaintext[], unsigned char origin_key[], int mode )
{
  unsigned char bMode;
  
  if(mode)
    bMode = 0;
  else
    bMode = 1;
    
  desdata(plaintext,origin_key,mode);
  desdata(plaintext,&origin_key[8],bMode);
  desdata(plaintext,origin_key,mode);

  desdata(&plaintext[8],origin_key,mode);
  desdata(&plaintext[8],&origin_key[8],bMode);
  desdata(&plaintext[8],origin_key,mode);
  
  
}

unsigned char TripleDes_Enc2(unsigned char bType,unsigned char bIndex,unsigned char *pRandR,unsigned char *pRandB,unsigned char *pEncData)
{
	unsigned char sKey[16];
	unsigned char sPlaintext[16];
	extern unsigned char gsIMEKmdk[16],gsIAEKmdk[16];
	
	switch(bType)
	{
		case 0: //IMEKmdk
			memcpy(sKey,gsIMEKmdk,16);
			break;
		case 4: //IAEKmdk
			memcpy(sKey,gsIAEKmdk,16);
			break;
			//?? read from file, bIndex
		default:   
			return 1;
		
	}
	
	memcpy(sPlaintext,&pRandR[4],4);
	memcpy(&sPlaintext[4],pRandB,4);
	memcpy(&sPlaintext[8],pRandR,4);
	memcpy(&sPlaintext[12],&pRandB[4],4);
	
	
	TripleDes(sPlaintext,sKey,1);
	memcpy(gsIMEKmdkTalk,sPlaintext,16);
	memcpy(pEncData,sPlaintext,16);
	return 0;
}

unsigned char TripleDes_Dec2(unsigned char bType,unsigned char bIndex,unsigned char *pRandR,unsigned char *pRandB,unsigned char *pEncData)
{
	unsigned char sKey[16];
	unsigned char sPlaintext[16];
	switch(bType)
	{
		case 0: //IMEKmdk
			memcpy(sKey,gsIMEKmdk,16);
			break;
		case 4: //IAEKmdk
			memcpy(sKey,gsIAEKmdk,16);
			break;
			//?? read from file, bIndex
		default:   
			return 1;
		
	}

	memcpy(sPlaintext,pEncData,16);
	TripleDes(sPlaintext,sKey,0);
	
	memcpy(pRandR,&sPlaintext[4],4);
	memcpy(&pRandR[4],&sPlaintext[12],4);

	memcpy(pRandB,&sPlaintext[8],4);
	memcpy(&pRandB[4],sPlaintext,4);
	
	return 0;
}


unsigned char TripleDes_Enc1(unsigned char bType,unsigned char bIndex,unsigned char *pRandR,unsigned char *pRandB,unsigned char *pEncData)
{
	unsigned char sKey[16];
	unsigned char sPlaintext[16];
	switch(bType)
	{
		case 0: //IMEKmdk
			memcpy(sKey,gsIMEKmdkTalk,16);
			break;
		case 4: //IAEKmdk
			memcpy(sKey,gsIAEKmdkTalk,16);
			break;
			//?? read from file, bIndex
		default:   
			return 1;
		
	}
	memcpy(sPlaintext,pRandB,8);
	memcpy(&sPlaintext[8],pRandR,8);
	TripleDes(sPlaintext,sKey,1);
	memcpy(pEncData,sPlaintext,16);
	return 0;
}

unsigned char TripleDes_Dec1(unsigned char bType,unsigned char bIndex,unsigned char *pRandM,unsigned char *pEncData)
{
	unsigned char sKey[16];
	unsigned char sPlaintext[16];
	switch(bType)
	{
		case 1: //use IMEKmdk Generate IMEK
			memcpy(sKey,gsIMEKmdkTalk,16);
			break;
		case 5: //use IAEKmdk Generate IAEK
			memcpy(sKey,gsIAEKmdkTalk,16);
			break;
			//?? read from file, bIndex
		default:   
			return 1;
	}
	memcpy(sPlaintext,pEncData,16);
	TripleDes(sPlaintext,sKey,0);

	memcpy(pRandM,sPlaintext,16);
	

	switch(bType) //generate type
	{
		case 1: //Generate IMEK
			//?? write file
			break;
		case 5: //Generate IAEK
			//?? write file
			break;
			//?? read from file, bIndex
		default:   
			return 1;
	}
	return 0;
}



