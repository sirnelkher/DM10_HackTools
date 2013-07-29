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
//check param
    int i, fChr = '\0';
    if (argc != 2) {
        printf("Usage: %s <source>\n", argv[0]);
        return (-1);
    } /**/
//payload Alesis Sound Rom V1.03
    uint8_t syxUnlockKey1[] = { 0xF0, 0x00, 0x00, 0x0E, 0x2F, 0x0C, 0x5D, 0x11, 0x0B, 0x55, 0x78, 0x22, 0x6D, 0x1E, 0x34, 0x5D, 0x33, 0x4E, 0x64, 0x3B, 0x6A, 0x76, 0x7C, 0x0B, 0x28, 0x54, 0x37, 0x75, 0x67, 0x14, 0x45, 0x79, 0x08, 0x7B, 0x22, 0x72, 0x69, 0x1B, 0x33, 0x5F, 0x24, 0x12, 0x53, 0x63, 0x75, 0x6B, 0x2E, 0x79, 0x62, 0x44, 0x4A, 0x65, 0x5C, 0x6D, 0x1D, 0x1C, 0x15, 0x10, 0x42, 0x3E, 0x65, 0x12, 0x62, 0x58, 0x10, 0x5B, 0x64, 0x2D, 0x0A, 0x32, 0x43, 0x5A, 0x33, 0x40, 0x57, 0x7E, 0x32, 0x15, 0x53, 0x68, 0x21, 0x17, 0x7D, 0x4E, 0x21, 0x54, 0x0C, 0x1B, 0x22, 0x50, 0x17, 0x57, 0x38, 0x30, 0x78, 0x4D, 0x6E, 0x0C, 0x49, 0x18, 0x76, 0x27, 0x73, 0x7E, 0x07, 0x06, 0x6C, 0x1B, 0x2D, 0x38, 0x6C, 0x68, 0x70, 0x6F, 0x23, 0x3D, 0x46, 0x3D, 0x54, 0x64, 0x4E, 0x42, 0x12, 0x63, 0x36, 0x3C, 0x1E, 0x45, 0x36, 0x37, 0x17, 0x08, 0x06, 0x0B, 0x73, 0x6D, 0x55, 0x18, 0x5A, 0x74, 0x59, 0x5E, 0x1F, 0x72, 0x41, 0x71, 0x76, 0x6B, 0x24, 0x3B, 0x25, 0x79, 0x08, 0x08, 0x7E, 0x33, 0x4A, 0x74, 0x01, 0x70, 0x19, 0x01, 0x28, 0x11, 0x20, 0x69, 0x1C, 0x4D, 0x29, 0x2C, 0x17, 0x71, 0x65, 0x2F, 0x02, 0x17, 0x41, 0x14, 0x7B, 0x1E, 0x02, 0x0B, 0x74, 0x42, 0x50, 0x30, 0x44, 0x6E, 0x04, 0x1E, 0x15, 0x29, 0x06, 0x4F, 0x3C, 0x2D, 0x51, 0x2A, 0x57, 0x58, 0x60, 0x11, 0x78, 0x04, 0x6B, 0x57, 0x02, 0x6D, 0x6E, 0x2E, 0x05, 0x58, 0x20, 0x63, 0x61, 0x56, 0x53, 0x0B, 0x50, 0x66, 0x46, 0x53, 0x57, 0x06, 0x62, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x4D, 0xF7 };
    uint8_t syxUnlockKey2[] = { 0xF0, 0x00, 0x00, 0x0E, 0x2F, 0x0C, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x41, 0xF7 };
	uint8_t syxUnlockKeyDefault[] = { 0x60, 0x42, 0xF2, 0x07, 0xB8, 0x70, 0x54, 0x45 };
//payload SYX
    uint8_t syxUnlockHeader[] = { 0xF0, 0x00, 0x00, 0x0E, 0x2F, 0x0A, 0x40, 0x01, 0xF7 };
    uint8_t syxFooter[] = { 0xF0, 0x00, 0x00, 0x0E, 0x2F, 0x0B, 0xF7 };	
    uint8_t syxSoundRomHeader[] = { 0xF0, 0x00, 0x00, 0x0E, 0x2F, 0x0A, 0x68, 0x04, 0x49, 0x3F, 0xF7 };
    uint8_t syxPayloadPreamble[] = { 0xF0, 0x00, 0x00, 0x0E, 0x2F, 0x03 };
    uint32_t payloadAddress = 0x000A0000;
    uint8_t payloadCRC8;	
