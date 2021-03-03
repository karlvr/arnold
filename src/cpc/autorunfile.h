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
#ifndef __AUTORUN_FILE_HEADER_INCLUDED__
#define __AUTORUN_FILE_HEADER_INCLUDED__

/* autorun a file loaded from host filesystem, e.g. Amstrad binary or Amstrad basic file */

/* init the auto type functions */
void AutoRunFile_Init(void);

BOOL AutoRunFile_Active(void);

void AutoRunFile_Finish(void);

/* set the string to auto type */
void AutoRunFile_SetData(const char *pData, unsigned long nDataLength, BOOL bWaitInput, BOOL bResetCPC, FILE_HEADER *pFileHeader);

/* execute this every emulated frame; even if it will be skipped */
void AutoRunFile_Update(void);

#endif
