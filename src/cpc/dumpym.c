/*
 *  Arnold emulator (c) Copyright, Kevin Thacker 1995-2015
 *
 *  This file is part of the Arnold emulator source code distribution.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "dumpym.h"
#include "cpcglob.h"
#include "cpcendian.h"
#include "host.h"
#include "headers.h"
#include "psg.h"
#include <stdint.h>

static unsigned char *YM5_SongName = NULL;
static unsigned char *YM5_AuthorName = NULL;
static unsigned char *YM5_Comments = NULL;
static AY_3_8912 *pPSGToDump = NULL;

const char *YM5_EndFileText ="End!";
const char *YM3_Ident_Text = "YM3!";
const char *YM5_Ident_Text = "YM5!";
const char *YM5_IdentString_Text ="LeOnArD!";
const char *LHA_CompressMethod="-lh0-";
const char *YMFilenameInLHA = "song.ym";
const unsigned long YMFilenameInLHALength = 7;

#define YM5_SONG_ATTRIBUTE_DATA_INTERLEAVED	0x0001
/* gcc and microsoft compilers */
#pragma pack(1)
typedef struct
{
	uint32_t ID;
	uint8_t IDString[8];
	uint32_t NumVBL;
	uint32_t SongAttributes;
	uint16_t NoOfDigiDrumSamples;
	uint32_t YMFrequency;
	uint16_t PlayerFrequency;
	uint32_t VBLLoopIndex;
	uint16_t SizeOfExtraData;
} YM5_HEADER; 
#pragma pack()

/* 140k could be used to store 3 minutes of raw ay data */
/* may be worth doing that instead of a temp file? */

static int YMOutput_Version = YMOUTPUT_VERSION_3;
static BOOL YMOutput_Enabled = FALSE;
static BOOL YMOutput_Recording = FALSE;

static BOOL YMOutput_RecordWhenSilenceEnds = FALSE;
static BOOL YMOutput_StopRecordWhenSilenceBegins = FALSE;

void    YMOutput_SetRecordWhenSilenceEnds(BOOL bState)
{
    YMOutput_RecordWhenSilenceEnds = bState;
}

void    YMOutput_SetVersion(int nVersion)
{
    if ((nVersion!=YMOUTPUT_VERSION_3) && (nVersion!=YMOUTPUT_VERSION_5))
    {
        nVersion = YMOUTPUT_VERSION_5;

    }
    YMOutput_Version = nVersion;
}

int YMOutput_GetVersion(void)
{

    return YMOutput_Version;
}

void    YMOutput_SetStopRecordwhenSilenceBegins(BOOL bState)
{
    YMOutput_StopRecordWhenSilenceBegins = bState;
}

BOOL    YMOutput_GetRecordWhenSilenceEnds(void)
{
    return YMOutput_RecordWhenSilenceEnds;
}

BOOL    YMOutput_GetStopRecordwhenSilenceBegins(void)
{
    return YMOutput_StopRecordWhenSilenceBegins;
}


static unsigned char *pRawBuffer;
static unsigned long RawBufferSize;
static unsigned char *pRawBufferPtr;
static int nVBL;

int YMOutput_GetVBL(void)
{
	return nVBL;
}

/* need to write data into temp buffer */
BOOL YMOutput_IsEnabled(void)
{
    return YMOutput_Enabled;
}

BOOL YMOutput_IsRecording(void)
{
    return YMOutput_Recording;
}


unsigned char *YMOutput_GetName(void)
{
	return YM5_SongName;
}

unsigned char *YMOutput_GetAuthor(void)
{
	return YM5_AuthorName;
}

unsigned char *YMOutput_GetComment(void)
{
	return YM5_Comments;
}


void	YMOutput_SetName(unsigned char *pNewName)
{
	int StringLength;

	if (pNewName == NULL)
		return;

	if (YM5_SongName!=NULL)
	{
		free(YM5_SongName);
		YM5_SongName = NULL;
	}

	StringLength = strlen((const char *)pNewName);

	if (StringLength==0)
		return;

	YM5_SongName = (unsigned char *)malloc(StringLength+1);

	if (YM5_SongName!=NULL)
	{
		memcpy(YM5_SongName, pNewName, StringLength+1);
	}
}

