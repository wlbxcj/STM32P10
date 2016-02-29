#ifndef _PCI_CMD_H
#define _PCI_CMD_H 

#define PCIREADVER_CMD          0x90
#define PCIGETRND_CMD           0x91
#define PCIACCESSAUTH_CMD       0x92
#define PCIWRITEPINMKEY_CMD     0x93
#define PCIWRITEPINKEY_CMD      0x94
#define PCIWRITEMACKEY_CMD      0x95
#define PCIWRITEMACMKEY_CMD     0x96
#define PCIWRITEDESKEY_CMD      0x97
//#define PCIDERIVEPINKEY_CMD     0x97
#define PCIDERIVEMACKEY_CMD     0x98
#define PCIDERIVEDESKEY_CMD     0x99
#define PCIGETPIN_CMD           0x9A
#define PCIGETMAC_CMD           0x9B
#define PCIDES_CMD              0x9C
#define PCIOFFLINEPLAINPIN_CMD  0x9D
#define PCIOFFLINEENCPIN_CMD    0x9E
//#define PCIGETPINAUTO_CMD       0x9F
#define PCIGETOFFLINEPIN_CMD    0x9F
#define DUKPTGETPIN_CMD         0xA0
#define DUKPTGETMAC_CMD         0xA1 

#define PCIGETPINFIXK_CMD       0xB0


#define PCI_LockedErr        (0x01)
#define PCI_KeyTypeErr       (0x02)
#define PCI_KeyLrcErr        (0x03)
#define PCI_KeyNoErr         (0x04)
#define PCI_KeyLenErr        (0x05)
#define PCI_KeyModeErr       (0x06) 
#define PCI_InputLenErr      (0x07)
#define PCI_InputCancelErr   (0x08)
#define PCI_InputNotMatchErr (0x09)
#define PCI_InputTimeOutErr  (0x0a)
#define PCI_CallTimeInteErr  (0x0b) 
#define PCI_NoKeyErr         (0x0c)
#define PCI_WriteKeyErr      (0x0d)
#define PCI_ReadKeyErr       (0x0e)
#define PCI_RsaKeyHashErr    (0x0f)
#define PCI_DataLenErr       (0x10) 
#define PCI_NoInputErr       (0x11)
#define PCI_AppNumOverErr    (0x12) 
#define PCI_ReadMMKErr       (0x13)
#define PCI_WriteMMKErr      (0x14)  
#define PCI_AuthErr          (0x15)
#define PCI_RsaKeyErr        (0x16)
#define PCI_KeySameErr       (0x17)


#define DUKPT_NoKeyErr				(0x20)
#define DUKPT_CounterOverFlowErr	(0x21)
#define DUKPT_NoEmptyListErr		(0x22)
#define DUKPT_InvalidAppNoErr		(0x23)
#define DUKPT_InvalidKeyIDErr		(0x24)
#define DUKPT_InvalidFutureKeyIDErr	(0x25)
#define DUKPT_InvalidCrcErr			(0x26)
#define DUKPT_InvalidBDKErr			(0x27)
#define DUKPT_InvalidKSNErr			(0x28)
#define DUKPT_InvalidModeErr		(0x29)
#define DUKPT_NotFoundErr			(0x2A)



#endif