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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../cpcglob.h"

static int m_BreakOnAction = 0;
static int m_BrokeOn = 0;

#include "breakpt.h"

BOOL BreakpointDescription_Compare(BREAKPOINT_DESCRIPTION *pDescriptionA, BREAKPOINT_DESCRIPTION *pDescriptionB)
{
	if (pDescriptionA->Type != pDescriptionB->Type)
		return FALSE;
	
	if (pDescriptionA->Address != pDescriptionB->Address)
		return FALSE;

	if (pDescriptionA->AddressMask != pDescriptionB->AddressMask)
		return FALSE;

	if (pDescriptionA->Data != pDescriptionB->Data)
		return FALSE;

	if (pDescriptionA->DataMask != pDescriptionB->DataMask)
		return FALSE;

	if (pDescriptionA->nReloadHitCount != pDescriptionB->nReloadHitCount)
		return FALSE;

	if (pDescriptionA->bTemporary != pDescriptionB->bTemporary)
		return FALSE;

	return TRUE;
}

void BreakpointDescription_Init(BREAKPOINT_DESCRIPTION *pDescription)
{
	memset(pDescription, 0, sizeof(BREAKPOINT_DESCRIPTION));
	pDescription->bTemporary = FALSE;
	pDescription->nReloadHitCount = 1;
	pDescription->Data = 0x0;
	pDescription->DataMask = 0x0ff;
	pDescription->Address = 0;
	pDescription->AddressMask = 0x0ffff;
}

static BREAKPOINT *pFirstBreakpoint = NULL;

/* there is one temporary breakpoint which is set for "run to cursor" */
static BREAKPOINT *pTemporaryBreakpoint = NULL;

static BOOL BreakpointsRequireFullRefresh = FALSE;

BOOL Breakpoints_FullRefreshInGUI()
{
    return BreakpointsRequireFullRefresh;
}

void Breakpoints_SetFullRefreshInGUI(BOOL bState)
{
    BreakpointsRequireFullRefresh = bState;
}

BOOL AnyBreakOn()
{
	/* broke on something and we want to break on that */
	return ((m_BrokeOn&m_BreakOnAction)!=0);
}
void ResetBreakOn()
{
	m_BrokeOn = 0;
}

void TriggerBreakOn(int nMask)
{
	m_BrokeOn |= nMask;
}

BOOL BreakOnAction(int nMask)
{
	return ((m_BreakOnAction & nMask)!=0);

}

void SetBreakOnAction(int nMask, BOOL bState)
{
	m_BreakOnAction &= ~nMask;
	if (bState)
	{
		m_BreakOnAction |= nMask;
	}
}


BREAKPOINT *Breakpoints_GetFirst()
{
	return pFirstBreakpoint;
}

BREAKPOINT *Breakpoints_GetNext(BREAKPOINT *pBreakpoint)
{
	return pBreakpoint->pNext;
}

BOOL Breakpoint_RequireRefreshInGUI(BREAKPOINT *pBreakpoint)
{
    return pBreakpoint->RequireRefreshInGUI;
}

void Breakpoint_SetRequireRefreshInGUI(BREAKPOINT *pBreakpoint, BOOL bState)
{
    pBreakpoint->RequireRefreshInGUI = bState;
}

BOOL Breakpoints_Valid(BREAKPOINT *pBreakpointWanted)
{
	BREAKPOINT *pBreakpoint;

	pBreakpoint = pFirstBreakpoint;

	while (pBreakpoint != NULL)
	{
		if (pBreakpoint == pBreakpointWanted)
			return TRUE;

		pBreakpoint = pBreakpoint->pNext;
	}
	return FALSE;
}


BREAKPOINT *Breakpoints_FindBreakpoint(BREAKPOINT_DESCRIPTION *pDescription)
{
	BREAKPOINT *pBreakpoint;

	pBreakpoint = pFirstBreakpoint;

	while (pBreakpoint!=NULL)
	{
		if (BreakpointDescription_Compare(&pBreakpoint->Description, pDescription))
		{
			return pBreakpoint;
		}

		pBreakpoint = pBreakpoint->pNext;
	}
	return NULL;
}