void	YMOutput_SetAuthor(unsigned char *pNewAuthor)
{
	int StringLength;

	if (pNewAuthor == NULL)
		return;

	if (YM5_AuthorName!=NULL)
	{
		free(YM5_AuthorName);
		YM5_AuthorName = NULL;
	}

	StringLength = strlen((const char *)pNewAuthor);

	if (StringLength==0)
		return;

	YM5_AuthorName = (unsigned char *)malloc(StringLength+1);

	if (YM5_AuthorName!=NULL)
	{
		memcpy(YM5_AuthorName, pNewAuthor, StringLength+1);
	}
}

void	YMOutput_SetComment(unsigned char *pNewComment)
{
	int StringLength;

	if (pNewComment == NULL)
		return;

	if (YM5_Comments!=NULL)
	{
		free(YM5_Comments);
		YM5_Comments = NULL;
	}

	StringLength = strlen((const char *)pNewComment);

	if (StringLength==0)
		return;

	YM5_Comments = (unsigned char *)malloc(StringLength+1);

	if (YM5_Comments!=NULL)
	{
		memcpy(YM5_Comments, pNewComment, StringLength+1);
	}
}

void    YMOutput_Init(void)
{
    pRawBuffer = NULL;
}


BOOL	YMOutput_StartRecording(BOOL bRecordWhenSilenceEnds, BOOL bStopRecordWhenSilenceBegins)
{
    YMOutput_StopRecordWhenSilenceBegins = bStopRecordWhenSilenceBegins;
    YMOutput_RecordWhenSilenceEnds = bRecordWhenSilenceEnds;
    YMOutput_Recording = FALSE;

    if (pRawBuffer!=NULL)
    {
        free(pRawBuffer);
    }

    RawBufferSize =
        /* frames per second */
        50*
        /* YM bytes per frame */
        16*
        /* seconds in a minute */
        60*
        /* minutes */
        3;

    pRawBuffer = (unsigned char *)malloc(RawBufferSize);

    if (pRawBuffer!=NULL)
    {
        pRawBufferPtr = pRawBuffer;
        YMOutput_Enabled = TRUE;
        nVBL = 0;
        return TRUE;
    }

    return FALSE;
}

void	YMOutput_Finish(void)
{
	if (YM5_SongName!=NULL)
	{
		free(YM5_SongName);
		YM5_SongName = NULL;
	}

	if (YM5_AuthorName!=NULL)
	{
		free(YM5_AuthorName);
		YM5_AuthorName = NULL;
	}

	if (YM5_Comments!=NULL)
	{
		free(YM5_Comments);
		YM5_Comments = NULL;
	}

	if (pRawBuffer!=NULL)
	{
	    free(pRawBuffer);
	    pRawBuffer = NULL;
	}
}

void YMOutput_SetPSGToDump(AY_3_8912 *pPSG)
{
  pPSGToDump = pPSG;
}

/* TRUE if output is silent, FALSE if not */
BOOL YMOutput_IsSilent(void)
{
	/* what defines silence? */

	/* - if R8, R9 and R10 are all zero, then no noise or tone will be output (noise and
	    tone can be active or inactive)
	   - if tone or noise are not active, we will not get any sound provided that R8,R9,R10
	do not change.
	   - if tone is active, but tone is in a specific in-audible range, it will be silent.
	   R8, R9, R10 can be any value.
	*/
  if (pPSGToDump==NULL)
      return TRUE;
  
	if ((PSG_GetRegisterData(pPSGToDump, 8)==0) &&
        (PSG_GetRegisterData(pPSGToDump, 9)==0) && (PSG_GetRegisterData(pPSGToDump, 10)==0))
	{
		/* volume regs are all zero = silence regardless of mixer,
		tone and noise settings. */
		return TRUE;
	}

	/* a volume register is not zero, something *could* be audible */
	if (
		(!(
		(PSG_GetFlags(pPSGToDump, 8)&AY_REG_UPDATED) |
		(PSG_GetFlags(pPSGToDump, 9)&AY_REG_UPDATED) |
		(PSG_GetFlags(pPSGToDump, 10)&AY_REG_UPDATED)
		)) &&
		((PSG_GetRegisterData(pPSGToDump, 7) & 0x03f)==0x03f))
	{
		/* AY Regs haven't changed, and tone and noise are disabled */
		return TRUE;
	}

	return FALSE;
}

