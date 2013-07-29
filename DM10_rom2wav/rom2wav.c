/*************************************************************************************************************
 * Alesis DM10 - Encode Rom File to WAV File
 *
 * Author: Fabrício Dias, fmgdias@gmail.com
 *
 * Compile:
 *		cd ~/DM10/DM10_HackTools/DM10_rom2wav/ ;
 *		gcc rom2wav.c lib/cJSON.c -o rom2wav ;
 *		./rom2wav ../../syx/sound/DM10_sound_rom-V1.03 ;
 *************************************************************************************************************/
 
/* Bibliotecas */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "lib/cJSON.h"

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

/* Chunk */
struct romChunkStruct {
	uint32_t Index;
	uint32_t OffSet;
	uint32_t Length;
	char Name[14];
	uint32_t Address;
	uint32_t PayloadSize;
	uint32_t PayloadRate;
	uint16_t PayloadChannel;
};

/* Chunk Sort */
int compareChunkOffset(const void *v1, const void *v2) 
{ 
	struct romChunkStruct *_v1 = (struct romChunkStruct *) v1;
	struct romChunkStruct *_v2 = (struct romChunkStruct *) v2;
	return (int)( _v1->OffSet - _v2->OffSet );
}

/* Main */
int main(int argc, char **argv)
{
//check param
    uint32_t i = 0, ii = 0;
	uint32_t k = 0, kk = 0;
	uint32_t limitList = 10;
	int fChr = '\0';
    if (argc != 2) {
        printf("Usage: %s <source>\n", argv[0]);
        return (-1);
    }
	
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
    uint8_t fDestinyBuffer[256]; memset(&fDestinyBuffer, 0, sizeof(fDestinyBuffer));
    uint8_t fDestinyBufferLen = 0;
	
//Rom, Location 
	printf("\n > Rom, Location: %s", fSourceName );
//Rom, Total Of Bytes
	uint32_t romTotalOfBytes = 0;
	fseek(fSourceHandler, 0x0000, SEEK_SET );
	for ( i = 0; i<4; i++ ) {
		romTotalOfBytes |= fgetc(fSourceHandler) << 8*i;
	}
	printf("\n > Rom, Total Of Bytes: %lu Kbytes", (romTotalOfBytes/1024) );
	printf("\n");
	
//Rom, Chunk, Count
	uint32_t romChunkAddress = 0x0008;
	uint32_t romChunkCount = 0;
	fseek(fSourceHandler, romChunkAddress, SEEK_SET );
	for ( i = 0; i<4; i++ ) {
		romChunkCount |= fgetc(fSourceHandler) << 8*i;
	}	
	printf("\n > Rom, Chunk, Number Of Chunks: %lu", romChunkCount);
	
//Rom, Chunk, Retry List
	struct romChunkStruct *romChunk = malloc(romChunkCount*sizeof(struct romChunkStruct));
	for ( k = 0; k<romChunkCount; k++ ) {
		//Index
		romChunk[k].Index = k;
		//Offset
		for ( i = 0; i<4; i++ ) {
			romChunk[k].OffSet |= fgetc(fSourceHandler) << 8*i;
		}
		//Length
		for ( i = 0; i<4; i++ ) {
			romChunk[k].Length |= fgetc(fSourceHandler) << 8*i;
		}
		//Name
		for ( i = 0; i<12; i++ ) {
			romChunk[k].Name[i] = fgetc(fSourceHandler);
		}
		//Global Offset
		romChunk[k].Address = 0;
	}
//Rom, Chunk, Sort List
	qsort(romChunk, romChunkCount, sizeof(struct romChunkStruct), compareChunkOffset);
	for ( k = 0; k<romChunkCount; k++ ) {
		if ( k == 0 ) {
			romChunk[k].Address = romChunkAddress + 4 + 20*romChunkCount;
		} else {
			romChunk[k].Address = romChunk[k-1].Address + romChunk[k-1].Length;
		}
	}
//Rom, Chunk, Show List
	printf("\n > Rom, Chunk, List Of Chunks:\n\n");
	printf("%5s |", "Idx");
	printf("%13s |", "Chunk Name");	
	printf("%11s |", "OffSet");
	printf("%11s |", "Length");	
	printf("%11s |", "Address");
	printf("\n");
	for ( k = 0; k<romChunkCount; k++ ) {
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
	for ( k = 0; k<romChunkCount; k++ ) {
		if ( strcmp(romChunk[k].Name,"INST TBL") == 0 ) {
			romChunkInstTblAddress = romChunk[k].Address;
		}
	}
	if ( romChunkInstTblAddress == 0 ) {
		printf("\n > Error: Invalid Address for INST TBL: 0x%8.8X ", romChunkInstTblAddress);
		return -1;
	}
//Rom, Chunk, InstTbl, Count
	fseek(fSourceHandler, romChunkInstTblAddress, SEEK_SET );
	for ( i = 0; i<4; i++ ) {
		romChunkInstTblCount |= fgetc(fSourceHandler) << 8*i;
	}	
	printf("\n > Rom, Inst Tbl, Number Of Samples: %lu", romChunkInstTblCount);
//Rom, Chunk, InstTbl, Retry List
	struct romChunkStruct *romChunkInstTbl = malloc(romChunkInstTblCount*sizeof(struct romChunkStruct));
	for ( k = 0; k<romChunkInstTblCount; k++ ) {
		//Index
		romChunkInstTbl[k].Index = k;
		//Offset
		for ( i = 0; i<4; i++ ) {
			romChunkInstTbl[k].OffSet |= fgetc(fSourceHandler) << 8*i;
		}
		//Length
		for ( i = 0; i<4; i++ ) {
			romChunkInstTbl[k].Length |= fgetc(fSourceHandler) << 8*i;
		}
		//Name
		for ( i = 0; i<12; i++ ) {
			romChunkInstTbl[k].Name[i] = fgetc(fSourceHandler);
		}
		//Global Offset
		romChunkInstTbl[k].Address = 0;
	}
//Rom, Chunk, InstTbl, Sort List
	qsort(romChunkInstTbl, romChunkInstTblCount, sizeof(struct romChunkStruct), compareChunkOffset);
	for ( k = 0; k<romChunkInstTblCount; k++ ) {
		if ( k == 0 ) {
			romChunkInstTbl[k].Address = romChunkInstTblAddress + 4 + 20*romChunkInstTblCount;
		} else {
			romChunkInstTbl[k].Address = romChunkInstTbl[k-1].Address + romChunkInstTbl[k-1].Length;
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
	for ( k = 0; k<limitList && k<romChunkInstTblCount; k++ ) {
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
	for ( k = 0; k<romChunkCount; k++ ) {
		if ( strcmp(romChunk[k].Name,"PGMS") == 0 ) {
			romChunkPgmsAddress = romChunk[k].Address;
		}
	}
	if ( romChunkPgmsAddress == 0 ) {
		printf("\n > Error: Invalid Address for PGMS: 0x%8.8X ", romChunkPgmsAddress);
		return -1;
	}	
//Rom, Chunk, Pgms, Count
	fseek(fSourceHandler, romChunkPgmsAddress, SEEK_SET );
	for ( i = 0; i<4; i++ ) {
		romChunkPgmsCount |= fgetc(fSourceHandler) << 8*i;
	}
	if ( romChunkPgmsCount == 0 ) {
		for ( i = 0; i<4; i++ ) {
			romChunkPgmsAddress = romChunkPgmsAddress + 4;
			romChunkPgmsCount |= fgetc(fSourceHandler) << 8*i;
		}
	}
	printf("\n > Rom, Pgms, Number Of Samples: %lu", romChunkPgmsCount);
//Rom, Chunk, Pgms, Retry List
	struct romChunkStruct *romChunkPgms = malloc(romChunkPgmsCount*sizeof(struct romChunkStruct));
	for ( k = 0; k<romChunkPgmsCount; k++ ) {
		//Index
		romChunkPgms[k].Index = k;
		//Offset
		for ( i = 0; i<4; i++ ) {
			romChunkPgms[k].OffSet |= fgetc(fSourceHandler) << 8*i;
		}
		//Length
		for ( i = 0; i<4; i++ ) {
			romChunkPgms[k].Length |= fgetc(fSourceHandler) << 8*i;
		}
		//Name
		for ( i = 0; i<12; i++ ) {
			romChunkPgms[k].Name[i] = fgetc(fSourceHandler);
		}
		//Global Offset
		romChunkPgms[k].Address = 0;
	}
//Rom, Chunk, Pgms, Sort List
	qsort(romChunkPgms, romChunkPgmsCount, sizeof(struct romChunkStruct), compareChunkOffset);
	for ( k = 0; k<romChunkPgmsCount; k++ ) {
		if ( k == 0 ) {
			romChunkPgms[k].Address = romChunkPgmsAddress + 4 + 20*romChunkPgmsCount;
		} else {
			romChunkPgms[k].Address = romChunkPgms[k-1].Address + romChunkPgms[k-1].Length;
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
	for ( k = 0; k<limitList && k<romChunkPgmsCount; k++ ) {
		printf(" %4lu |", romChunkPgms[k].Index);
		printf(" %12.12s |", romChunkPgms[k].Name);		
		printf(" 0x%8.8X |", romChunkPgms[k].OffSet);
		printf(" 0x%8.8X |", romChunkPgms[k].Length);		
		printf(" 0x%8.8X |", romChunkPgms[k].Address);
		printf("\n");
	} /**/
	
//Rom, Chunk, SampTbl, Adress
	uint32_t romChunkSampTblAddress = 0;
	uint32_t romChunkSampTblCount = 0;
	for ( k = 0; k<romChunkCount; k++ ) {
		if ( strcmp(romChunk[k].Name,"SAMP TBL") == 0 ) {
			romChunkSampTblAddress = romChunk[k].Address;
		}
	}
	if ( romChunkSampTblAddress == 0 ) {
		printf("\n > Error: Invalid Address for SAMP TBL: 0x%8.8X ", romChunkSampTblAddress);
		return -1;
	}
//Rom, Chunk, SampTbl, Count
	fseek(fSourceHandler, romChunkSampTblAddress, SEEK_SET );
	for ( i = 0; i<4; i++ ) {
		romChunkSampTblCount |= fgetc(fSourceHandler) << 8*i;
	}	
	printf("\n > Rom, Samp Tbl, Number Of Samples: %lu", romChunkSampTblCount);
//Rom, Chunk, SampTbl, Retry List
	struct romChunkStruct *romChunkSampTbl = malloc(romChunkSampTblCount*sizeof(struct romChunkStruct));
	for ( k = 0; k<romChunkSampTblCount; k++ ) {
		//Index
		romChunkSampTbl[k].Index = k;
		//Offset
		for ( i = 0; i<4; i++ ) {
			romChunkSampTbl[k].OffSet |= fgetc(fSourceHandler) << 8*i;
		}
		//Length
		for ( i = 0; i<4; i++ ) {
			romChunkSampTbl[k].Length |= fgetc(fSourceHandler) << 8*i;
		}
		//Name
		for ( i = 0; i<12; i++ ) {
			romChunkSampTbl[k].Name[i] = fgetc(fSourceHandler);
		}
		//Global Offset
		romChunkSampTbl[k].Address = 0;
	}
//Rom, Chunk, SampTbl, Sort List
	qsort(romChunkSampTbl, romChunkSampTblCount, sizeof(struct romChunkStruct), compareChunkOffset);
	for ( k = 0; k<romChunkSampTblCount; k++ ) {
		if ( k == 0 ) {
			romChunkSampTbl[k].Address = romChunkSampTblAddress + 4 + 20*romChunkSampTblCount;
		} else {
			romChunkSampTbl[k].Address = romChunkSampTbl[k-1].Address + romChunkSampTbl[k-1].Length;
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
	for ( k = 0; k<limitList && k<romChunkSampTblCount; k++ ) {
		printf(" %4lu |", romChunkSampTbl[k].Index);
		printf(" %12.12s |", romChunkSampTbl[k].Name);		
		printf(" 0x%8.8X |", romChunkSampTbl[k].OffSet);
		printf(" 0x%8.8X |", romChunkSampTbl[k].Length);		
		//printf("   0x%8.8X |", romChunkSampTbl[k].Address);
		printf(" 0x%8.8X |", romChunkSampTbl[k].Address);
		printf("\n");
	} /**/
//Blue Jay
	printf("\n > Rom, Samp Tbl, Analyzing Data...\n");
	for ( k = 0; k<romChunkSampTblCount; k++ ) {
		fseek(fSourceHandler, romChunkSampTbl[k].Address, SEEK_SET );
		//PayloadSize
		for ( i = 0; i<4; i++ ) {
			romChunkSampTbl[k].PayloadSize |= fgetc(fSourceHandler) << 8*i;
		}
		//PayloadRate
		for ( i = 0; i<2; i++ ) {
			romChunkSampTbl[k].PayloadRate |= fgetc(fSourceHandler) << 8*i;
		}
		//PayloadChannel
		for ( i = 0; i<1; i++ ) {
			romChunkSampTbl[k].PayloadChannel |= fgetc(fSourceHandler) << 8*i;
		}
	}
	
//Rom, JSON Make
	printf("\n > Rom, Save Attributes...");
	cJSON *jsonObjRoot;
	jsonObjRoot = cJSON_CreateObject();	
		cJSON_AddItemToObject(jsonObjRoot, "RomLocation", cJSON_CreateString(fSourceName));
		cJSON_AddItemToObject(jsonObjRoot, "RomTotalOfBytes", cJSON_CreateNumber(romTotalOfBytes));
		cJSON_AddItemToObject(jsonObjRoot, "NumberOfChunks", cJSON_CreateNumber(romChunkCount));		
		cJSON *jsonArrayChunk, *jsonObjChunk;
		cJSON_AddItemToObject(jsonObjRoot, "ListOfChunks", jsonArrayChunk = cJSON_CreateArray());
		for ( k = 0; k<romChunkCount; k++ ) {
			cJSON_AddItemToArray(jsonArrayChunk, jsonObjChunk = cJSON_CreateObject());
				cJSON_AddNumberToObject(jsonObjChunk, "Index", romChunk[k].Index);
				cJSON_AddNumberToObject(jsonObjChunk, "OffSet", romChunk[k].OffSet);
				cJSON_AddNumberToObject(jsonObjChunk, "Length", romChunk[k].Length);
				cJSON_AddStringToObject(jsonObjChunk, "Name", romChunk[k].Name);
				cJSON_AddNumberToObject(jsonObjChunk, "Address", romChunk[k].Address);
			if ( strcmp(romChunk[k].Name,"INST TBL") == 0 ) {
				cJSON *jsonArrayInstTbl, *jsonObjInstTbl;
				cJSON_AddItemToObject(jsonObjChunk, "ListOfInstruments", jsonArrayInstTbl = cJSON_CreateArray());
					for ( kk = 0; kk<romChunkInstTblCount; kk++ ) {
						cJSON_AddItemToArray(jsonArrayInstTbl, jsonObjInstTbl = cJSON_CreateObject());
							cJSON_AddNumberToObject(jsonObjInstTbl, "Index", romChunkInstTbl[kk].Index);
							cJSON_AddNumberToObject(jsonObjInstTbl, "OffSet", romChunkInstTbl[kk].OffSet);
							cJSON_AddNumberToObject(jsonObjInstTbl, "Length", romChunkInstTbl[kk].Length);
							cJSON_AddStringToObject(jsonObjInstTbl, "Name", romChunkInstTbl[kk].Name);
							cJSON_AddNumberToObject(jsonObjInstTbl, "Address", romChunkInstTbl[kk].Address);
					}
			}
			if ( strcmp(romChunk[k].Name,"SAMP TBL") == 0 ) {
				cJSON *jsonArraySampTbl, *jsonObjSampTbl;
				cJSON_AddItemToObject(jsonObjChunk, "ListOfSamples", jsonArraySampTbl = cJSON_CreateArray());
					for ( kk = 0; kk<romChunkSampTblCount; kk++ ) {
						cJSON_AddItemToArray(jsonArraySampTbl, jsonObjSampTbl = cJSON_CreateObject());
							cJSON_AddNumberToObject(jsonObjSampTbl, "Index", romChunkSampTbl[kk].Index);
							cJSON_AddNumberToObject(jsonObjSampTbl, "OffSet", romChunkSampTbl[kk].OffSet);
							cJSON_AddNumberToObject(jsonObjSampTbl, "Length", romChunkSampTbl[kk].Length);
							cJSON_AddStringToObject(jsonObjSampTbl, "Name", romChunkSampTbl[kk].Name);
							cJSON_AddNumberToObject(jsonObjSampTbl, "Address", romChunkSampTbl[kk].Address);
							cJSON_AddNumberToObject(jsonObjSampTbl, "PayloadSize", romChunkSampTbl[kk].PayloadSize);
							cJSON_AddNumberToObject(jsonObjSampTbl, "PayloadRate", romChunkSampTbl[kk].PayloadRate);
							cJSON_AddNumberToObject(jsonObjSampTbl, "PayloadChannel", romChunkSampTbl[kk].PayloadChannel);
					}
			}
		}		
//Rom, JSON Save
	char *jsonTextChunk;
	jsonTextChunk = cJSON_Print(jsonObjRoot);
	snprintf(fDestinyName, 256, "%s/", argv[1]);
	mkdir(fDestinyName, 0777);
	snprintf(fDestinyName, 256, "%s/index.json", argv[1]);
	if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL) {
		fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
		return (-1);
	}
	fputs(jsonTextChunk, fDestinyHandler);
	fclose(fDestinyHandler);
	//printf("\n%s\n",jsonTextChunk);
	cJSON_Delete(jsonObjRoot);
	free(jsonTextChunk);
/**/	
	
//Rom, Chunk, InstTbl, Extract Files
	printf("\n\n > Rom, InstTbl, Creating Directorys...");
	snprintf(fDestinyName, 256, "%s/", argv[1]);
	mkdir(fDestinyName, 0777);
	snprintf(fDestinyName, 256, "%s/%s/", argv[1], "INSTTBL");
	mkdir(fDestinyName, 0777);
	printf("\n > Rom, InstTbl, Extracting Files to: %s", fDestinyName);
	for ( k = 0; k<romChunkInstTblCount; k++ ) {
	//Info
		fseek(fSourceHandler, romChunkInstTbl[k].Address, SEEK_SET );
		printf("\n > Rom, InstTbl, Extracting %d_%d_%.12s.rom, Length: %lu KBytes...", k, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name, (romChunkInstTbl[k].Length/1024) );
	//Create File ROM
		snprintf(fDestinyName, 256, "%s/INSTTBL/%d_%d_%.12s.rom", argv[1], k, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name);
		if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL) {
			fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
			return (-1);
		}
	//Raw Data
		for ( i = 0; i<romChunkInstTbl[k].Length; i++ ) {
			fChr = fgetc(fSourceHandler);
			if (fChr != EOF) {
				fputc( fChr, fDestinyHandler);
			} else {
				i = romChunkInstTbl[k].Length;
				k = romChunkInstTblCount;
			}
		}
	//Close
		fclose(fDestinyHandler);
	} /**/
	
//Rom, Chunk, Inst Tbl, Split Files
	uint32_t romChunkInstTblType = 0;
	uint32_t romChunkInstTblNotesCount = 0;
	printf("\n\n > Rom, Inst Tbl, Creating Directorys...");
	snprintf(fDestinyName, 256, "%s/", argv[1]);
	mkdir(fDestinyName, 0777);
	snprintf(fDestinyName, 256, "%s/%s/", argv[1], "INSTTBL");
	mkdir(fDestinyName, 0777);
	printf("\n > Rom, Inst Tbl, Spliting Files to: %s", fDestinyName);
	for ( k = 0; k<romChunkInstTblCount; k++ ) {
	//Info
		fseek(fSourceHandler, romChunkInstTbl[k].Address, SEEK_SET );
		romChunkInstTblType = fgetc(fSourceHandler);
		romChunkInstTblNotesCount = fgetc(fSourceHandler);
		snprintf(fDestinyName, 256, "%s/%s/Type_%d/", argv[1], "INSTTBL", romChunkInstTblType);
		mkdir(fDestinyName, 0777);
		snprintf(fDestinyName, 256, "%s/%s/Type_%d/%d_%.12s/", argv[1], "INSTTBL", romChunkInstTblType, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name);
		mkdir(fDestinyName, 0777);
		printf("\n > Rom, Inst Tbl, Extraction to: %s", fDestinyName);
	//Create File Header ROM
		fseek(fSourceHandler, romChunkInstTbl[k].Address, SEEK_SET );
		snprintf(fDestinyName, 256, "%s/INSTTBL/Type_%d/%d_%.12s/%d_%.12s_Header.rom", argv[1], romChunkInstTblType, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name);
		if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL) {
			fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
			return (-1);
		}
	//Raw Data
		for ( i = 0; i<20; i++ ) {
			fChr = fgetc(fSourceHandler);
			if (fChr != EOF) {
				fputc( fChr, fDestinyHandler);
			} else {
				i = romChunkInstTbl[k].Length;
				k = romChunkInstTblCount;
			}
			if ( i == 0 ) {
				romChunkInstTblType = 0;
				romChunkInstTblType |= fChr;
			}
			if ( i == 1 ) {
				romChunkInstTblNotesCount = 0;
				romChunkInstTblNotesCount |= fChr;
			}
		}
	
	//Split File Notes Rom
		for ( ii = 0; ii<romChunkInstTblNotesCount; ii++ ) {
		//Create File ROM
			snprintf(fDestinyName, 256, "%s/INSTTBL/Type_%d/%d_%.12s/%d_%.12s_Note_%d.rom", argv[1], romChunkInstTblType, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name, romChunkInstTbl[k].Index, romChunkInstTbl[k].Name, ii);
			if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL) {
				fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
				return (-1);
			}
		//Raw Data
			for ( i = 0; i<644; i++ ) {
				fChr = fgetc(fSourceHandler);
				if (fChr != EOF) {
					fputc( fChr, fDestinyHandler);
				} else {
					i = romChunkInstTbl[k].Length;
					k = romChunkInstTblCount;
				}
			}
		}
	//Close
		fclose(fDestinyHandler);
	} /**/	
	
//Rom, Chunk, Pgms, Extract Files
	printf("\n\n > Rom, Pgms, Creating Directorys...");
	snprintf(fDestinyName, 256, "%s/", argv[1]);
	mkdir(fDestinyName, 0777);
	snprintf(fDestinyName, 256, "%s/%s/", argv[1], "PGMS");
	mkdir(fDestinyName, 0777);
	printf("\n > Rom, Pgms, Extracting Files to: %s", fDestinyName);
	for ( k = 0; k<romChunkPgmsCount; k++ ) {
	//Info
		fseek(fSourceHandler, romChunkPgms[k].Address, SEEK_SET );
		printf("\n > Rom, Pgms, Extracting %d_%d_%.12s.rom, Length: %lu KBytes...", k, romChunkPgms[k].Index, romChunkPgms[k].Name, (romChunkPgms[k].Length/1024) );
	//Create File ROM
		snprintf(fDestinyName, 256, "%s/PGMS/%d_%d_%.12s.rom", argv[1], k, romChunkPgms[k].Index, romChunkPgms[k].Name);
		if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL) {
			fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
			return (-1);
		}
	//Raw Data
		for ( i = 0; i<romChunkPgms[k].Length; i++ ) {
			fChr = fgetc(fSourceHandler);
			if (fChr != EOF) {
				fputc( fChr, fDestinyHandler);
			} else {
				i = romChunkPgms[k].Length;
				k = romChunkPgmsCount;
			}
		}
	//Close
		fclose(fDestinyHandler);
	} /**/
	
//Wav Mono, 44.1 KHZ, 16 bits
	char romSampTblWavHeaderChunkId[] = { 'R', 'I', 'F', 'F' };
	char romSampTblWavHeaderChunkFormat[] = { 'W', 'A', 'V', 'E' };
	char romSampTblWavHeaderSubChunk1Id[] = { 'f', 'm', 't', ' ' } ;
	char romSampTblWavHeaderSubChunk1Size[] = { 0x10, 0x00, 0x00, 0x00 };
	char romSampTblWavHeaderSubChunk1CompressionCode[] = { 0x01, 0x00 }; // 0x0001-PCM
	char romSampTblWavHeaderSubChunk1NumberOfChannels[] = { 0x01, 0x00 }; // 0x01 - Mono, 0x02-Stereo
	char romSampTblWavHeaderSubChunk1SampleRate[] = { 0x44, 0xAC, 0x00, 0x00 }; // 44100Hz
	char romSampTblWavHeaderSubChunk1AverageBytesPerSecond[] = { 0x88, 0x58, 0x01, 0x00 }; // ByteRate = SampleRate * NumChannels * BitsPerSample/8
	char romSampTblWavHeaderSubChunk1BlockAlign[] = { 0x02, 0x00 }; //BlockAlign = NumChannels * BitsPerSample/8
	char romSampTblWavHeaderSubChunk1SignificantBitsPerSample[] = { 0x10, 0x00 }; //BitsPerSample = Number Of Bits To Define Each Sample: 8, 16, 24 or 32
	char romSampTblWavHeaderSubChunk2Id[] = { 'd', 'a', 't', 'a' };

	//char romSampTblWavHeaderSubChunk3Id[] = { 'L','I','S','T' };
	//char romSampTblWavHeaderSubChunk3Size[] = { 0x40, 0x00, 0x00, 0x00 } ;	
	//char romSampTblWavHeaderSubChunk3Info[] = { 'I','N','F','O' };
	//
	//char romSampTblWavHeaderSubChunk3Info1Key[] = { 'I','P','R','D' };
	//uint32_t romSampTblWavHeaderSubChunk3Info1Len = 5;
	//char romSampTblWavHeaderSubChunk3Info1Value[] = "DM10";	
	//
	//char romSampTblWavHeaderSubChunk3Info2Key[] = { 'I','A','R','T' };
	//uint32_t romSampTblWavHeaderSubChunk3Info2Len = 7;
	//char romSampTblWavHeaderSubChunk3Info2Value[] = "Alesis";
    //
	//char romSampTblWavHeaderSubChunk3Info3Key[] = { 'I','N','A','M' };
	//uint32_t romSampTblWavHeaderSubChunk3Info3Len = 8;
	//char romSampTblWavHeaderSubChunk3Info3Value[] = "wavXXXX";
	
//Rom, Chunk, SampTbl, Extract Wavs Files
	printf("\n\n > Rom, Samp Tbl, Creating Directorys...");
	snprintf(fDestinyName, 256, "%s/", argv[1]);
	mkdir(fDestinyName, 0777);
	snprintf(fDestinyName, 256, "%s/%s/", argv[1], "SAMPLTBL");
	mkdir(fDestinyName, 0777);
	printf("\n > Rom, Samp Tbl, Extracting Wavs Files to: %s", fDestinyName);
	for ( k = 0; k<romChunkSampTblCount; k++ ) {
	//Info
		fseek(fSourceHandler, romChunkSampTbl[k].Address, SEEK_SET );
		//fseek( fSourceHandler, 16, SEEK_CUR );
		printf("\n > Rom, Samp Tbl, Extracting %d_%d_%.12s.wav, Length: %lu KBytes...", k, romChunkSampTbl[k].Index, romChunkSampTbl[k].Name, (romChunkSampTbl[k].Length/1024) );
	//Create File Wav
		snprintf(fDestinyName, 256, "%s/SAMPLTBL/%d_%d_%.12s.wav", argv[1], k, romChunkSampTbl[k].Index, romChunkSampTbl[k].Name);
		if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL) {
			fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
			return (-1);
		}
	//Wav Header Chunk
		for (i = 0; i < sizeof(romSampTblWavHeaderChunkId); i++) {
			fputc(romSampTblWavHeaderChunkId[i], fDestinyHandler);
		}
		for ( i = 0; i<4; i++ ) {
			fputc((romChunkSampTbl[k].Length+36) >> 8*i, fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderChunkFormat); i++) {
			fputc(romSampTblWavHeaderChunkFormat[i], fDestinyHandler);
		}			
	//Wav Header SubChunk fmt
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1Id); i++) {
			fputc(romSampTblWavHeaderSubChunk1Id[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1Size); i++) {
			fputc(romSampTblWavHeaderSubChunk1Size[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1CompressionCode); i++) {
			fputc(romSampTblWavHeaderSubChunk1CompressionCode[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1NumberOfChannels); i++) {
			fputc(romSampTblWavHeaderSubChunk1NumberOfChannels[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1SampleRate); i++) {
			fputc(romSampTblWavHeaderSubChunk1SampleRate[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1AverageBytesPerSecond); i++) {
			fputc(romSampTblWavHeaderSubChunk1AverageBytesPerSecond[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1BlockAlign); i++) {
			fputc(romSampTblWavHeaderSubChunk1BlockAlign[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1SignificantBitsPerSample); i++) {
			fputc(romSampTblWavHeaderSubChunk1SignificantBitsPerSample[i], fDestinyHandler);
		}
	//Wav Header SubChunk data
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk2Id); i++) {
			fputc(romSampTblWavHeaderSubChunk2Id[i], fDestinyHandler);
		}
		for ( i = 0; i<4; i++ ) {
			fputc(romChunkSampTbl[k].Length >> 8*i, fDestinyHandler);
		}
	//Wav Raw Data		
		for ( i = 0; i<romChunkSampTbl[k].Length; i++ ) {
			fChr = fgetc(fSourceHandler);
			if (fChr != EOF) {
				fputc( fChr, fDestinyHandler);
			} else {
				i = romChunkSampTbl[k].Length;
				k = romChunkSampTblCount;
			}
		}	
	//Close
		fclose(fDestinyHandler);		
	/*/Create File Wav
		snprintf(fDestinyName, 256, "%s/SAMPLTBL/%d_%d_%.12s_Left.wav", argv[1], k, romChunkSampTbl[k].Index, romChunkSampTbl[k].Name);
		if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL) {
			fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
			return (-1);
		}
	//Wav Header Chunk
		for (i = 0; i < sizeof(romSampTblWavHeaderChunkId); i++) {
			fputc(romSampTblWavHeaderChunkId[i], fDestinyHandler);
		}
		for ( i = 0; i<4; i++ ) {
			fputc((romChunkSampTbl[k].Length+36) >> 8*i, fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderChunkFormat); i++) {
			fputc(romSampTblWavHeaderChunkFormat[i], fDestinyHandler);
		}			
	//Wav Header SubChunk fmt
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1Id); i++) {
			fputc(romSampTblWavHeaderSubChunk1Id[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1Size); i++) {
			fputc(romSampTblWavHeaderSubChunk1Size[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1CompressionCode); i++) {
			fputc(romSampTblWavHeaderSubChunk1CompressionCode[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1NumberOfChannels); i++) {
			fputc(romSampTblWavHeaderSubChunk1NumberOfChannels[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1SampleRate); i++) {
			fputc(romSampTblWavHeaderSubChunk1SampleRate[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1AverageBytesPerSecond); i++) {
			fputc(romSampTblWavHeaderSubChunk1AverageBytesPerSecond[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1BlockAlign); i++) {
			fputc(romSampTblWavHeaderSubChunk1BlockAlign[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1SignificantBitsPerSample); i++) {
			fputc(romSampTblWavHeaderSubChunk1SignificantBitsPerSample[i], fDestinyHandler);
		}
	//Wav Header SubChunk data
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk2Id); i++) {
			fputc(romSampTblWavHeaderSubChunk2Id[i], fDestinyHandler);
		}
		for ( i = 0; i<4; i++ ) {
			fputc(romChunkSampTbl[k].Length >> 8*i, fDestinyHandler);
		}
	//Wav Raw Data		
		for ( i = 0; i<romChunkSampTbl[k].PayloadSize*2; i++ ) {
			fChr = fgetc(fSourceHandler);
			if (fChr != EOF) {
				fputc( fChr, fDestinyHandler);
			} else {
				i = romChunkSampTbl[k].Length;
				k = romChunkSampTblCount;
			}
		}
	//Close
		fclose(fDestinyHandler);		
	//Create File Wav
		snprintf(fDestinyName, 256, "%s/SAMPLTBL/%d_%d_%.12s_Right.wav", argv[1], k, romChunkSampTbl[k].Index, romChunkSampTbl[k].Name);
		if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL) {
			fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
			return (-1);
		}
	//Wav Header Chunk
		for (i = 0; i < sizeof(romSampTblWavHeaderChunkId); i++) {
			fputc(romSampTblWavHeaderChunkId[i], fDestinyHandler);
		}
		for ( i = 0; i<4; i++ ) {
			fputc((romChunkSampTbl[k].Length+36) >> 8*i, fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderChunkFormat); i++) {
			fputc(romSampTblWavHeaderChunkFormat[i], fDestinyHandler);
		}			
	//Wav Header SubChunk fmt
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1Id); i++) {
			fputc(romSampTblWavHeaderSubChunk1Id[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1Size); i++) {
			fputc(romSampTblWavHeaderSubChunk1Size[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1CompressionCode); i++) {
			fputc(romSampTblWavHeaderSubChunk1CompressionCode[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1NumberOfChannels); i++) {
			fputc(romSampTblWavHeaderSubChunk1NumberOfChannels[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1SampleRate); i++) {
			fputc(romSampTblWavHeaderSubChunk1SampleRate[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1AverageBytesPerSecond); i++) {
			fputc(romSampTblWavHeaderSubChunk1AverageBytesPerSecond[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1BlockAlign); i++) {
			fputc(romSampTblWavHeaderSubChunk1BlockAlign[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1SignificantBitsPerSample); i++) {
			fputc(romSampTblWavHeaderSubChunk1SignificantBitsPerSample[i], fDestinyHandler);
		}
	//Wav Header SubChunk data
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk2Id); i++) {
			fputc(romSampTblWavHeaderSubChunk2Id[i], fDestinyHandler);
		}
		for ( i = 0; i<4; i++ ) {
			fputc(romChunkSampTbl[k].Length >> 8*i, fDestinyHandler);
		}
	//Wav Raw Data
		for ( i = 0; i<romChunkSampTbl[k].PayloadSize*2; i++ ) {
			fChr = fgetc(fSourceHandler);
			if (fChr != EOF) {
				fputc( fChr, fDestinyHandler);
			} else {
				i = romChunkSampTbl[k].Length;
				k = romChunkSampTblCount;
			}
		}		
	//Wav Footer
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Id); i++) {
			fputc(romSampTblWavHeaderSubChunk3Id[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Size); i++) {
			fputc(romSampTblWavHeaderSubChunk3Size[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info); i++) {
			fputc(romSampTblWavHeaderSubChunk3Info[i], fDestinyHandler);
		}
	//Wav Footer Info 1
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info1Key); i++) {
			fputc(romSampTblWavHeaderSubChunk3Info1Key[i], fDestinyHandler);
		}
		romSampTblWavHeaderSubChunk3Info1Len = sizeof(romSampTblWavHeaderSubChunk3Info1Value);
		for ( i = 0; i<4; i++ ) {
			fputc(romSampTblWavHeaderSubChunk3Info1Len >> 8*i, fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info1Value); i++) {
			fputc(romSampTblWavHeaderSubChunk3Info1Value[i], fDestinyHandler);
		}
	//Wav Footer Info 2
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info2Key); i++) {
			fputc(romSampTblWavHeaderSubChunk3Info2Key[i], fDestinyHandler);
		}
		romSampTblWavHeaderSubChunk3Info2Len = sizeof(romSampTblWavHeaderSubChunk3Info2Value);
		for ( i = 0; i<4; i++ ) {
			fputc(romSampTblWavHeaderSubChunk3Info2Len >> 8*i, fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info2Value); i++) {
			fputc(romSampTblWavHeaderSubChunk3Info2Value[i], fDestinyHandler);
		}
	//Wav Footer Info 3
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info3Key); i++) {
			fputc(romSampTblWavHeaderSubChunk3Info3Key[i], fDestinyHandler);
		}
		romSampTblWavHeaderSubChunk3Info3Len = sizeof(romSampTblWavHeaderSubChunk3Info3Value);
		for ( i = 0; i<4; i++ ) {
			fputc(romSampTblWavHeaderSubChunk3Info3Len >> 8*i, fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info3Value); i++) {
			fputc(romSampTblWavHeaderSubChunk3Info3Value[i], fDestinyHandler);
		}
	//Close
		fclose(fDestinyHandler);
	/**/
	} /**/
	
/*/Wav Stereo, 44.1 KHZ, 16 bits
	char romSampTblWavHeaderChunkId[] = { 'R', 'I', 'F', 'F' };
	char romSampTblWavHeaderChunkFormat[] = { 'W', 'A', 'V', 'E' };
	char romSampTblWavHeaderSubChunk1Id[] = { 'f', 'm', 't', ' ' } ;
	char romSampTblWavHeaderSubChunk1Size[] = { 0x10, 0x00, 0x00, 0x00 };
	char romSampTblWavHeaderSubChunk1CompressionCode[] = { 0x01, 0x00 }; // 0x0001-PCM
	char romSampTblWavHeaderSubChunk1NumberOfChannels[] = { 0x02, 0x00 }; // 0x01 - Mono, 0x02-Stereo
	char romSampTblWavHeaderSubChunk1SampleRate[] = { 0x44, 0xAC, 0x00, 0x00 }; // 44100Hz
	char romSampTblWavHeaderSubChunk1AverageBytesPerSecond[] = { 0x88, 0x58, 0x01, 0x00 }; // ByteRate = SampleRate * NumChannels * BitsPerSample/8
	char romSampTblWavHeaderSubChunk1BlockAlign[] = { 0x02, 0x00 }; //BlockAlign = NumChannels * BitsPerSample/8
	char romSampTblWavHeaderSubChunk1SignificantBitsPerSample[] = { 0x10, 0x00 }; //BitsPerSample = Number Of Bits To Define Each Sample: 8, 16, 24 or 32
	char romSampTblWavHeaderSubChunk2Id[] = { 'd', 'a', 't', 'a' };
	
	//char romSampTblWavHeaderSubChunk3Id[] = { 'L','I','S','T' };
	//char romSampTblWavHeaderSubChunk3Size[] = { 0x40, 0x00, 0x00, 0x00 } ;	
	//char romSampTblWavHeaderSubChunk3Info[] = { 'I','N','F','O' };
	//
	//char romSampTblWavHeaderSubChunk3Info1Key[] = { 'I','P','R','D' };
	//uint32_t romSampTblWavHeaderSubChunk3Info1Len = 5;
	//char romSampTblWavHeaderSubChunk3Info1Value[] = "DM10";	
	//
	//char romSampTblWavHeaderSubChunk3Info2Key[] = { 'I','A','R','T' };
	//uint32_t romSampTblWavHeaderSubChunk3Info2Len = 7;
	//char romSampTblWavHeaderSubChunk3Info2Value[] = "Alesis";
    //
	//char romSampTblWavHeaderSubChunk3Info3Key[] = { 'I','N','A','M' };
	//uint32_t romSampTblWavHeaderSubChunk3Info3Len = 8;
	//char romSampTblWavHeaderSubChunk3Info3Value[] = "wavXXXX";
	
//Rom, Chunk, SampTbl, Extract Wavs Files
	printf("\n\n > Rom, Samp Tbl, Creating Directorys...");
	snprintf(fDestinyName, 256, "%s/", argv[1]);
	mkdir(fDestinyName, 0777);
	snprintf(fDestinyName, 256, "%s/%s/", argv[1], "SAMPLTBL");
	mkdir(fDestinyName, 0777);
	printf("\n > Rom, Samp Tbl, Extracting Wavs Files to: %s", fDestinyName);
	for ( k = 0; k<romChunkSampTblCount; k++ ) {
	//Info
		fseek(fSourceHandler, romChunkSampTbl[k].Address, SEEK_SET );
		fseek( fSourceHandler, 16, SEEK_CUR );
		printf("\n > Rom, Samp Tbl, Extracting %d_%d_%.12s.wav, Length: %lu KBytes...", k, romChunkSampTbl[k].Index, romChunkSampTbl[k].Name, (romChunkSampTbl[k].Length/1024) );
	//Create File Wav
		snprintf(fDestinyName, 256, "%s/SAMPLTBL/%d_%d_%.12s.wav", argv[1], k, romChunkSampTbl[k].Index, romChunkSampTbl[k].Name);
		if ((fDestinyHandler = fopen(fDestinyName, "w")) == NULL) {
			fprintf(stderr, "\nError %d: Fail to open file %s: %s.\n", errno, fDestinyName, strerror(errno));
			return (-1);
		}
	//Wav Header Chunk
		for (i = 0; i < sizeof(romSampTblWavHeaderChunkId); i++) {
			fputc(romSampTblWavHeaderChunkId[i], fDestinyHandler);
		}
		for ( i = 0; i<4; i++ ) {
			fputc((romChunkSampTbl[k].Length+36) >> 8*i, fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderChunkFormat); i++) {
			fputc(romSampTblWavHeaderChunkFormat[i], fDestinyHandler);
		}			
	//Wav Header SubChunk fmt
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1Id); i++) {
			fputc(romSampTblWavHeaderSubChunk1Id[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1Size); i++) {
			fputc(romSampTblWavHeaderSubChunk1Size[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1CompressionCode); i++) {
			fputc(romSampTblWavHeaderSubChunk1CompressionCode[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1NumberOfChannels); i++) {
			fputc(romSampTblWavHeaderSubChunk1NumberOfChannels[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1SampleRate); i++) {
			fputc(romSampTblWavHeaderSubChunk1SampleRate[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1AverageBytesPerSecond); i++) {
			fputc(romSampTblWavHeaderSubChunk1AverageBytesPerSecond[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1BlockAlign); i++) {
			fputc(romSampTblWavHeaderSubChunk1BlockAlign[i], fDestinyHandler);
		}
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk1SignificantBitsPerSample); i++) {
			fputc(romSampTblWavHeaderSubChunk1SignificantBitsPerSample[i], fDestinyHandler);
		}
	//Wav Header SubChunk data
		for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk2Id); i++) {
			fputc(romSampTblWavHeaderSubChunk2Id[i], fDestinyHandler);
		}
		for ( i = 0; i<4; i++ ) {
			fputc(romChunkSampTbl[k].Length >> 8*i, fDestinyHandler);
		}
	//Wav Raw Data		
		for ( i = 0; i<romChunkSampTbl[k].PayloadSize*2; i=i+2 ) {
		//Seek Left Ch
			fseek(fSourceHandler, romChunkSampTbl[k].Address + 16 + i, SEEK_SET );
			fputc( fgetc(fSourceHandler), fDestinyHandler);
			fputc( fgetc(fSourceHandler), fDestinyHandler);
		//Seek Right Ch
			fseek(fSourceHandler, romChunkSampTbl[k].Address + 16 + romChunkSampTbl[k].PayloadSize*2 + i, SEEK_SET );
			fputc( fgetc(fSourceHandler), fDestinyHandler);
			fputc( fgetc(fSourceHandler), fDestinyHandler);			
		}
	//Wav Footer
	//	for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Id); i++) {
	//		fputc(romSampTblWavHeaderSubChunk3Id[i], fDestinyHandler);
	//	}
	//	for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Size); i++) {
	//		fputc(romSampTblWavHeaderSubChunk3Size[i], fDestinyHandler);
	//	}
	//	for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info); i++) {
	//		fputc(romSampTblWavHeaderSubChunk3Info[i], fDestinyHandler);
	//	}
	//Wav Footer Info 1
	//	for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info1Key); i++) {
	//		fputc(romSampTblWavHeaderSubChunk3Info1Key[i], fDestinyHandler);
	//	}
	//	romSampTblWavHeaderSubChunk3Info1Len = sizeof(romSampTblWavHeaderSubChunk3Info1Value);
	//	for ( i = 0; i<4; i++ ) {
	//		fputc(romSampTblWavHeaderSubChunk3Info1Len >> 8*i, fDestinyHandler);
	//	}
	//	for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info1Value); i++) {
	//		fputc(romSampTblWavHeaderSubChunk3Info1Value[i], fDestinyHandler);
	//	}
	//Wav Footer Info 2
	//	for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info2Key); i++) {
	//		fputc(romSampTblWavHeaderSubChunk3Info2Key[i], fDestinyHandler);
	//	}
	//	romSampTblWavHeaderSubChunk3Info2Len = sizeof(romSampTblWavHeaderSubChunk3Info2Value);
	//	for ( i = 0; i<4; i++ ) {
	//		fputc(romSampTblWavHeaderSubChunk3Info2Len >> 8*i, fDestinyHandler);
	//	}
	//	for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info2Value); i++) {
	//		fputc(romSampTblWavHeaderSubChunk3Info2Value[i], fDestinyHandler);
	//	}
	//Wav Footer Info 3
	//	for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info3Key); i++) {
	//		fputc(romSampTblWavHeaderSubChunk3Info3Key[i], fDestinyHandler);
	//	}
	//	romSampTblWavHeaderSubChunk3Info3Len = sizeof(romSampTblWavHeaderSubChunk3Info3Value);
	//	for ( i = 0; i<4; i++ ) {
	//		fputc(romSampTblWavHeaderSubChunk3Info3Len >> 8*i, fDestinyHandler);
	//	}
	//	for (i = 0; i < sizeof(romSampTblWavHeaderSubChunk3Info3Value); i++) {
	//		fputc(romSampTblWavHeaderSubChunk3Info3Value[i], fDestinyHandler);
	//	}		
	//Close
		fclose(fDestinyHandler);
		return 0;
	} /**/
	
//Finished
	printf("\n\n > Finished!");
    printf("\n");
    return (0);
}
