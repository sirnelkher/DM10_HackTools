/*************************************************************************************************************
 * Alesis DM10 - Encode Rom File to Syx File
 *
 * Author: Fabrício Dias, fmgdias@gmail.com
 *
 * Compile:
 *		cd ~/DM10/DM10_HackTools/DM10_rom2syx/ ;
 *		gcc rom2syx.c lib/crc.c -o rom2syx ;
 *		./rom2syx ../../syx/sound/DM10_sound_rom-V1.03 ;
 *************************************************************************************************************/
 
/* Bibliotecas */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "lib/crc.h"

/* Parse String */
uint8_t parseString(uint8_t *_stringSource, uint8_t _stringPosition, uint8_t _stringLength, uint8_t *_stringDestination )
{
    uint8_t i = 0;
    while ( i < _stringLength ) {
        _stringDestination[i] = _stringSource[_stringPosition];
        _stringPosition++;
        i++;
    }
    return i;
}

/* Encode Payload */
uint8_t encodePayload(uint8_t *_stringSource, uint8_t _stringLength, uint8_t *_stringDestination)
{
    int i = 0, j = 0, b = 0;
    for ( i = 0; i < _stringLength; i++) {
        for ( j = 7; j >= 0; --j ) {
            //printf("\n\t > %d, i:%d, j:%d > 0x%.2X 0x%.2X 0x%.2X ", b, i, j, _stringSource[i], (_stringSource[i] & (1 << j)), (1 << (6 - (b % 7))) );
            if ( (_stringSource[i] & (1 << j)) > 0 ) {
                _stringDestination[b/7] |= (1 << (6 - (b % 7)));
            }
            b++;
        }
    }
    return ((_stringLength/7)*8);
}

/* Main */
int main(int argc, char **argv)
{
//Check
    int i, fChr = '\0';
    if (argc != 2) {
        printf("Usage: %s <ROM_NAME_WITHOUT_EXTENSION>\n", argv[0]);
        return (-1);
    } /**/
//File Syx Remix
    FILE *fRemixHandler;
    char fRemixName[256];
    snprintf(fRemixName, 256, "%s-Remix.syx", argv[1]);
    uint8_t fRemixBuffer[256]; memset(&fRemixBuffer, 0, sizeof(fRemixBuffer));
    uint8_t fRemixBufferLen = 0;
    uint32_t fRemixCount = 0;
	printf("\n > Remix, Location: %s", fRemixName );
    if ((fRemixHandler = fopen(fRemixName, "w")) == NULL) {
        fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fRemixName, strerror(errno));
        return (-1);
    }	
//File SYX
    FILE *fSyxHandler;
    char fSyxName[256];
    snprintf(fSyxName, 256, "%s.syx", argv[1]);
    uint8_t fSyxBuffer[256]; memset(&fSyxBuffer, 0, sizeof(fSyxBuffer));
    uint8_t fSyxBufferLen = 0;
	printf("\n > Syx, Location: %s", fSyxName );
    if ((fSyxHandler = fopen(fSyxName, "r")) == NULL) {
        fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fSyxName, strerror(errno));
        return (-1);
    }	
//File ROM
    FILE *fRomHandler;
    char fRomName[256];
    snprintf(fRomName, 256, "%s.rom", argv[1]);
    uint8_t fRomBuffer[256]; memset(&fRomBuffer, 0, sizeof(fRomBuffer));
    uint8_t fRomBufferLen = 0;
	printf("\n > Rom, Location: %s", fRomName );
    if ((fRomHandler = fopen(fRomName, "r")) == NULL) {
        fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fRomName, strerror(errno));
        return (-1);
    }
//Rom, Total Of Bytes
	long fRomTotalOfBytes = 0;
	fseek(fRomHandler, 0, SEEK_END);
	fRomTotalOfBytes = ftell(fRomHandler);
	fseek(fRomHandler, 0x0000, SEEK_SET );
	printf("\n > Rom, Total Of Bytes: %lu Kbytes", (fRomTotalOfBytes/1024) );
	if ( fRomTotalOfBytes == 133317221 ) {
		printf("\n > Rom, Alesis Sound Rom V1.03");
	} else if ( fRomTotalOfBytes == 65932817 ) {
		printf("\n > Rom, Alesis BlueJay V1.00");
	} else {
		printf("\n > Rom, Invalid Rom");
		return -1;
	}
//Copy HEADER SYX Instruction from SYX Original
	for ( i = 0; i <= 280; i++ ) {
        fputc(fgetc(fSyxHandler), fRemixHandler);
    } /**/
//payload SYX
	uint32_t payloadAddress = 0x000A0000;
    uint8_t syxPayloadPreamble[] = { 0xF0, 0x00, 0x00, 0x0E, 0x2F, 0x03 };
	uint8_t syxFooter[] = { 0xF0, 0x00, 0x00, 0x0E, 0x2F, 0x0B, 0xF7 };
//payload Buffer
    uint8_t syxPayload[256]; memset(&syxPayload, 0, sizeof(syxPayload));
    uint8_t syxPayloadLen = 0;
    uint8_t romPayload[256]; memset(&romPayload, 0, sizeof(romPayload));
    uint8_t romPayloadLen = 0;
/*/Debug BlueJay
	payloadAddress = 66587776;
	fseek(fRomHandler, 65932608, SEEK_SET );
/**/
//CRC
	uint8_t payloadCRC8;
    crcInit();
