/**********************************************************************
 *
 * Filename:    crc.h
 * 
 * Description: A header file describing the various CRC standards.
 *
 * Notes:       
 *
 * 
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/

#ifndef _crc_h
#define _crc_h


#define FALSE	0
#define TRUE	!FALSE

/*
 * Select the CRC standard from the list that follows.
 */
#define CRC

#if defined(CRC)

typedef unsigned short  crc;

#define CRC_NAME			"CRC"
#define WIDTH 				   8
#define POLYNOMIAL			0x01
#define INITIAL_REMAINDER	0x00
#define FINAL_XOR_VALUE		0x00
#define REFLECT_DATA		TRUE
#define REFLECT_REMAINDER	TRUE

#else

#error "One of CRC must be #define'd."

#endif


void  crcInit(void);
crc   crcSlow(unsigned char const message[], int nBytes);
crc   crcFast(unsigned char const message[], int nBytes);

#endif /* _crc_h */