/* generate a temporary record  */
void	YMOutput_GenerateTempRecord(unsigned char *Regs)
{
	int i;

	for (i=0; i<13; i++)
	{
    if (pPSGToDump==NULL)
    {
        Regs[i] = 0;
    }
    else
    {
      Regs[i] = PSG_GetRegisterData(pPSGToDump, i);
    }
  }

	if ((pPSGToDump==NULL) || ((PSG_GetFlags(pPSGToDump,13) & AY_REG_UPDATED)==0))
	{
		Regs[13] = 0x0ff;
	}
	else
	{
		Regs[13] = PSG_GetRegisterData(pPSGToDump,13);
	}



	for (i=14; i<16; i++)
	{
	    Regs[i] = 0;
	}

}


int		YMOutput_ValidateVersion(int Version)
{
	if ((Version!=YMOUTPUT_VERSION_3) && (Version!=YMOUTPUT_VERSION_5))
	{
		Version = YMOUTPUT_VERSION_3;
	}

	return Version;
}

/* calculate size of header for YM file */
unsigned long YMOutput_GenerateHeaderOutputSize(int nVersion)
{
	unsigned long nSize;

	nSize = 0;

	if (nVersion==YMOUTPUT_VERSION_3)
	{
		nSize = 4;
	}
	else if (nVersion == YMOUTPUT_VERSION_5)
	{
		/* version 5 */
		nSize = sizeof(YM5_HEADER);

		/* write name of song */
		if (YM5_SongName!=NULL)
		{
			nSize += strlen((char *) YM5_SongName);
		}
		nSize++;

		/* write author name */
		if (YM5_AuthorName!=NULL)
		{
			nSize += strlen((char *) YM5_AuthorName);
		}
		nSize++;

		if (YM5_Comments!=NULL)
		{
			nSize += strlen((char *) YM5_Comments);
		}
		nSize++;
	}

	return nSize;
}

unsigned long YMOutput_GenerateTrailerOutputSize(int nVersion)
{
	unsigned long nSize;

	nSize = 0;

	if (nVersion == YMOUTPUT_VERSION_5)
	{
		/* version 5 */
		nSize = 4;
	}

	return nSize;
}

/* setup header */
void YMOutput_GenerateHeaderData(unsigned char *pData, int nVersion, int nVBLRecorded)
{
	if (nVersion == YMOUTPUT_VERSION_3)
	{
		memcpy(pData, YM3_Ident_Text, 4);
	}
	else
	{
		YM5_HEADER YM5_Header;

		memcpy(&YM5_Header.ID, YM5_Ident_Text, 4);
		memcpy(&YM5_Header.IDString, YM5_IdentString_Text, 8);
		YM5_Header.NumVBL = nVBLRecorded;
		YM5_Header.SongAttributes = YM5_SONG_ATTRIBUTE_DATA_INTERLEAVED;
		YM5_Header.NoOfDigiDrumSamples = 0;
		YM5_Header.YMFrequency = 1000000;
		YM5_Header.PlayerFrequency = 50;
		YM5_Header.VBLLoopIndex = 0;
		YM5_Header.SizeOfExtraData = 0;

#ifdef CPC_LSB_FIRST
		YM5_Header.NumVBL = SwapEndianLong(YM5_Header.NumVBL);
		YM5_Header.SongAttributes = SwapEndianLong(YM5_Header.SongAttributes);
		YM5_Header.YMFrequency = SwapEndianLong(YM5_Header.YMFrequency);
		YM5_Header.VBLLoopIndex = SwapEndianLong(YM5_Header.VBLLoopIndex);
		YM5_Header.NoOfDigiDrumSamples = SwapEndianWord(YM5_Header.NoOfDigiDrumSamples);
		YM5_Header.PlayerFrequency = SwapEndianWord(YM5_Header.PlayerFrequency);
		YM5_Header.SizeOfExtraData = SwapEndianWord(YM5_Header.SizeOfExtraData);
#endif
		memcpy(pData, &YM5_Header,sizeof(YM5_HEADER));
		pData+=sizeof(YM5_HEADER);

		/* write name of song */
		if (YM5_SongName!=NULL)
		{
			int nStringLen = strlen((char *) YM5_SongName);
			memcpy(pData, YM5_SongName, nStringLen);
			pData+=nStringLen;
		}

		pData[0] = '\0';
		pData++;

		/* write author name */
		if (YM5_AuthorName!=NULL)
		{
			int nStringLen = strlen((char *) YM5_AuthorName);
			memcpy(pData, YM5_AuthorName, nStringLen);
			pData+=nStringLen;
		}

		pData[0] = '\0';
		pData++;

		if (YM5_Comments!=NULL)
		{
			int nStringLen = strlen((char *) YM5_Comments);
			memcpy(pData, YM5_Comments, nStringLen);
			pData+=nStringLen;
		}

		pData[0] = '\0';
	}
}

