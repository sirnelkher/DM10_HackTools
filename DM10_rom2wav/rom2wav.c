/*************************************************************************************************************
 * Alesis DM10 - Encode Rom File to WAV File
 *
 * Author: Fabrício Dias, fmgdias@gmail.com
 *
 * Compile:
 *      cd /Users/fmgdias/Dropbox/Desenvolvimento/DM10/DM10_HackTools/DM10_rom2wav/ ;
 *      gcc -w rom2wav.c lib/cJSON.c -o rom2wav && ./rom2wav ../../syx/sound/DM10_sound_rom-V1.03 ;
 *************************************************************************************************************/

/* Bibliotecas */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lib/cJSON.h"

/* Parse String */
uint8_t parseString(uint8_t *_stringSource, uint8_t _stringPosition, uint8_t _stringLength, uint8_t *_stringDestination )
{
    uint8_t i = 0;
    while ( i < _stringLength )
    {
        _stringDestination[i] = _stringSource[_stringPosition];
        _stringPosition++;
        i++;
    }
    return i;
}

/* Chunk */
struct romChunkStruct
{
    uint8_t Order;
    uint32_t Index;
    uint32_t OffSet;
    uint32_t Length;
    char Name[14];
    uint32_t Address;
    uint32_t HeaderOffset;
    uint32_t PayloadSize;
    uint32_t PayloadSampleRate;
    uint8_t PayloadNumberOfChannel;
    uint8_t S01;
    uint8_t S02;
    uint8_t S03;
    uint8_t S04;
    uint8_t S05;
    uint8_t S06;
    uint8_t S07;
    uint8_t S08;
    uint8_t S09;
    uint8_t S10;
};

/* Chunk Sort */
int compareChunkOrder(const void *v1, const void *v2)
{
    struct romChunkStruct *_v1 = (struct romChunkStruct *) v1;
    struct romChunkStruct *_v2 = (struct romChunkStruct *) v2;
    return (int)( _v1->Order - _v2->Order );
}
int compareChunkOffset(const void *v1, const void *v2)
{
    struct romChunkStruct *_v1 = (struct romChunkStruct *) v1;
    struct romChunkStruct *_v2 = (struct romChunkStruct *) v2;
    return (int)( _v1->OffSet - _v2->OffSet );
}

/* Instrument */
struct romInstNoteDynStruct
{
    uint8_t X01;
    uint8_t X02;
    uint8_t X03;
    uint8_t X04;
};
struct romInstNoteDynSampStruct
{
    uint16_t SampleIndex;
    uint8_t Y01;
    uint8_t Y02;
    uint8_t Y03;
    uint8_t Y04;
    uint8_t Y05;
    uint8_t Y06;
    uint8_t Y07;
    uint8_t Y08;
    uint8_t Y09;
    uint8_t Y10;
    uint8_t Y11;
    uint8_t Y12;
    uint8_t Y13;
    uint8_t Y14;
};