/* add a breakpoint */
BREAKPOINT *Breakpoints_AddBreakpointI(BREAKPOINT_DESCRIPTION *pDescription)
{
	BREAKPOINT *pBreakpoint;

	/* no, add it. */
	pBreakpoint = (BREAKPOINT *)malloc(sizeof(BREAKPOINT));

	if (pBreakpoint!=NULL)
	{
		memset(pBreakpoint, 0, sizeof(BREAKPOINT));

		memcpy(&pBreakpoint->Description, pDescription, sizeof(BREAKPOINT_DESCRIPTION));

		pBreakpoint->bEnabled = TRUE;
		pBreakpoint->nCurrentHitCount = pBreakpoint->Description.nReloadHitCount;
		pBreakpoint->pNext = pFirstBreakpoint;
		pBreakpoint->RequireRefreshInGUI = TRUE;
		Breakpoints_SetFullRefreshInGUI(TRUE);
		if (!pDescription->bTemporary)
		{
			pFirstBreakpoint = pBreakpoint;
		}
	}
	return pBreakpoint;
}

/* we can set a temporary breakpoint at the same place as a normal breakpoint, a temporary one will always hit */
BREAKPOINT *Breakpoints_AddTemporaryBreakpoint(BREAKPOINT_DESCRIPTION *pDescription)
{
	pDescription->bTemporary = TRUE;
	
	/* remove existing temporary breakpoint */
	Breakpoints_RemoveTemporaryBreakpoint();

	/* add a new one */
	
	pTemporaryBreakpoint = Breakpoints_AddBreakpointI(pDescription);
  return pTemporaryBreakpoint;
  }

/* add a breakpoint */
BREAKPOINT *Breakpoints_AddBreakpoint(BREAKPOINT_DESCRIPTION *pDescription)
{
	pDescription->bTemporary = FALSE;

	/* already set as a breakpoint? */
	BREAKPOINT *pBreakpoint = Breakpoints_FindBreakpoint(pDescription);
	if (pBreakpoint!=NULL)
	{
		return pBreakpoint;
	}
	
	return Breakpoints_AddBreakpointI(pDescription);
}

/* remove an existing breakpoint */
void    Breakpoints_RemoveBreakpoint(BREAKPOINT *pBreakpoint)
{
	if (pBreakpoint==pFirstBreakpoint)
	{
		pFirstBreakpoint = pBreakpoint->pNext;
	}
	else
	{
		BREAKPOINT *pCurrent = pFirstBreakpoint;
		while (pCurrent!=NULL)
		{
			if (pCurrent->pNext==pBreakpoint)
			{
				pCurrent->pNext = pBreakpoint->pNext;
				break;
			}

			pCurrent = pCurrent->pNext;
		}

	}
	free(pBreakpoint);
	Breakpoints_SetFullRefreshInGUI(TRUE);
}

/* remove a breakpoint by description */
void	Breakpoints_RemoveBreakpointByDescription(BREAKPOINT_DESCRIPTION *pDescription)
{
	/* find matching breakpoint */
	BREAKPOINT *pBreakpoint = Breakpoints_FindBreakpoint(pDescription);
	if (pBreakpoint)
	{
		/* found it, now remove it */
		Breakpoints_RemoveBreakpoint(pBreakpoint);
	}
}

void	Breakpoints_RemoveTemporaryBreakpoint()
{
	if (pTemporaryBreakpoint!=NULL)
	{
		free(pTemporaryBreakpoint);
		Breakpoints_SetFullRefreshInGUI(TRUE);
		pTemporaryBreakpoint = NULL;
	}
}

/* free all breakpoints */
void	Breakpoints_Free(void)
{
	BREAKPOINT *pBreakpoint;

	pBreakpoint = pFirstBreakpoint;

	while (pBreakpoint!=NULL)
	{
		BREAKPOINT *pNext;

		pNext = pBreakpoint->pNext;

		free(pBreakpoint);

		pBreakpoint = pNext;
	}

	pFirstBreakpoint = NULL;
	Breakpoints_SetFullRefreshInGUI(TRUE);
}

void Breakpoints_SetEnabled(BREAKPOINT *pBreakpoint, BOOL bEnable)
{
	pBreakpoint->bEnabled = bEnable;
	pBreakpoint->RequireRefreshInGUI = TRUE;
}

void BreakpointDescription_SetNoAddressMask(BREAKPOINT_DESCRIPTION *pDescription)
{
	/* this mask means the exact address must be specified */
	pDescription->AddressMask = 0x0ffff;
}

void BreakpointDescription_SetAddressMask(BREAKPOINT_DESCRIPTION *pDescription, int nMask)
{
	/* this mask means it can match multiple addresses */
	pDescription->AddressMask = nMask;
}