void YMOutput_GenerateTrailerData(unsigned char *pData, int nVersion)
{

	if (nVersion == YMOUTPUT_VERSION_5)
	{
		/* version 5 */
		memcpy(pData, (char *) YM5_EndFileText, 4);
	}
}

void YMOutput_ConvertTempData(const unsigned char *pSrcData, unsigned char *pOutputData, int nVersion, int nVBLRecorded)
{
	int j,i;
	unsigned long nPos = 0;

	for (j=0; j<14; j++)
	{
		for (i=0; i<nVBLRecorded; i++)
		{
			unsigned char Data = pSrcData[(i*16) + j];
            if (j==7)
            {
                Data &= 0x03f;
            }
			pOutputData[nPos] = Data;
			nPos++;
		}
	}

	if (nVersion == YMOUTPUT_VERSION_5)
	{
		/* write register 14, 15 details - in this case just zero's. */
		memset(&pOutputData[nPos], 0, (nVBLRecorded *2));
	}
}

void YMOutput_Update(void)
{
    if (YMOutput_Enabled)
    {

        /* not recording? */
        if (!YMOutput_Recording)
        {
            /* record when silence ends? */
            if (YMOutput_RecordWhenSilenceEnds)
            {
                /* is there anything? */
                if (!YMOutput_IsSilent())
                {
                    /* start recording */
                    YMOutput_Recording = TRUE;
                }
            }
            else
            {
                YMOutput_Recording = TRUE;
            }
        }

        if (YMOutput_Recording)
        {
            unsigned char RegsToWrite[16];

            YMOutput_GenerateTempRecord(RegsToWrite);

            /* if we are about to go over end of buffer then stop recording */
            if ((pRawBufferPtr+16)>(pRawBuffer+RawBufferSize))
            {
                YMOutput_Enabled = FALSE;
            }
            else
            {
                /* continue to write into buffer */
                memcpy(pRawBufferPtr, RegsToWrite, 16);
                nVBL++;
                pRawBufferPtr+=16;
            }

            /* stop recording  when silence begins again? */
            if (YMOutput_StopRecordWhenSilenceBegins)
            {
                /* is there anything? */
                if (YMOutput_IsSilent())
                {
                    /* no, so stop */
                    YMOutput_Enabled = FALSE;
                }
            }



        }
        if (pPSGToDump)
        {
          PSG_ResetFlags(pPSGToDump);
      }
    }
}

static unsigned long YM_GetYMSize(int nVersion)
{
    unsigned long nYMSize;

    /* header */
    nYMSize = YMOutput_GenerateHeaderOutputSize(nVersion);

    if (nVersion==YMOUTPUT_VERSION_5)
    {
        /* records */
        nYMSize += nVBL*16;
    }
    else
    {
        nYMSize += nVBL*14;
    }

    /* trailer */
    nYMSize += YMOutput_GenerateTrailerOutputSize(nVersion);

    return nYMSize;
}

unsigned long YM_GetOutputSize(int nVersion)
{
    unsigned long nSize;

    /* header */
    nSize = /* LHA header */
            /* checksum and header length */
            2+
            19+
            /* byte to indicate length of filename */
            1+
            /* filename length itself */
            YMFilenameInLHALength +
            /* file checksum */
            2 +
            /* lha second header indicating end of file */
            1;
    nSize += YM_GetYMSize(nVersion);

    return nSize;
}


