/*************************************************************************************************************
 * Alesis DM10 - Decode Syx File to Rom Payload File
 *
 * Author: Fabrício Dias, fmgdias@gmail.com
 *
 * Compile: 
 *		cd ~/DM10/DM10_HackTools/DM10_syx2payload/
 *		gcc syx2payload.c -o syx2payload ;
 *		./syx2payload ../../syx/sound/DM10_sound_rom-V1.03 ;
 *************************************************************************************************************/

/* Bibliotecas */
#include <stdio.h>
#include <errno.h>

/* Parse Syx String */
uint8_t parseSyx(uint8_t *_stringSource, uint8_t _stringPosition, uint8_t _stringLength, uint8_t *_stringDestination)
{
    uint8_t i = 0;
    while ( i < _stringLength && _stringSource[_stringPosition] != 0xF7 ) {
        _stringDestination[i] = _stringSource[_stringPosition];
        _stringPosition++;
        i++;
    }
    return i;
}

/* Decode Payload */
uint8_t decodePayload(uint8_t *_stringSource, uint8_t _stringLength, uint8_t *_stringDestination)
{
    int i = 0, j = 0, b = 0;
    for ( i = 0; i < _stringLength; i++) {
        for ( j = 6; j >= 0; --j ) {
            if ( (_stringSource[i] & (1 << j)) > 0 ) {
                _stringDestination[b/8] |= (1 << (7 - (b % 8)));
            }
            b++;
        }
    }
    return ((_stringLength/8)*7);
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
//payload param
    uint8_t syxPayload[256]; memset(&syxPayload, 0, sizeof(syxPayload));
    uint8_t syxPayloadLen = 0;
    uint8_t romPayload[256]; memset(&romPayload, 0, sizeof(romPayload));
    uint8_t romPayloadLen = 0;
//file source
    FILE *fSourceHandler;
    char fSourceName[256];
    snprintf(fSourceName, 256, "%s.syx", argv[1]);
    uint8_t fSourceBuffer[256]; memset(&fSourceBuffer, 0, sizeof(fSourceBuffer));
    uint8_t fSourceBufferLen = 0;
    if ((fSourceHandler = fopen(fSourceName, "r")) == NULL) {
        fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fSourceName, strerror(errno));
        return (-1);
    }
//file destiny
    FILE *fDestinyHandler;
    char fDestinyName[256];
    snprintf(fDestinyName, 256, "%s.payload", argv[1]);
    uint8_t fDestinyBuffer[256]; memset(&fDestinyBuffer, 0, sizeof(fDestinyBuffer));
    uint8_t fDestinyBufferLen = 0;
    uint32_t fDestinyCount = 0;
    if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL) {
        fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
        return (-1);
    }
//read file source
    while (!feof(fSourceHandler)) {
        fChr = fgetc(fSourceHandler);
        if (fChr != EOF) {
            fSourceBuffer[fSourceBufferLen] = fChr;
            fSourceBufferLen++;
        }
        if ( fChr == 0xF7 ) {
            if( fSourceBuffer[0] == 0xF0 && fSourceBuffer[1] == 0x00 && fSourceBuffer[2] == 0x00 && fSourceBuffer[3] == 0x0E && fSourceBuffer[5] == 0x03 ) {
            //Skip 6 bytes, Get 232 Bytes
                syxPayloadLen = parseSyx(fSourceBuffer, 6, 232, syxPayload);
            /*/Debug
                printf("\n > syxPayload[%d]: ", syxPayloadLen);
                for ( i = 0; i < syxPayloadLen; i++) {
                    if ( i%20 == 0 ) { printf("\n\t"); } printf("0x%.2X ", syxPayload[i]);
                } /**/
            //Decode Payload (232 Bytes) to Payload (203 Byte)
                romPayloadLen = decodePayload(syxPayload, syxPayloadLen, romPayload);
            /*/Debug
                printf("\n > romPayload[%d]: ", romPayloadLen);
                for ( i = 0; i < romPayloadLen; i++) {
                    if ( i%20 == 0 ) { printf("\n\t"); } printf("0x%.2X ", romPayload[i]);
                } /**/
            //Write, Data (192 bytes) + Address (8 bytes) + Control (2 bytes) +Checksum (1 byte)
                for (i = 0; i < romPayloadLen; i++) {
                    fputc(romPayload[i], fDestinyHandler);
                } 
            /**/
            //Next
                fDestinyCount++;
                if ( fDestinyCount%10000 == 0 ) {
                    printf("\n > Processed: %d KBytes", (fDestinyCount*203/1024));
                }
            }
        //Clean Buffer
            memset(&romPayload, 0, sizeof(romPayload));
            memset(&syxPayload, 0, sizeof(syxPayload));
            memset(&fSourceBuffer, 0, sizeof(fSourceBuffer));
            fSourceBufferLen = 0;
        }
    };
    printf("\n > Processed: %d KBytes", (fDestinyCount*203/1024));
    fclose(fSourceHandler);
    fclose(fDestinyHandler);
//Finished
    printf("\n");
    return (0);
}