void BreakpointDescription_SetIgnoreData(BREAKPOINT_DESCRIPTION *pDescription)
{
	/* setup data and mask so any data is allowed */

	/* equals this */
	pDescription->Data = 0x0;
	/* mask with this */
	pDescription->DataMask = 0x0;
}

void BreakpointDescription_SetDataWithMask(BREAKPOINT_DESCRIPTION *pDescription, int nData, int nMask)
{
	/* data and mask set so various data is allowed that matches */
	pDescription->Data = nData;
	pDescription->DataMask = nMask;
}

void BreakpointDescription_SetDataNoMask(BREAKPOINT_DESCRIPTION *pDescription, int nData)
{
	/* data and mask set so a specific value is allowed */
	pDescription->Data = nData;
	pDescription->DataMask = 0x0ff;
}


void BreakpointDescription_SetHitCount(BREAKPOINT_DESCRIPTION *pDescription, int nCount)
{
	if (nCount<1)
	{
		nCount = 1;
	}
	pDescription->nReloadHitCount = nCount;
}

BOOL Breakpoints_CanBreakpointBeTriggered(BREAKPOINT *pBreakpoint, BREAKPOINT_TYPE Type, int Address)
{
	if (
		/* the type we want */
		(pBreakpoint->Description.Type == Type) &&
		/* enabled */
		(pBreakpoint->bEnabled)
		)
	{
		/* did we reach this address? */
		if ((Address & pBreakpoint->Description.AddressMask) == pBreakpoint->Description.Address)
		{
			/* update hit count */
			pBreakpoint->nCurrentHitCount--;

			/* count over? */
			if (pBreakpoint->nCurrentHitCount == 0)
			{
				/* hit count hit, reload it and indicate we hit this breakpoint */
				pBreakpoint->nCurrentHitCount = pBreakpoint->Description.nReloadHitCount;

				pBreakpoint->RequireRefreshInGUI = TRUE;

				/* TODO: If it's temporary we should remove it */

				/* hit the breakpoint */
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* true if the address matches any of the breakpoints and their hit count reaches zero */
BOOL Breakpoints_IsAHitableBreakpoint(BREAKPOINT_TYPE Type, int Address)
{
	if (pTemporaryBreakpoint)
	{
		// TODO: Do we need multiple temporary breakpoints?
		if (Breakpoints_CanBreakpointBeTriggered(pTemporaryBreakpoint, Type, Address))
			return TRUE;
	}

	{
		BREAKPOINT *pBreakpoint;

		pBreakpoint = pFirstBreakpoint;

		while (pBreakpoint != NULL)
		{
			if (Breakpoints_CanBreakpointBeTriggered(pBreakpoint, Type, Address))
				return TRUE;

			pBreakpoint = pBreakpoint->pNext;
		}

		return FALSE;
	}
}


/* true if the address matches any of the breakpoints and their hit count reaches zero */
BOOL Breakpoints_IsAHitableBreakpointData(BREAKPOINT_TYPE Type, int Address, int nData)
{
	BREAKPOINT *pBreakpoint;

	pBreakpoint = pFirstBreakpoint;

	while (pBreakpoint!=NULL)
	{
		if (
            /* the type we want */
			(pBreakpoint->Description.Type == Type) &&
			/* enabled */
			(pBreakpoint->bEnabled)
		)
		{
      /* address? */
			if ((Address & pBreakpoint->Description.AddressMask) == pBreakpoint->Description.Address)
			{
          /* data matches? */
				if ((nData & pBreakpoint->Description.DataMask) == pBreakpoint->Description.Data)
			   {

			   pBreakpoint->RequireRefreshInGUI = TRUE;

				/* update hit count */
				pBreakpoint->nCurrentHitCount--;
				if (pBreakpoint->nCurrentHitCount==0)
				{
					/* hit count hit, reload it and indicate we hit this breakpoint */
					pBreakpoint->nCurrentHitCount = pBreakpoint->Description.nReloadHitCount;

					return TRUE;
				}
			   }			}
		}

		pBreakpoint = pBreakpoint->pNext;
	}

	return FALSE;
}

BREAKPOINT *Breakpoints_IsAVisibleBreakpoint(BREAKPOINT_DESCRIPTION *pDescription)
{
	BREAKPOINT *pBreakpoint = Breakpoints_FindBreakpoint(pDescription);
	if (pBreakpoint!=NULL)
	{
		if (!pBreakpoint->Description.bTemporary)
			return pBreakpoint;
	}
	return NULL;
}