void YM_GenerateOutputData(unsigned char *pBuffer, int nVersion)
{
    int i;
    unsigned char *pBufferPtr = pBuffer;
    int nSize;
    unsigned char chChecksum = 0;
    unsigned short CRC = 0;
    unsigned char *pCRCPtr;
    unsigned char *pFilePtr;
    int crcTable[256];
	int YMSize;
	unsigned long HeaderSize;

    for( i = 0 ; i < 256; i++ )
    {
        int j;
        crcTable[i] = i;

        for( j = 0 ; j < 8; j++ )
        {
            if( ( crcTable[i] & 1 ) != 0 )
            {
                crcTable[i] = ( crcTable[i] >> 1 ) ^ 0x0a001;
            }
            else
            {
                crcTable[i] >>= 1;
            }
        }
    }



    nVersion = YMOutput_ValidateVersion(nVersion);

    YMSize =YM_GetYMSize(nVersion);
    HeaderSize = 19+1+2+YMFilenameInLHALength;


    /* write LHA header */
    *pBufferPtr = HeaderSize&0x0ff;
    ++pBufferPtr;
    /* checksum; filled in later */
    *pBufferPtr = 0;
    ++pBufferPtr;

    /* compression method */
    memcpy(pBufferPtr, LHA_CompressMethod, 5);
    pBufferPtr+=5;

    /* compressed size */
    *pBufferPtr =(YMSize & 0x0ff);
    ++pBufferPtr;
    *pBufferPtr =((YMSize>>8) & 0x0ff);
    ++pBufferPtr;
    *pBufferPtr =((YMSize>>16) & 0x0ff);
    ++pBufferPtr;
    *pBufferPtr =((YMSize>>24) & 0x0ff);
    ++pBufferPtr;

    /* uncompressed size */
    *pBufferPtr =(YMSize & 0x0ff);
    ++pBufferPtr;
    *pBufferPtr =((YMSize>>8) & 0x0ff);
    ++pBufferPtr;
    *pBufferPtr =((YMSize>>16) & 0x0ff);
    ++pBufferPtr;
    *pBufferPtr =((YMSize>>24) & 0x0ff);
    ++pBufferPtr;

    /* date/time */
    *pBufferPtr = 0;
    ++pBufferPtr;
    *pBufferPtr = 0;
    ++pBufferPtr;
    *pBufferPtr = 0;
    ++pBufferPtr;
    *pBufferPtr = 0;
    ++pBufferPtr;


    /* MSDOS file attribute */
    *pBufferPtr = 0x020;
    ++pBufferPtr;
    /* non extended header */
    *pBufferPtr = 0x0;
    ++pBufferPtr;

    /* length of filename */
    *pBufferPtr = (YMFilenameInLHALength & 0x0ff);
    ++pBufferPtr;

    /* filename itself */
    memcpy(pBufferPtr, YMFilenameInLHA, (YMFilenameInLHALength & 0x0ff));
    pBufferPtr+=YMFilenameInLHALength;

    pCRCPtr = pBufferPtr;

    /* crc of file */
    *pBufferPtr = 0x0;
    ++pBufferPtr;
    *pBufferPtr = 0x0;
    ++pBufferPtr;

    /* now here follows the file */


    pFilePtr = pBufferPtr;

    /* header */
    nSize = YMOutput_GenerateHeaderOutputSize(nVersion);
    YMOutput_GenerateHeaderData(pBufferPtr, nVersion, nVBL);
    pBufferPtr+=nSize;

    YMOutput_ConvertTempData(pRawBuffer, pBufferPtr, nVersion, nVBL);
    if (nVersion==YMOUTPUT_VERSION_5)
    {
        pBufferPtr+=(nVBL*16);
    }
    else
    {
        pBufferPtr+=(nVBL*14);
    }

    /* trailer */
    nSize = YMOutput_GenerateTrailerOutputSize(nVersion);
    YMOutput_GenerateTrailerData(pBufferPtr, nVersion);
    pBufferPtr+=nSize;

    CRC = 0;
    for (i=0; i<YMSize; i++)
    {
       CRC = (CRC>>8) ^ crcTable[(CRC^pFilePtr[i])&0x0ff];
    }
    *pCRCPtr = CRC & 0x0ff;
    ++pCRCPtr;
    *pCRCPtr = (CRC>>8) & 0x0ff;
    ++pCRCPtr;


    /* calc checksum */
    for (i=0; i<HeaderSize; i++)
    {
        chChecksum += pBuffer[i+2];
    }
    /* write checksum into header */
    pBuffer[1] = chChecksum;

    /* put lha end of file marker */
    *pBufferPtr = 0;
   ++pBufferPtr;

}

void YMOutput_StopRecording(void)
{
    /* this is a forced stop */
    if (YMOutput_Enabled)
    {
        YMOutput_Enabled = FALSE;
    }
}



