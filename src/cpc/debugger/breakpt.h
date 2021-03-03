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
#ifndef __BREAKPOINT_HEADER_INCLUDED__
#define __BREAKPOINT_HEADER_INCLUDED__

#include "../cpcglob.h"

/* VSYNC */
#define BREAK_ON_VSYNC_START (1<<0)
#define BREAK_ON_VSYNC_END (1<<1)

/* HSYNC */
#define BREAK_ON_HSYNC_START (1<<2)
#define BREAK_ON_HSYNC_END (1<<3)

/* INTERRUPT */
#define BREAK_ON_INT (1<<4)
#define BREAK_ON_NMI (1<<5)

/* ASIC */
#define BREAK_ON_ASIC_UNLOCK (1<<6)
#define BREAK_ON_ASIC_LOCK (1<<7)

/* define a template/preset for an i/o breakpoint */
typedef struct 
{
	BOOL bRead;
	const char *sName;		/* name of the template */
	int Address;			/*address */
	BOOL bUseAddressMask;	/* yes to use the address mask */
	int AddressMask;		/*the mask */
	BOOL bUseData;			/*yes to use the data */
	int Data;				/* the data */
	BOOL bUseDataMask;		/* yes to use the data mask */
	int DataMask;			/*the data mask */
} BREAKPOINT_IO_TEMPLATE;

typedef enum  
{
	BREAKPOINT_MAJOR_TYPE_REGISTER, /* any register operation */
	BREAKPOINT_MAJOR_TYPE_MEMORY, /* any memory operation */
	BREAKPOINT_MAJOR_TYPE_IO, /* any i/o operation */
	BREAKPOINT_MAJOR_TYPE_MAX /* keep last */
} BREAKPOINT_MAJOR_TYPE;

typedef enum
{
    BREAKPOINT_TYPE_PC = 0, /* address breakpoint matched against PC */
	BREAKPOINT_TYPE_SP, /* address breakpoint matched against SP */
    BREAKPOINT_TYPE_MEMORY_WRITE,  /* data write breakpoint */
    BREAKPOINT_TYPE_MEMORY_READ, /* data read breakpoint */
	BREAKPOINT_TYPE_IO_WRITE,  /* data write breakpoint */
	BREAKPOINT_TYPE_IO_READ /* data read breakpoint */
} BREAKPOINT_TYPE;

typedef struct
{
	BREAKPOINT_TYPE Type;

	/* breakpoint address */
	int Address;
	/* the mask to apply to the address before comparison; more suited to I/O breakpoints */
	int AddressMask;
	/* data value to match if a data breakpoint */
	int Data;
	/* mask to apply before comparing to Data */
	int DataMask;
	/* the initial and reload hit count */
	int nReloadHitCount;

	/* true if this is a temporary breakpoint that should be removed once hit */
	BOOL bTemporary;
} BREAKPOINT_DESCRIPTION;

BOOL Breakpoint_CompareDescription(BREAKPOINT_DESCRIPTION *pDescriptionA, BREAKPOINT_DESCRIPTION *pDescriptionB);

typedef struct _BREAKPOINT
{
	BREAKPOINT_DESCRIPTION Description;
	/* the current hit count */
	int nCurrentHitCount;
	/* true if enabled */
	BOOL bEnabled;
	/* requires refresh or not */
	BOOL RequireRefreshInGUI;

	/* next breakpoint */
	struct _BREAKPOINT *pNext;
} BREAKPOINT;

BOOL BreakOnAction(int nMask);
void SetBreakOnAction(int nMask, BOOL bState);
void ResetBreakOn(void);
void TriggerBreakOn(int nMask);
BOOL AnyBreakOn(void);

BOOL Breakpoints_Valid(BREAKPOINT *pBreakpointWanted);
BOOL Breakpoints_FullRefreshInGUI(void);
void Breakpoints_SetFullRefreshInGUI(BOOL bState);
void Breakpoints_SetEnabled(BREAKPOINT *pBreakpoint, BOOL bEnable);

void BreakpointDescription_Init(BREAKPOINT_DESCRIPTION *pDescription);
void BreakpointDescription_SetHitCount(BREAKPOINT_DESCRIPTION *pBreakpoint, int nCount);
BOOL BreakpointDescription_Compare(BREAKPOINT_DESCRIPTION *pDescriptionA, BREAKPOINT_DESCRIPTION *pDescriptionB);
void BreakpointDescription_SetIgnoreData(BREAKPOINT_DESCRIPTION *pBreakpoint);
void BreakpointDescription_SetDataWithMask(BREAKPOINT_DESCRIPTION *pBreakpoint, int nData, int nMask);
void BreakpointDescription_SetDataNoMask(BREAKPOINT_DESCRIPTION *pBreakpoint, int nData);
void BreakpointDescription_SetNoAddressMask(BREAKPOINT_DESCRIPTION *pBreakpoint);
void BreakpointDescription_SetAddressMask(BREAKPOINT_DESCRIPTION *pBreakpoint, int nMask);

BOOL Breakpoints_IsAHitableBreakpointData(BREAKPOINT_TYPE Type, int Address, int nData);

BREAKPOINT *Breakpoints_AddTemporaryBreakpoint(BREAKPOINT_DESCRIPTION *pDescription);
void 	Breakpoints_RemoveTemporaryBreakpoint(void);
BREAKPOINT *Breakpoints_AddBreakpoint(BREAKPOINT_DESCRIPTION *pDescription);

void    Breakpoints_RemoveBreakpoint(BREAKPOINT *pBreakpoint);
BREAKPOINT *Breakpoints_GetFirst(void);
BREAKPOINT *Breakpoints_GetNext(BREAKPOINT *pBreakpoint);

BOOL Breakpoints_IsAHitableBreakpoint(BREAKPOINT_TYPE Type, int Address);
BREAKPOINT *Breakpoints_IsAVisibleBreakpoint(BREAKPOINT_DESCRIPTION *pDescription);


void Breakpoints_RemoveBreakpointByDescription(BREAKPOINT_DESCRIPTION *pDescription);


BOOL Breakpoint_RequireRefreshInGUI(BREAKPOINT *pBreakpoint);
void Breakpoint_SetRequireRefreshInGUI(BREAKPOINT *pBreakpoint,BOOL bState);
void Breakpoints_Free(void);


#endif