/* Main */
int main(int argc, char **argv)
{
    //temp
    uint8_t X[256]; memset(&X, 0, sizeof(X));
    uint8_t Y[256]; memset(&Y, 0, sizeof(Y));
    uint8_t Z[256]; memset(&Z, 0, sizeof(Y));
    //check param
    uint32_t i = 0, ii = 0, iii = 0;
    uint32_t k = 0, kk = 0;
    uint32_t limitList = 10;
    int fChr = '\0';
    if (argc != 2)
    {
        printf("Usage: %s <source>\n", argv[0]);
        return (-1);
    }
    //Auxiliar Pipe Buffer
    uint32_t fPipeRows = 256, fPipe1Len = 0, fPipe2Len = 0, fPipeOffset = 0, fPipeValidate = 0;
    uint8_t *fPipe1 = NULL, *fPipe2 = NULL;
    //File Destiny
    FILE *fDestinyHandler;
    char fDestinyName[256];
    uint8_t fDestinyBuffer[256]; memset(&fDestinyBuffer, 0, sizeof(fDestinyBuffer));
    uint8_t fDestinyBufferLen = 0;
    //File SampTbl
    FILE *fSampTblHandler;
    char fSampTblName[256];
    uint8_t fSampTblBuffer[256]; memset(&fSampTblBuffer, 0, sizeof(fSampTblBuffer));
    uint8_t fSampTblBufferLen = 0;
    //File InstTbl
    FILE *fInstTblHandler;
    char fInstTblName[256];
    uint8_t fInstTblBuffer[256]; memset(&fInstTblBuffer, 0, sizeof(fInstTblBuffer));
    uint8_t fInstTblBufferLen = 0;
    //File InstTblDyn
    FILE *fInstTblDynHandler;
    char fInstTblDynName[256];
    uint8_t fInstTblDynBuffer[256]; memset(&fInstTblDynBuffer, 0, sizeof(fInstTblDynBuffer));
    uint8_t fInstTblDynBufferLen = 0;
    //File ROM
    FILE *fRomHandler;
    char fRomName[256];
    snprintf(fRomName, 256, "%s.rom", argv[1]);
    uint8_t fRomBuffer[256]; memset(&fRomBuffer, 0, sizeof(fRomBuffer));
    uint8_t fRomBufferLen = 0;
    printf("\n > Rom, Location: %s", fRomName );
    if ((fRomHandler = fopen(fRomName, "r")) == NULL)
    {
        fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fRomName, strerror(errno));
        return (-1);
    }
    //Rom, Total Of Bytes
    long fRomTotalOfBytes = 0;
    fseek(fRomHandler, 0, SEEK_END);
    fRomTotalOfBytes = ftell(fRomHandler);
    fseek(fRomHandler, 0x0000, SEEK_SET );
    printf("\n > Rom, Total Of Bytes: %lu Kbytes", (fRomTotalOfBytes) );
    if ( fRomTotalOfBytes == 133317221 )
    {
        printf("\n > Rom, Alesis Sound Rom V1.03");
    }
    else if ( fRomTotalOfBytes == 65932817 )
    {
        printf("\n > Rom, Alesis BlueJay V1.00");
    }
    else
    {
        printf("\n > Rom, Invalid Rom");
        return -1;
    }
    //Rom, Total Of Bytes
    uint32_t romTotalOfBytes = 0;
    fseek(fRomHandler, 0x0000, SEEK_SET );
    for ( i = 0; i < 4; i++ )
    {
        romTotalOfBytes |= fgetc(fRomHandler) << 8 * i;
    }
    printf("\n");

    //Rom, Chunk, Count
    uint32_t romChunkAddress = 0x0008;
    uint32_t romChunkCount = 0;
    fseek(fRomHandler, romChunkAddress, SEEK_SET );
    for ( i = 0; i < 4; i++ )
    {
        romChunkCount |= fgetc(fRomHandler) << 8 * i;
    }
    printf("\n > Rom, Chunk, Number Of Chunks: %lu", romChunkCount);
    //Rom, Chunk, Retry List
    struct romChunkStruct *romChunk = malloc(romChunkCount * sizeof(struct romChunkStruct));
    for ( k = 0; k < romChunkCount; k++ )
    {
        //Order
        romChunk[k].Order = 0;
        //Index
        romChunk[k].Index = k;
        //Offset
        romChunk[k].OffSet = 0;
        for ( i = 0; i < 4; i++ )
        {
            romChunk[k].OffSet |= fgetc(fRomHandler) << 8 * i;
        }
        //Length
        romChunk[k].Length = 0;
        for ( i = 0; i < 4; i++ )
        {
            romChunk[k].Length |= fgetc(fRomHandler) << 8 * i;
        }
        //Name
        for ( i = 0; i < 12; i++ )
        {
            romChunk[k].Name[i] = fgetc(fRomHandler);
        }
        //Global Offset
        romChunk[k].Address = 0;
    }

    //Rom, Chunk, Sort List
    qsort(romChunk, romChunkCount, sizeof(struct romChunkStruct), compareChunkOffset);
    for ( k = 0; k < romChunkCount; k++ )
    {
        romChunk[k].Order = k;
        if ( k == 0 )
        {
            romChunk[k].Address = romChunkAddress + 4 + 20 * romChunkCount;
        }
        else
        {
            romChunk[k].Address = romChunk[k - 1].Address + romChunk[k - 1].Length;
        }
    }
    //Rom, Chunk, Show List
    printf("\n > Rom, Chunk, List Of Chunks:\n\n");
    printf("%5s |", "#");
    printf("%5s |", "Idx");
    printf("%13s |", "Chunk Name");
    printf("%11s |", "OffSet");
    printf("%11s |", "Length");
    printf("%11s |", "Address");
    printf("\n");
    for ( k = 0; k < romChunkCount; k++ )
    {
        printf(" %4lu |", romChunk[k].Order);
        printf(" %4lu |", romChunk[k].Index);
        printf(" %12.12s |", romChunk[k].Name);
        printf(" 0x%8.8X |", romChunk[k].OffSet);
        printf(" 0x%8.8X |", romChunk[k].Length);
        printf(" 0x%8.8X |", romChunk[k].Address);
        printf("\n");
    }

    //Rom, Chunk, InstTbl, Adress
    uint32_t romChunkInstTblAddress = 0;
    uint32_t romChunkInstTblCount = 0;
    for ( k = 0; k < romChunkCount; k++ )
    {
        if ( strcmp(romChunk[k].Name, "INST TBL") == 0 )
        {
            romChunkInstTblAddress = romChunk[k].Address;
        }
    }
    if ( romChunkInstTblAddress == 0 )
    {
        printf("\n > Error: Invalid Address for INST TBL: 0x%8.8X ", romChunkInstTblAddress);
        return -1;
    }
    //Rom, Chunk, InstTbl, Count
    fseek(fRomHandler, romChunkInstTblAddress, SEEK_SET );
    for ( i = 0; i < 4; i++ )
    {
        romChunkInstTblCount |= fgetc(fRomHandler) << 8 * i;
    }
    printf("\n > Rom, Inst Tbl, Number Of Samples: %lu", romChunkInstTblCount);
    //Rom, Chunk, InstTbl, Retry List
    struct romChunkStruct *romChunkInstTbl = malloc(romChunkInstTblCount * sizeof(struct romChunkStruct));
    for ( k = 0; k < romChunkInstTblCount; k++ )
    {
        //Index
        romChunkInstTbl[k].Index = k;
        //Offset
        romChunkInstTbl[k].OffSet = 0;
        for ( i = 0; i < 4; i++ )
        {
            romChunkInstTbl[k].OffSet |= fgetc(fRomHandler) << 8 * i;
        }
        //Length
        romChunkInstTbl[k].Length = 0;
        for ( i = 0; i < 4; i++ )
        {
            romChunkInstTbl[k].Length |= fgetc(fRomHandler) << 8 * i;
        }
        //Name
        for ( i = 0; i < 12; i++ )
        {
            romChunkInstTbl[k].Name[i] = fgetc(fRomHandler);
        }
        //Global Offset
        romChunkInstTbl[k].Address = 0;
    }
    //Rom, Chunk, InstTbl, Sort List
    qsort(romChunkInstTbl, romChunkInstTblCount, sizeof(struct romChunkStruct), compareChunkOffset);
    for ( k = 0; k < romChunkInstTblCount; k++ )
    {
        if ( k == 0 )
        {
            romChunkInstTbl[k].Address = romChunkInstTblAddress + 4 + 20 * romChunkInstTblCount;
        }
        else
        {
            romChunkInstTbl[k].Address = romChunkInstTbl[k - 1].Address + romChunkInstTbl[k - 1].Length;
        }
    }
    //Rom, Chunk, InstTbl, Print List
    printf("\n > Rom, Inst Tbl, List Of Samples:\n\n");
    printf("%5s |", "Idx");
    printf("%13s |", "Chunk Name");
    printf("%11s |", "OffSet");
    printf("%11s |", "Length");
    printf("%11s |", "Address");
    printf("\n");
    for ( k = 0; k < limitList && k < romChunkInstTblCount; k++ )
    {
        printf(" %4lu |", romChunkInstTbl[k].Index);
        printf(" %12.12s |", romChunkInstTbl[k].Name);
        printf(" 0x%8.8X |", romChunkInstTbl[k].OffSet);
        printf(" 0x%8.8X |", romChunkInstTbl[k].Length);
        printf(" 0x%8.8X |", romChunkInstTbl[k].Address);
        printf("\n");
    } /**/

    //Rom, Chunk, Pgms, Adress
    uint32_t romChunkPgmsAddress = 0;
    uint32_t romChunkPgmsCount = 0;
    for ( k = 0; k < romChunkCount; k++ )
    {
        if ( strcmp(romChunk[k].Name, "PGMS") == 0 )
        {
            romChunkPgmsAddress = romChunk[k].Address;
        }
    }
    if ( romChunkPgmsAddress == 0 )
    {
        printf("\n > Error: Invalid Address for PGMS: 0x%8.8X ", romChunkPgmsAddress);
        return -1;
    }
    //Rom, Chunk, Pgms, Count
    fseek(fRomHandler, romChunkPgmsAddress, SEEK_SET );
    for ( i = 0; i < 4; i++ )
    {
        romChunkPgmsCount |= fgetc(fRomHandler) << 8 * i;
    }
    if ( romChunkPgmsCount == 0 )
    {
        for ( i = 0; i < 4; i++ )
        {
            romChunkPgmsAddress = romChunkPgmsAddress + 4;
            romChunkPgmsCount |= fgetc(fRomHandler) << 8 * i;
        }
    }
    printf("\n > Rom, Pgms, Number Of Samples: %lu", romChunkPgmsCount);
    //Rom, Chunk, Pgms, Retry List
    struct romChunkStruct *romChunkPgms = malloc(romChunkPgmsCount * sizeof(struct romChunkStruct));
    for ( k = 0; k < romChunkPgmsCount; k++ )
    {
        //Index
        romChunkPgms[k].Index = k;
        //Offset
        romChunkPgms[k].OffSet = 0;
        for ( i = 0; i < 4; i++ )
        {
            romChunkPgms[k].OffSet |= fgetc(fRomHandler) << 8 * i;
        }
        //Length
        romChunkPgms[k].Length = 0;
        for ( i = 0; i < 4; i++ )
        {
            romChunkPgms[k].Length |= fgetc(fRomHandler) << 8 * i;
        }
        //Name
        for ( i = 0; i < 12; i++ )
        {
            romChunkPgms[k].Name[i] = fgetc(fRomHandler);
        }
        //Global Offset
        romChunkPgms[k].Address = 0;
    }
    //Rom, Chunk, Pgms, Sort List
    qsort(romChunkPgms, romChunkPgmsCount, sizeof(struct romChunkStruct), compareChunkOffset);
    for ( k = 0; k < romChunkPgmsCount; k++ )
    {
        if ( k == 0 )
        {
            romChunkPgms[k].Address = romChunkPgmsAddress + 4 + 20 * romChunkPgmsCount;
        }
        else
        {
            romChunkPgms[k].Address = romChunkPgms[k - 1].Address + romChunkPgms[k - 1].Length;
        }
    }
    //Rom, Chunk, Pgms, Print List
    printf("\n > Rom, Pgms, List Of Samples:\n\n");
    printf("%5s |", "Idx");
    printf("%13s |", "Chunk Name");
    printf("%11s |", "OffSet");
    printf("%11s |", "Length");
    printf("%11s |", "Address");
    printf("\n");
    for ( k = 0; k < limitList && k < romChunkPgmsCount; k++ )
    {
        printf(" %4lu |", romChunkPgms[k].Index);
        printf(" %12.12s |", romChunkPgms[k].Name);
        printf(" 0x%8.8X |", romChunkPgms[k].OffSet);
        printf(" 0x%8.8X |", romChunkPgms[k].Length);
        printf(" 0x%8.8X |", romChunkPgms[k].Address);
        printf("\n");
    } 
    /* */

    //Rom, Chunk, SampTbl, Adress
    uint32_t romChunkSampTblAddress = 0;
    uint32_t romChunkSampTblCount = 0;
    for ( k = 0; k < romChunkCount; k++ )
    {
        if ( strcmp(romChunk[k].Name, "SAMP TBL") == 0 )
        {
            romChunkSampTblAddress = romChunk[k].Address;
        }
    }
    if ( romChunkSampTblAddress == 0 )
    {
        printf("\n > Error: Invalid Address for SAMP TBL: 0x%8.8X ", romChunkSampTblAddress);
        return -1;
    }
    //Rom, Chunk, SampTbl, Count
    fseek(fRomHandler, romChunkSampTblAddress, SEEK_SET );
    for ( i = 0; i < 4; i++ )
    {
        romChunkSampTblCount |= fgetc(fRomHandler) << 8 * i;
    }
    printf("\n > Rom, Samp Tbl, Number Of Samples: %lu", romChunkSampTblCount);
    //Rom, Chunk, SampTbl, Retry List
    struct romChunkStruct *romChunkSampTbl = malloc(romChunkSampTblCount * sizeof(struct romChunkStruct));
    for ( k = 0; k < romChunkSampTblCount; k++ )
    {
        //Index
        romChunkSampTbl[k].Index = k;
        //Offset
        romChunkSampTbl[k].OffSet = 0;
        for ( i = 0; i < 4; i++ )
        {
            romChunkSampTbl[k].OffSet |= fgetc(fRomHandler) << 8 * i;
        }
        //Length
        romChunkSampTbl[k].Length = 0;
        for ( i = 0; i < 4; i++ )
        {
            romChunkSampTbl[k].Length |= fgetc(fRomHandler) << 8 * i;
        }
        //Name
        for ( i = 0; i < 12; i++ )
        {
            romChunkSampTbl[k].Name[i] = fgetc(fRomHandler);
        }
        //Global Offset
        romChunkSampTbl[k].Address = 0;
        //HeaderOffset
        romChunkSampTbl[k].HeaderOffset = 0;
        //PayloadSize
        romChunkSampTbl[k].PayloadSize = 0;
        //PayloadSampleRate
        romChunkSampTbl[k].PayloadSampleRate = 0;
        //PayloadNumberOfChannel
        romChunkSampTbl[k].PayloadNumberOfChannel = 0;
    }
    //Rom, Chunk, SampTbl, Sort List
    qsort(romChunkSampTbl, romChunkSampTblCount, sizeof(struct romChunkStruct), compareChunkOffset);
    for ( k = 0; k < romChunkSampTblCount; k++ )
    {
        if ( k == 0 )
        {
            romChunkSampTbl[k].Address = romChunkSampTblAddress + 4 + 20 * romChunkSampTblCount;
        }
        else
        {
            romChunkSampTbl[k].Address = romChunkSampTbl[k - 1].Address + romChunkSampTbl[k - 1].Length;
        }
    }
    //Rom, Chunk, SampTbl, Print List
    printf("\n > Rom, Samp Tbl, List Of Samples:\n\n");
    printf("%5s |", "Idx");
    printf("%13s |", "Chunk Name");
    printf("%11s |", "OffSet");
    printf("%11s |", "Length");
    printf("%11s |", "Address");
    printf("\n");
    for ( k = 0; k < limitList && k < romChunkSampTblCount; k++ )
    {
        printf(" %4lu |", romChunkSampTbl[k].Index);
        printf(" %12.12s |", romChunkSampTbl[k].Name);
        printf(" 0x%8.8X |", romChunkSampTbl[k].OffSet);
        printf(" 0x%8.8X |", romChunkSampTbl[k].Length);
        //printf("   0x%8.8X |", romChunkSampTbl[k].Address);
        printf(" 0x%8.8X |", romChunkSampTbl[k].Address);
        printf("\n");
    } /**/
    //Rom, Chunk, SampTbl, Analyzing Data
    printf("\n > Rom, Samp Tbl, Analyzing Data...");
    for ( k = 0; k < romChunkSampTblCount; k++ )
    {
        //Apontar para o Inicio do Raw Wav
        fseek(fRomHandler, romChunkSampTbl[k].Address, SEEK_SET );
        //Get All Wav Raw Data
        fPipeOffset = 0;
        fPipeValidate = 0;
        fPipeRows = romChunkSampTbl[k].Length;
        fPipe1 = malloc(fPipeRows * sizeof(uint8_t *)); memset(fPipe1, 0, sizeof(fPipe1));
        for ( i = 0; i < (romChunkSampTbl[k].Length); i++ )
        {
            fPipe1[i] = fgetc(fRomHandler);
        }
        //Search for Header Data
        for ( i = 12; fPipeValidate == 0 && i < (romChunkSampTbl[k].Length - 4); i++ )
        {
            if ( (fPipe1[i + 0] == 0x3C && fPipe1[i + 1] == 0x00 && fPipe1[i + 2] == 0x00 && fPipe1[i + 3] == 0x00) || (fPipe1[i + 0] == 0x48 && fPipe1[i + 1] == 0x00 && fPipe1[i + 2] == 0x00 && fPipe1[i + 3] == 0x00) )
            {
                fPipeOffset = i - 12;
                if ( fPipe1[fPipeOffset + 4] == 0x44 && fPipe1[fPipeOffset + 5] == 0xAC )
                {
                    //44.100 Hz
                    fPipeValidate = 1;
                }
                else if ( fPipe1[fPipeOffset + 4] == 0x00 && fPipe1[fPipeOffset + 5] == 0x7D )
                {
                    //32.000 Hz
                    fPipeValidate = 1;
                }
                else if ( fPipe1[fPipeOffset + 4] == 0x12 && fPipe1[fPipeOffset + 5] == 0x7A )
                {
                    //31.250 Hz
                    fPipeValidate = 1;
                }
                else if ( fPipe1[fPipeOffset + 4] == 0x30 && fPipe1[fPipeOffset + 5] == 0x75 )
                {
                    //30.000 Hz
                    fPipeValidate = 1;
                }
                else if ( fPipe1[fPipeOffset + 4] == 0x90 && fPipe1[fPipeOffset + 5] == 0x65 )
                {
                    //26.000 Hz
                    fPipeValidate = 1;
                }
                else if ( fPipe1[fPipeOffset + 4] == 0x22 && fPipe1[fPipeOffset + 5] == 0x56 )
                {
                    //22.050 Hz
                    fPipeValidate = 1;
                }
                else if ( fPipe1[fPipeOffset + 4] == 0x80 && fPipe1[fPipeOffset + 5] == 0x3E )
                {
                    //16.000 Hz
                    fPipeValidate = 1;
                }
                else
                {
                    fPipeValidate = 0;
                    fPipeOffset = 0;
                }
                if ( fPipeValidate == 1 )
                {
                    if ( fPipe1[fPipeOffset + 6] == 1 || fPipe1[fPipeOffset + 6] == 2 )
                    {
                        fPipeValidate = 1;
                    }
                    else
                    {
                        fPipeValidate = 0;
                    }
                }
            }
        }
        if ( fPipeValidate == 0 )
        {
            romChunkSampTbl[k].HeaderOffset = 0;
            romChunkSampTbl[k].PayloadSize = romChunkSampTbl[k].Length / 2;
            romChunkSampTbl[k].PayloadSampleRate = 0;
            romChunkSampTbl[k].PayloadNumberOfChannel = 0;
            /*printf("\n > Name: %12s, Index: %4lu, k: %4lu, fPipeOffset: %4lu,    ", romChunkSampTbl[k].Name, romChunkSampTbl[k].Index, k, fPipeOffset );
            printf("\n   ");
            for ( i = 0; i<(50); i++ ) {
                printf("%2.2X ", fPipe1[i]);
            }
            / **/
        }
        //Get Header Data
        if ( fPipeValidate == 1 )
        {
            fseek(fRomHandler, romChunkSampTbl[k].Address, SEEK_SET );
            fseek( fRomHandler, fPipeOffset, SEEK_CUR );
            //HeaderOffset
            romChunkSampTbl[k].HeaderOffset = fPipeOffset;
            //PayloadSize
            romChunkSampTbl[k].PayloadSize = 0;
            for ( i = 0; i < 4; i++ )
            {
                romChunkSampTbl[k].PayloadSize |= fgetc(fRomHandler) << 8 * i;
            }
            //PayloadSampleRate
            romChunkSampTbl[k].PayloadSampleRate = 0;
            for ( i = 0; i < 2; i++ )
            {
                romChunkSampTbl[k].PayloadSampleRate |= fgetc(fRomHandler) << 8 * i;
            }
            //PayloadNumberOfChannel
            romChunkSampTbl[k].PayloadNumberOfChannel = 0;
            for ( i = 0; i < 1; i++ )
            {
                romChunkSampTbl[k].PayloadNumberOfChannel |= fgetc(fRomHandler) << 8 * i;
            }
            //Unknow Bits
            for ( i = 1; i < 10; i++ )
            {
                switch (i)
                {
                case 1: romChunkSampTbl[k].S01 = fgetc(fRomHandler); break;
                case 2: romChunkSampTbl[k].S02 = fgetc(fRomHandler); break;
                case 3: romChunkSampTbl[k].S03 = fgetc(fRomHandler); break;
                case 4: romChunkSampTbl[k].S04 = fgetc(fRomHandler); break;
                case 5: romChunkSampTbl[k].S05 = fgetc(fRomHandler); break;
                case 6: romChunkSampTbl[k].S06 = fgetc(fRomHandler); break;
                case 7: romChunkSampTbl[k].S07 = fgetc(fRomHandler); break;
                case 8: romChunkSampTbl[k].S08 = fgetc(fRomHandler); break;
                case 9: romChunkSampTbl[k].S09 = fgetc(fRomHandler); break;
                }
            }
        }
    }

    //SampTbl, CSV SampTbl
    snprintf(fSampTblName, 256, "%s/SampTbl.csv", argv[1]);
    if ((fSampTblHandler = fopen(fSampTblName, "w")) == NULL)
    {
        fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fSampTblName, strerror(errno));
        return (-1);
    }
    printf("\n > Rom, Samp Tbl, CSV: %s\n", fSampTblName);
    fputs("IndexOfSample;NameOfSample;TotalOfBytes;WavAddress;HeaderOffset;PayloadSize;SampleRate;NumberOfChannel;S01;S02;S03;S04;S05;S06;S07;S08;S09;\n", fSampTblHandler);
    for ( k = 0; k < romChunkSampTblCount; k++ )
    {
        //CSV
        snprintf(fSampTblBuffer, 256, "%13d;%12s;%12d;%10d;%12d;%11d;%10d;%15d;%3d;%3d;%3d;%3d;%3d;%3d;%3d;%3d;%3d;\n", romChunkSampTbl[k].Index, romChunkSampTbl[k].Name, romChunkSampTbl[k].Length, romChunkSampTbl[k].Address, romChunkSampTbl[k].HeaderOffset, romChunkSampTbl[k].PayloadSize, romChunkSampTbl[k].PayloadSampleRate, romChunkSampTbl[k].PayloadNumberOfChannel, romChunkSampTbl[k].S01, romChunkSampTbl[k].S02, romChunkSampTbl[k].S03, romChunkSampTbl[k].S04, romChunkSampTbl[k].S05, romChunkSampTbl[k].S06, romChunkSampTbl[k].S07, romChunkSampTbl[k].S08, romChunkSampTbl[k].S09);
        fputs(fSampTblBuffer, fSampTblHandler);
    }
    fclose(fSampTblHandler);

    /*/Rom, Chunk, InstTbl, Extract Files
        printf("\n\n > Rom, InstTbl, Creating Directorys...");
        snprintf(fDestinyName, 256, "%s/", argv[1]);
        mkdir(fDestinyName, 0777);
        snprintf(fDestinyName, 256, "%s/%s/", argv[1], "INSTTBL");
        mkdir(fDestinyName, 0777);
        printf("\n > Rom, InstTbl, Extracting Files to: %s", fDestinyName);
        for ( k = 0; k<romChunkInstTblCount; k++ ) {
        //Info
            fseek(fRomHandler, romChunkInstTbl[k].Address, SEEK_SET );
            printf("\n > Rom, InstTbl, Extracting %d_%d_%.12s.rom, Length: %lu KBytes...", k, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name, (romChunkInstTbl[k].Length/1024) );
        //Create File ROM
            snprintf(fDestinyName, 256, "%s/INSTTBL/%d_%d_%.12s.rom", argv[1], k, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name);
            if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL) {
                fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
                return (-1);
            }
        //Raw Data
            for ( i = 0; i<romChunkInstTbl[k].Length; i++ ) {
                fChr = fgetc(fRomHandler);
                if (fChr != EOF) {
                    fputc( fChr, fDestinyHandler);
                } else {
                    i = romChunkInstTbl[k].Length;
                    k = romChunkInstTblCount;
                }
            }
        //Close
            fclose(fDestinyHandler);
        } 
    / **/

    //Rom, Chunk, Inst Tbl, Split Files
    uint32_t romChunkInstTblType = 0;
    uint32_t romChunkInstTblNotesCount = 0;
    printf("\n\n > Rom, Inst Tbl, Creating Directorys...");
    snprintf(fDestinyName, 256, "%s/", argv[1]);
    mkdir(fDestinyName, 0777);
    snprintf(fDestinyName, 256, "%s/%s/", argv[1], "INSTTBL");
    mkdir(fDestinyName, 0777);
    printf("\n > Rom, Inst Tbl, Spliting Files to: %s", fDestinyName);
    //CSV Instrument
    snprintf(fInstTblName, 256, "%s/InstTbl.csv", argv[1]);
    if ((fInstTblHandler = fopen(fInstTblName, "w")) == NULL)
    {
        fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fInstTblName, strerror(errno));
        return (-1);
    }
    printf("\n > Rom, Inst Tbl, CSV: %s", fInstTblName);
    fputs("IndexOfInstrument;NameOfInstrument;TypeOfInstrument;NumberOfNote;", fInstTblHandler);
    for ( i = 2; i < 20; i++ )
    {
        snprintf(fInstTblBuffer, 256, " I%.2d;", i);
        fputs(fInstTblBuffer, fInstTblHandler);
    }
    fputs("\n", fInstTblHandler);
    //CSV Instrument
    snprintf(fInstTblDynName, 256, "%s/InstTblDyn.csv", argv[1]);
    if ((fInstTblDynHandler = fopen(fInstTblDynName, "w")) == NULL)
    {
        fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fInstTblDynName, strerror(errno));
        return (-1);
    }
    printf("\n > Rom, Inst Tbl Dyn, CSV: %s\n", fInstTblDynName);
    fputs("IndexOfInstrument;IndexOfNote; X00; X01; X02; X03;", fInstTblDynHandler);
    for ( i = 0; i < 16; i++ )
    {
        snprintf(fInstTblDynBuffer, 256, " Y%.2d;", i);
        fputs(fInstTblDynBuffer, fInstTblDynHandler);
    }
    fputs("\n", fInstTblDynHandler);
    for ( k = 0; k < romChunkInstTblCount; k++ )
    {
        //Info
        fseek(fRomHandler, romChunkInstTbl[k].Address, SEEK_SET );
        romChunkInstTblType = fgetc(fRomHandler);
        romChunkInstTblNotesCount = fgetc(fRomHandler);
        //Create File Header ROM
        snprintf(fDestinyName, 256, "%s/%s/Type_%d/", argv[1], "INSTTBL", romChunkInstTblType);
        mkdir(fDestinyName, 0777);
        snprintf(fDestinyName, 256, "%s/%s/Type_%d/%d_%.12s/", argv[1], "INSTTBL", romChunkInstTblType, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name);
        mkdir(fDestinyName, 0777);
        //printf("\n > Rom, Inst Tbl, Extraction: %s", romChunkInstTbl[k].Name);
        fseek(fRomHandler, romChunkInstTbl[k].Address, SEEK_SET );
        snprintf(fDestinyName, 256, "%s/INSTTBL/Type_%d/%d_%.12s/%d_%.12s_Header.rom", argv[1], romChunkInstTblType, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name);
        if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL)
        {
            fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
            return (-1);
        }
        //CSV
        snprintf(fInstTblBuffer, 256, "%17d;%16s;", romChunkInstTbl[k].Index, romChunkInstTbl[k].Name);
        fputs(fInstTblBuffer, fInstTblHandler);
        //Raw Data
        for ( i = 0; i < 20; i++ )
        {
            fChr = fgetc(fRomHandler);
            fputc( fChr, fDestinyHandler);
            //CSV
            if ( i == 0 )
            {
                snprintf(fInstTblBuffer, 256, "%16d;", fChr);
            }
            else if ( i == 1 )
            {
                snprintf(fInstTblBuffer, 256, "%12d;", fChr);
            }
            else
            {
                snprintf(fInstTblBuffer, 256, "%4d;", fChr);
            }
            fputs(fInstTblBuffer, fInstTblHandler);
        }
        fputs("\n", fInstTblHandler);
        //Split File Notes Rom
        for ( iii = 0; iii < romChunkInstTblNotesCount; iii++ )
        {
            //Create File ROM
            fseek(fRomHandler, (romChunkInstTbl[k].Address + 20 + (644 * iii)), SEEK_SET );
            snprintf(fDestinyName, 256, "%s/INSTTBL/Type_%d/%d_%.12s/%d_%.12s_Note_%d.rom", argv[1], romChunkInstTblType, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name, iii);
            if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL)
            {
                fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
                return (-1);
            }
            //Raw Data
            memset(&X, 0, sizeof(X));
            for ( i = 0; i < 4; i++ )
            {
                fChr = fgetc(fRomHandler);
                fputc( fChr, fDestinyHandler);
                X[i] = fChr;
            }
            //Dynamic
            for ( ii = 0; ii < (40); ii++ )
            {
                //Dynamic Var
                memset(&Y, 0, sizeof(Y));
                for ( i = 0; i < (16); i++ )
                {
                    //GET
                    fChr = fgetc(fRomHandler);
                    fputc( fChr, fDestinyHandler);
                    Y[i] = fChr;
                }
                //CSV
                if ( ii < X[2] )
                {
                    snprintf(fInstTblDynBuffer, 256, "%17d;%11d;", romChunkInstTbl[k].Index, iii);
                    fputs(fInstTblDynBuffer, fInstTblDynHandler);
                    for ( i = 0; i < (4); i++ )
                    {
                        snprintf(fInstTblDynBuffer, 256, "%4d;", X[i]);
                        fputs(fInstTblDynBuffer, fInstTblDynHandler);
                    }
                    for ( i = 0; i < (16); i++ )
                    {
                        snprintf(fInstTblDynBuffer, 256, "%4d;", Y[i]);
                        fputs(fInstTblDynBuffer, fInstTblDynHandler);
                    }
                    fputs("\n", fInstTblDynHandler);
                }
            }
            //Close
            fclose(fDestinyHandler);
        }
    }
    fclose(fInstTblHandler);
    fclose(fInstTblDynHandler);
    /* */

    //Rom, JSON Make
    printf("\n > Rom, Save Attributes...");
    cJSON *jsonObjRoot;
    jsonObjRoot = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonObjRoot, "RomLocation", cJSON_CreateString(fRomName));
    cJSON_AddItemToObject(jsonObjRoot, "RomTotalOfBytes", cJSON_CreateNumber(romTotalOfBytes));
    cJSON_AddItemToObject(jsonObjRoot, "NumberOfChunks", cJSON_CreateNumber(romChunkCount));
    cJSON *jsonArrayChunk, *jsonObjChunk;
    cJSON_AddItemToObject(jsonObjRoot, "ListOfChunks", jsonArrayChunk = cJSON_CreateArray());
    for ( k = 0; k < romChunkCount; k++ )
    {
        cJSON_AddItemToArray(jsonArrayChunk, jsonObjChunk = cJSON_CreateObject());
        cJSON_AddNumberToObject(jsonObjChunk, "Index", romChunk[k].Index);
        cJSON_AddNumberToObject(jsonObjChunk, "OffSet", romChunk[k].OffSet);
        cJSON_AddNumberToObject(jsonObjChunk, "Length", romChunk[k].Length);
        cJSON_AddStringToObject(jsonObjChunk, "Name", romChunk[k].Name);
        cJSON_AddNumberToObject(jsonObjChunk, "Address", romChunk[k].Address);
        if ( strcmp(romChunk[k].Name, "INST TBL") == 0 )
        {
            cJSON *jsonArrayInstTbl, *jsonObjInstTbl;
            cJSON_AddItemToObject(jsonObjChunk, "ListOfInstruments", jsonArrayInstTbl = cJSON_CreateArray());
            for ( kk = 0; kk < romChunkInstTblCount; kk++ )
            {
                cJSON_AddItemToArray(jsonArrayInstTbl, jsonObjInstTbl = cJSON_CreateObject());
                cJSON_AddNumberToObject(jsonObjInstTbl, "Index", romChunkInstTbl[kk].Index);
                cJSON_AddNumberToObject(jsonObjInstTbl, "OffSet", romChunkInstTbl[kk].OffSet);
                cJSON_AddNumberToObject(jsonObjInstTbl, "Length", romChunkInstTbl[kk].Length);
                cJSON_AddStringToObject(jsonObjInstTbl, "Name", romChunkInstTbl[kk].Name);
                cJSON_AddNumberToObject(jsonObjInstTbl, "Address", romChunkInstTbl[kk].Address);
            }
        }
        if ( strcmp(romChunk[k].Name, "SAMP TBL") == 0 )
        {
            cJSON *jsonArraySampTbl, *jsonObjSampTbl;
            cJSON_AddItemToObject(jsonObjChunk, "ListOfSamples", jsonArraySampTbl = cJSON_CreateArray());
            for ( kk = 0; kk < romChunkSampTblCount; kk++ )
            {
                cJSON_AddItemToArray(jsonArraySampTbl, jsonObjSampTbl = cJSON_CreateObject());
                cJSON_AddNumberToObject(jsonObjSampTbl, "Index", romChunkSampTbl[kk].Index);
                cJSON_AddNumberToObject(jsonObjSampTbl, "OffSet", romChunkSampTbl[kk].OffSet);
                cJSON_AddNumberToObject(jsonObjSampTbl, "Length", romChunkSampTbl[kk].Length);
                cJSON_AddStringToObject(jsonObjSampTbl, "Name", romChunkSampTbl[kk].Name);
                cJSON_AddNumberToObject(jsonObjSampTbl, "Address", romChunkSampTbl[kk].Address);
                cJSON_AddNumberToObject(jsonObjSampTbl, "PayloadSize", romChunkSampTbl[kk].PayloadSize);
                cJSON_AddNumberToObject(jsonObjSampTbl, "PayloadSampleRate", romChunkSampTbl[kk].PayloadSampleRate);
                cJSON_AddNumberToObject(jsonObjSampTbl, "PayloadNumberOfChannel", romChunkSampTbl[kk].PayloadNumberOfChannel);
                //cJSON_AddNumberToObject(jsonObjSampTbl, "PayloadBitsPerSample", romChunkSampTbl[kk].PayloadBitsPerSample);
            }
        }
    }
    //Rom, JSON Save
    char *jsonTextChunk;
    jsonTextChunk = cJSON_Print(jsonObjRoot);
    snprintf(fDestinyName, 256, "%s/", argv[1]);
    mkdir(fDestinyName, 0777);
    snprintf(fDestinyName, 256, "%s/index.json", argv[1]);
    if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL)
    {
        fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
        return (-1);
    }
    fputs(jsonTextChunk, fDestinyHandler);
    fclose(fDestinyHandler);
    //printf("\n%s\n",jsonTextChunk);
    cJSON_Delete(jsonObjRoot);
    free(jsonTextChunk);
    /**/

    /*/Rom, Chunk, Pgms, Extract Files
        printf("\n\n > Rom, Pgms, Creating Directorys...");
        snprintf(fDestinyName, 256, "%s/", argv[1]);
        mkdir(fDestinyName, 0777);
        snprintf(fDestinyName, 256, "%s/%s/", argv[1], "PGMS");
        mkdir(fDestinyName, 0777);
        printf("\n > Rom, Pgms, Extracting Files to: %s", fDestinyName);
        for ( k = 0; k<romChunkPgmsCount; k++ ) {
        //Info
            fseek(fRomHandler, romChunkPgms[k].Address, SEEK_SET );
            //printf("\n > Rom, Pgms, Extracting %d_%d_%.12s.rom, Length: %lu KBytes...", k, romChunkPgms[k].Index, romChunkPgms[k].Name, (romChunkPgms[k].Length/1024) );
        //Create File ROM
            snprintf(fDestinyName, 256, "%s/PGMS/%d_%d_%.12s.rom", argv[1], k, romChunkPgms[k].Index, romChunkPgms[k].Name);
            if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL) {
                fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
                return (-1);
            }
        //Raw Data
            for ( i = 0; i<romChunkPgms[k].Length; i++ ) {
                fChr = fgetc(fRomHandler);
                if (fChr != EOF) {
                    fputc( fChr, fDestinyHandler);
                } else {
                    i = romChunkPgms[k].Length;
                    k = romChunkPgmsCount;
                }
            }
        //Close
            fclose(fDestinyHandler);
        }
    / **/

    //Extract Wav Files
    char romSampTblWavHeaderChunkId[] = { 'R', 'I', 'F', 'F' };
    char romSampTblWavHeaderChunkFormat[] = { 'W', 'A', 'V', 'E' };
    char romSampTblWavHeaderSubChunk1Id[] = { 'f', 'm', 't', ' ' } ;
    uint32_t romSampTblWavHeaderSubChunk1Size = 0x00000010;
    uint16_t romSampTblWavHeaderSubChunk1CompressionCode = 0x0001; // 0x0001-PCM
    uint16_t romSampTblWavHeaderSubChunk1NumberOfChannels = 0x0000; // 0x01 - Mono, 0x02-Stereo
    uint32_t romSampTblWavHeaderSubChunk1SampleRate = 0x00000000; // 44100Hz; 32000Hz
    uint32_t romSampTblWavHeaderSubChunk1AverageBytesPerSecond  = 0x00000000; // = romSampTblWavHeaderSubChunk1SampleRate * romSampTblWavHeaderSubChunk1NumberOfChannels * romSampTblWavHeaderSubChunk1SignificantBitsPerSample/8
    uint16_t romSampTblWavHeaderSubChunk1BlockAlign = 0x00; // = romSampTblWavHeaderSubChunk1NumberOfChannels * romSampTblWavHeaderSubChunk1SignificantBitsPerSample/8
    uint16_t romSampTblWavHeaderSubChunk1SignificantBitsPerSample = 0x0000; // = Number Of Bits To Define Each Sample: 8, 16, 24 or 32
    char romSampTblWavHeaderSubChunk2Id[] = { 'd', 'a', 't', 'a' };
    //Rom, Chunk, SampTbl, Extract Wavs Files
    printf("\n\n > Rom, Samp Tbl, Creating Directorys...");
    snprintf(fDestinyName, 256, "%s/", argv[1]);
    mkdir(fDestinyName, 0777);
    snprintf(fDestinyName, 256, "%s/%s/", argv[1], "SAMPLTBL");
    mkdir(fDestinyName, 0777);
    printf("\n > Rom, Samp Tbl, Extracting Wavs Files to: %s", fDestinyName);
    for ( k = 0; k < romChunkSampTblCount; k++ )
    {
        //Create File Wav
        //printf("\n > Rom, Samp Tbl, Prepare %.12s.wav, Length: %lu KBytes...", romChunkSampTbl[k].Name, (romChunkSampTbl[k].Length/1024) );
        snprintf(fDestinyName, 256, "%s/SAMPLTBL/%.12s.wav", argv[1], romChunkSampTbl[k].Name);
        if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL)
        {
            fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
            return (-1);
        }
        //Apontar para o Inicio do Raw Wav
        fseek(fRomHandler, romChunkSampTbl[k].Address, SEEK_SET );
        //Skip Header in Raw Wav
        fseek( fRomHandler, romChunkSampTbl[k].HeaderOffset + 16, SEEK_CUR );
        //Config Header WAV
        romSampTblWavHeaderSubChunk1SignificantBitsPerSample = 0x0010; //16 bits;
        if ( romChunkSampTbl[k].PayloadNumberOfChannel > 0 )
        {
            romSampTblWavHeaderSubChunk1NumberOfChannels = romChunkSampTbl[k].PayloadNumberOfChannel;
        }
        else
        {
            romSampTblWavHeaderSubChunk1NumberOfChannels = 1; //Mono
        }
        if ( romChunkSampTbl[k].PayloadSampleRate > 0 )
        {
            romSampTblWavHeaderSubChunk1SampleRate = romChunkSampTbl[k].PayloadSampleRate;
        }
        else
        {
            romSampTblWavHeaderSubChunk1SampleRate = 44100; //44.100Hz
        }
        //Get All Wav Raw Data
        fPipeRows = romChunkSampTbl[k].Length;
        fPipe1 = malloc(fPipeRows * sizeof(uint8_t *)); memset(fPipe1, 0, sizeof(fPipe1));
        fPipe2 = malloc(fPipeRows * sizeof(uint8_t *)); memset(fPipe2, 0, sizeof(fPipe2));
        for ( i = 0; i < (romChunkSampTbl[k].Length) && i < (romChunkSampTbl[k].PayloadSize) * 2; i++ )
        {
            fPipe1[i] = fgetc(fRomHandler);
        }
        if ( romSampTblWavHeaderSubChunk1NumberOfChannels >= 2 )
        {
            for ( i = 0; i < (romChunkSampTbl[k].Length) && i < (romChunkSampTbl[k].PayloadSize) * 2; i++ )
            {
                fPipe2[i] = fgetc(fRomHandler);
            }
        }
        //Wav Header Chunk
        for (i = 0; i < sizeof(romSampTblWavHeaderChunkId); i++)
        {
            fputc(romSampTblWavHeaderChunkId[i], fDestinyHandler);
        }
        for ( i = 0; i < 4; i++ )
        {
            fputc( (romChunkSampTbl[k].PayloadSize * 2 * romSampTblWavHeaderSubChunk1NumberOfChannels + 36) >> 8 * i, fDestinyHandler);
        }
        for (i = 0; i < sizeof(romSampTblWavHeaderChunkFormat); i++)
        {
            fputc(romSampTblWavHeaderChunkFormat[i], fDestinyHandler);
        }
        //Wav Header SubChunk fmt
        for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1Id); i++)
        {
            fputc(romSampTblWavHeaderSubChunk1Id[i], fDestinyHandler);
        }
        for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1Size); i++)
        {
            fputc(romSampTblWavHeaderSubChunk1Size >> 8 * i, fDestinyHandler);
        }
        for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1CompressionCode); i++)
        {
            fputc(romSampTblWavHeaderSubChunk1CompressionCode >> 8 * i, fDestinyHandler);
        }
        for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1NumberOfChannels); i++)
        {
            fputc(romSampTblWavHeaderSubChunk1NumberOfChannels >> 8 * i, fDestinyHandler);
        }
        for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1SampleRate); i++)
        {
            fputc(romSampTblWavHeaderSubChunk1SampleRate >> 8 * i, fDestinyHandler);
        }
        romSampTblWavHeaderSubChunk1AverageBytesPerSecond = romSampTblWavHeaderSubChunk1SampleRate * romSampTblWavHeaderSubChunk1NumberOfChannels * romSampTblWavHeaderSubChunk1SignificantBitsPerSample / 8;
        for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1AverageBytesPerSecond); i++)
        {
            fputc(romSampTblWavHeaderSubChunk1AverageBytesPerSecond >> 8 * i, fDestinyHandler);
        }
        romSampTblWavHeaderSubChunk1BlockAlign = romSampTblWavHeaderSubChunk1NumberOfChannels * romSampTblWavHeaderSubChunk1SignificantBitsPerSample / 8;
        for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1BlockAlign); i++)
        {
            fputc(romSampTblWavHeaderSubChunk1BlockAlign >> 8 * i, fDestinyHandler);
        }
        for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1SignificantBitsPerSample); i++)
        {
            fputc(romSampTblWavHeaderSubChunk1SignificantBitsPerSample >> 8 * i, fDestinyHandler);
        }
        //Wav Header SubChunk data
        for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk2Id); i++)
        {
            fputc(romSampTblWavHeaderSubChunk2Id[i], fDestinyHandler);
        }
        for ( i = 0; i < 4; i++ )
        {
            fputc( (romChunkSampTbl[k].PayloadSize * 2 * romSampTblWavHeaderSubChunk1NumberOfChannels) >> 8 * i, fDestinyHandler);
        }
        //Wav Raw Data
        fPipe1Len = 0; fPipe2Len = 0;
        for ( i = 0; i < (romChunkSampTbl[k].Length) && i < (romChunkSampTbl[k].PayloadSize); i++ )
        {
            fputc( fPipe1[fPipe1Len++], fDestinyHandler);
            fputc( fPipe1[fPipe1Len++], fDestinyHandler);
            if ( romSampTblWavHeaderSubChunk1NumberOfChannels >= 2 )
            {
                fputc( fPipe2[fPipe2Len++], fDestinyHandler);
                fputc( fPipe2[fPipe2Len++], fDestinyHandler);
            }
        }
        //Close
        fclose(fDestinyHandler);
    }

    //Finished
    printf("\n\n > Finished!");
    printf("\n");
    return (0);
}