//CRC
    crcInit();
//payload buffer
    uint8_t syxPayload[256]; memset(&syxPayload, 0, sizeof(syxPayload));
    uint8_t syxPayloadLen = 0;
    uint8_t romPayload[256]; memset(&romPayload, 0, sizeof(romPayload));
    uint8_t romPayloadLen = 0;
//file source
    FILE *fSourceHandler;
    char fSourceName[256];
    snprintf(fSourceName, 256, "%s.rom", argv[1]);
    uint8_t fSourceBuffer[256]; memset(&fSourceBuffer, 0, sizeof(fSourceBuffer));
    uint8_t fSourceBufferLen = 0;
    if ((fSourceHandler = fopen(fSourceName, "r")) == NULL) {
        fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fSourceName, strerror(errno));
        return (-1);
    }
//file destiny
    FILE *fDestinyHandler;
    char fDestinyName[256];
    snprintf(fDestinyName, 256, "%s-Remix.syx", argv[1]);
    uint8_t fDestinyBuffer[256]; memset(&fDestinyBuffer, 0, sizeof(fDestinyBuffer));
    uint8_t fDestinyBufferLen = 0;
    uint32_t fDestinyCount = 0;
    if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL) {
        fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
        return (-1);
    } /**/
//Syx Unlock File
    for (i = 0; i < sizeof(syxUnlockHeader); i++) {
        fputc(syxUnlockHeader[i], fDestinyHandler);
    }
    for (i = 0; i < sizeof(syxUnlockKey1); i++) {
        fputc(syxUnlockKey1[i], fDestinyHandler);
    }
    for (i = 0; i < sizeof(syxUnlockKey2); i++) {
        fputc(syxUnlockKey2[i], fDestinyHandler);
    }
    for (i = 0; i < sizeof(syxFooter); i++) {
        fputc(syxFooter[i], fDestinyHandler);
    }
//Syx SoundRom File
    for (i = 0; i < sizeof(syxSoundRomHeader); i++) {
        fputc(syxSoundRomHeader[i], fDestinyHandler);
    } /**/
//Force to use SoundROM Keys, ignore first 8 bytes
	fseek( fSourceHandler, 8, SEEK_CUR );
	for (i = 0; i < 8; i++) {
		fSourceBuffer[fSourceBufferLen] = syxUnlockKeyDefault[i];
		fSourceBufferLen++;
		if ( fDestinyCount > 0 ) {
			payloadAddress++;
		}
	} /**/
//read file source
    while (!feof(fSourceHandler)) {
        fChr = fgetc(fSourceHandler);
        if (fChr != EOF) {
            fSourceBuffer[fSourceBufferLen] = fChr;
            fSourceBufferLen++;
            if ( fDestinyCount > 0 ) {
                payloadAddress++;
            }
        }
        //printf("\n > fSourceBufferLen[%d]: ", fSourceBufferLen);
        if ( fSourceBufferLen > 0 && ( fSourceBufferLen%192 == 0 || fChr == EOF ) ) {
        //Get ROM Bytes
            romPayloadLen = parseString(fSourceBuffer, 0, fSourceBufferLen, romPayload);
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
                fputc(syxPayloadPreamble[i], fDestinyHandler);
            }
            for (i = 0; i < syxPayloadLen; i++) {
                fputc(syxPayload[i], fDestinyHandler);
            }
            fputc(0xF7, fDestinyHandler);
        //Next
            fDestinyCount++;
            if ( fDestinyCount%10000 == 0 ) {
                printf("\n > Processed: %d KBytes", (fDestinyCount*239/1024));
            }
        //Clean Buffer
            memset(&romPayload, 0, sizeof(romPayload));
            memset(&syxPayload, 0, sizeof(syxPayload));
            memset(&fSourceBuffer, 0, sizeof(fSourceBuffer));
            fSourceBufferLen = 0;
            fChr = 0;
        }
    };
    for (i = 0; i < sizeof(syxFooter); i++) {
        fputc(syxFooter[i], fDestinyHandler);
    }
//Close
    printf("\n > Processed: %d KBytes", (fDestinyCount*239/1024) );
    fclose(fSourceHandler);
    fclose(fDestinyHandler);
//Finished
    printf("\n");
    return (0);
}
