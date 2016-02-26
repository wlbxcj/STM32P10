#ifndef AFX_GLOBAL_H
#define AFX_GLOBAL_H

#ifdef AFX_VARIABLES
    #define EXTERN
#else
    #define EXTERN extern 
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>


//#include "coredef.h"
#include "struct.h"
//#include "poslib.h"
#include "Func.h"
#include "Def.h"
#include "OperCmd.h"

#include "EmvLib.h"
#include "qpboc.h"
#include "PED.h"



#endif