//Read File Rom
    while (!feof(fRomHandler)) {
	//Debug
		//if ( fRomBufferLen == 0 && payloadAddress>66587390 ) {
		//	printf("\n > Rom, Address: %lu, Seek: %lu", payloadAddress, ftell(fRomHandler) );
		//} else {
		//	//printf("\n >      Address: %lu, Seek: %lu", payloadAddress, ftell(fRomHandler) );
		//}
	//Get
        fChr = fgetc(fRomHandler);		
        if (fChr != EOF) {
            fRomBuffer[fRomBufferLen] = fChr;
            fRomBufferLen++;
            if ( fRemixCount > 0 || payloadAddress > 0x000A0000 ) {
                payloadAddress++;
            }
        }
	//Process
        //printf("\n > fRomBufferLen[%d]: ", fRomBufferLen);
        if ( fRomBufferLen > 0 && ( fRomBufferLen%192 == 0 || fChr == EOF ) ) {
        //Get ROM Bytes
            romPayloadLen = parseString(fRomBuffer, 0, fRomBufferLen, romPayload);
        //Set Address, Control, CRC
            if ( fChr != EOF ) {
            //Set Address Init
                for ( i = 0; i<4; i++ ) {
                    romPayload[romPayloadLen] = payloadAddress >> 8*i; romPayloadLen++;
                }
            //Set Address End
                for ( i = 0; i<4; i++ ) {
                    romPayload[romPayloadLen] = 0; romPayloadLen++;
                }
            //Type
                romPayload[romPayloadLen] = 0; romPayloadLen++;
                romPayload[romPayloadLen] = 5; romPayloadLen++;
            //CRC
                payloadCRC8 = crcFast((unsigned char *)romPayload, romPayloadLen);
                romPayload[romPayloadLen] = payloadCRC8; romPayloadLen++;
            } else {
			//SOUND ROM V1.03
				if ( fRomTotalOfBytes == 133317221 ) {
				//Set Address Init
					for ( i = 1; i<=4; i++ ) {
						romPayload[romPayloadLen] = 0; romPayloadLen++;
					}            
				//Set Address End
					for ( i = 1; i<4; i++ ) {
						if ( i == 1 ) {
							romPayload[romPayloadLen] = payloadAddress >> 8*i; romPayload[romPayloadLen]++; romPayloadLen++;
						} else {
							romPayload[romPayloadLen] = payloadAddress >> 8*i; romPayloadLen++;
						}
					}
					romPayload[romPayloadLen] = 0; romPayloadLen++;
				//Type                
					romPayload[romPayloadLen] = 0; romPayloadLen++;
					romPayload[romPayloadLen] = 2; romPayloadLen++;				
				}
			//BLUEJAY V1.0
				if ( fRomTotalOfBytes == 65932817 ) {
				//Set Address End
					for ( i = 1; i<4; i++ ) {
						if ( i == 1 ) {
							romPayload[romPayloadLen] = payloadAddress >> 8*i; romPayload[romPayloadLen]++; romPayloadLen++;
						} else {
							romPayload[romPayloadLen] = payloadAddress >> 8*i; romPayloadLen++;
						}
					}
				//Set Address Init
					for ( i = 1; i<=4; i++ ) {
						romPayload[romPayloadLen] = 0; romPayloadLen++;
					}    				
					romPayload[romPayloadLen] = 0; romPayloadLen++;
				//Type                
					romPayload[romPayloadLen] = 0; romPayloadLen++;
					romPayload[romPayloadLen] = 6; romPayloadLen++;					
				}
            //CRC
                payloadCRC8 = crcFast((unsigned char *)romPayload, romPayloadLen);
                romPayload[romPayloadLen] = payloadCRC8; romPayloadLen++;                
            }
        /*/Debug
            printf("\n\n > romPayload[%d]: ", romPayloadLen);
            for ( i = 0; i < romPayloadLen; i++) {
                if ( i%20 == 0 ) { printf("\n\t"); } printf("0x%.2X ", romPayload[i]);
            } /**/
        //Encode ROM (203 Bytes) to SYX (232 Byte)
            syxPayloadLen = encodePayload(romPayload, romPayloadLen, syxPayload);
        /*/Debug
            printf("\n > syxPayload[%d]: ", syxPayloadLen);
            for ( i = 0; i < syxPayloadLen; i++) {
                if ( i%20 == 0 ) { printf("\n\t"); } printf("0x%.2X ", syxPayload[i]);
            } /**/
        //Write, Preamble (6bytes) + Payload (232 Byte)
            for (i = 0; i < sizeof(syxPayloadPreamble); i++) {
                fputc(syxPayloadPreamble[i], fRemixHandler);
            }
            for (i = 0; i < syxPayloadLen; i++) {
                fputc(syxPayload[i], fRemixHandler);
            }
            fputc(0xF7, fRemixHandler);
        //Next
            fRemixCount++;
            if ( fRemixCount%10000 == 0 ) {
                printf("\n > Rom, Processed: %d KBytes", (fRemixCount*239/1024));
            }
        //Clean Buffer
            memset(&romPayload, 0, sizeof(romPayload));
            memset(&syxPayload, 0, sizeof(syxPayload));
            memset(&fRomBuffer, 0, sizeof(fRomBuffer));
            fRomBufferLen = 0;
            fChr = 0;
        }
    };
    for (i = 0; i < sizeof(syxFooter); i++) {
        fputc(syxFooter[i], fRemixHandler);
    }
//Close
    printf("\n > Rom, Processed: %d KBytes", (fRemixCount*239/1024) );
    fclose(fRomHandler);
    fclose(fRemixHandler);
//Finished
    printf("\n");
    return (0);
}
