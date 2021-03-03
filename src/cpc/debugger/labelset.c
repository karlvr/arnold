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
#include "labelset.h"

/* list of labelsets */
static LABELSET *m_Labelsets = NULL;

LABELSET *labelset_get_first()
{
	return m_Labelsets;
}


void labelsets_init()
{
    m_Labelsets = NULL;
}


static void label_free(LABEL *label)
{
	if (label==NULL)
		return;
	
	/* free name */
	if (label->m_sName!=NULL)
	{
		free(label->m_sName);
		label->m_sName = NULL;
	}
	free(label);
}

static void labelset_free(LABELSET *labelset)
{
	LABEL *pCurrent;
	
	if (labelset==NULL)
		return;
	
	pCurrent = labelset->m_pLabels;
	while (pCurrent!=NULL)
	{
		LABEL *pNext = pCurrent->m_pNext;
		label_free(pCurrent);
		pCurrent = pNext;
	}	
	if (labelset->m_sName!=NULL)
	{
		free(labelset->m_sName);
		labelset->m_sName = NULL;
	}
	free(labelset);
}

void labelsets_free()
{
	LABELSET *pCurrent = m_Labelsets;
	while (pCurrent!=NULL)
	{
		LABELSET *pNext = pCurrent->m_pNext;
		labelset_free(pCurrent);
		pCurrent = pNext;
	}
	m_Labelsets = NULL;
}
void label_set_name(LABEL *pLabel, const char *sName)
{
	int LabelLength;
	if (pLabel->m_sName!=NULL)
	{
		free(pLabel->m_sName);
		pLabel->m_sName = NULL;
	}
	
		LabelLength= strlen(sName);
		
	    pLabel->m_sName = (char *)malloc(LabelLength+1);
		if (pLabel->m_sName!=NULL)
		{
			strcpy(pLabel->m_sName, sName);
		}
	
}

/* add a label to the label set */
LABEL *labelset_add_label(LABELSET *pLabelSet, const char *sName, Z80_WORD Address)
{
	LABEL *pLabel;
	
	if (pLabelSet==NULL)
		return NULL;
	if (sName==NULL)
		return NULL;
	
    pLabel = (LABEL *)malloc(sizeof(LABEL));
	if (pLabel!=NULL)
	{
		memset(pLabel, 0, sizeof(LABEL));
		
		label_set_name(pLabel, sName);
		
	    pLabel->m_Address = Address;
	    pLabel->m_bActive = TRUE;

		pLabel->m_pNext = pLabelSet->m_pLabels;
		pLabelSet->m_pLabels = pLabel;
	}
	return pLabel;
}

void label_enable(LABEL *pLabel, BOOL bEnable)
{
	if (pLabel==NULL)
		return;
	
	pLabel->m_bActive = bEnable;
}

LABELSET *labelset_find_by_name(const char *sName)
{
	LABELSET *pCurrent = m_Labelsets;
	while (pCurrent!=NULL)
	{
#ifdef WIN32
		if (stricmp(pCurrent->m_sName, sName)==0)
#else
		if (strcasecmp(pCurrent->m_sName, sName)==0)
#endif		
		{
			return pCurrent;
		}
		pCurrent = pCurrent->m_pNext;
	}
	return NULL;
}


void labelset_delete_label(LABELSET *pLabelSet, LABEL *pLabel)
{
	if (pLabelSet->m_pLabels==pLabel)
	{
		pLabelSet->m_pLabels = pLabel->m_pNext;
	}
	else
	{
		LABEL *pCurrent = pLabelSet->m_pLabels;
		while (pCurrent!=NULL)
		{
			if (pCurrent->m_pNext==pLabel)
			{
				pCurrent->m_pNext = pLabel->m_pNext;
				break;
			}
			pCurrent = pCurrent->m_pNext;
		}
	}

	label_free(pLabel);
}

void labelset_delete(LABELSET *pLabelSet)
{
	if (pLabelSet==m_Labelsets)
	{
		m_Labelsets = pLabelSet->m_pNext;
	}
	else
	{
		LABELSET *pCurrent = m_Labelsets;
		while (pCurrent!=NULL)
		{
			if (pCurrent->m_pNext==pLabelSet)
			{
				pCurrent->m_pNext = pLabelSet->m_pNext;
				break;
			}
			pCurrent = pCurrent->m_pNext;
		}

	}

	labelset_free(pLabelSet);
}


void labelset_enable(LABELSET *pLableSet, BOOL bEnable)
{
	if (pLableSet==NULL)
		return;
	
	pLableSet->m_bActive = bEnable;
}

void labelset_set_name(LABELSET *pLabelSet, const char *sName)
{
	int NameLength;
	
	if (pLabelSet->m_sName!=NULL)
	{
		free(pLabelSet->m_sName);
		pLabelSet->m_sName = NULL;
	}
	
		NameLength = strlen(sName);
		
	    pLabelSet->m_sName = (char *)malloc(NameLength+1);
	if (pLabelSet->m_sName!=NULL)
	{		
		strcpy(pLabelSet->m_sName, sName);
	}		
}

LABELSET *labelset_create(const char *sName)
{
	LABELSET *pLabelSet;
	
	if (sName==NULL)
		return NULL;
	
    pLabelSet = (LABELSET *)malloc(sizeof(LABELSET));
	if (pLabelSet!=NULL)
	{
		memset(pLabelSet, 0, sizeof(LABELSET));
		
		labelset_set_name(pLabelSet, sName);
	    pLabelSet->m_bActive = TRUE;
	    pLabelSet->m_pLabels = NULL;
		pLabelSet->m_pNext = m_Labelsets;
		m_Labelsets = pLabelSet;
	}

    return pLabelSet;
}

LABEL *labelsets_find_label_by_exact_address(Z80_WORD Address)
{
	LABELSET *pCurrent = m_Labelsets;
	while (pCurrent!=NULL)
	{
		if (pCurrent->m_bActive)
		{
			LABEL *pCurrentLabel = pCurrent->m_pLabels;
			while (pCurrentLabel!=NULL)
			{
				if (pCurrentLabel->m_bActive)
				{
					if (pCurrentLabel->m_Address==Address)
						return pCurrentLabel;
				}				
				pCurrentLabel = pCurrentLabel->m_pNext;
			}
		}
			
		
		pCurrent = pCurrent->m_pNext;
	}

	return NULL;
}


LABEL *labelsets_find_label_by_name(const char *sName)
{
	LABELSET *pCurrent = m_Labelsets;
	while (pCurrent!=NULL)
	{
		if (pCurrent->m_bActive)
		{
			LABEL *pCurrentLabel = pCurrent->m_pLabels;
			while (pCurrentLabel!=NULL)
			{
				if (pCurrentLabel->m_bActive)
				{
#ifdef WIN32
		if (stricmp(pCurrentLabel->m_sName, sName)==0)
#else
		if (strcasecmp(pCurrentLabel->m_sName, sName)==0)
#endif	
		{
			return pCurrentLabel;
		}
				}				
				pCurrentLabel = pCurrentLabel->m_pNext;
			}
		}
			
		
		pCurrent = pCurrent->m_pNext;
	}

	return NULL;
}

