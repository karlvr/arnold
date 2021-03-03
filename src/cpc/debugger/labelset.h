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
#ifndef __LABELSET_HEADER_INCLUDED__
#define __LABELSET_HEADER_INCLUDED__

#include "../cpc.h"

typedef struct _LABEL
{
  /* name of label */
  char *m_sName;
  BOOL m_bActive;
  /* address of label */
  Z80_WORD m_Address;
	struct _LABEL *m_pNext;
} LABEL;

typedef struct _LABELSET
{
    /* name of label set */
    char *m_sName;
    /* is this label set active? */
    BOOL m_bActive;
    /* list of labels */
    LABEL *m_pLabels;
	
	struct _LABELSET *m_pNext;
} LABELSET;

/* TODO: Remove labelset and remove label */

void labelsets_init(void);
void labelsets_free(void);

void labelset_delete(LABELSET *labelset);
void labelset_delete_label(LABELSET *pLabelSet, LABEL *pLabel);

void labelset_set_name(LABELSET *labelset, const char *sName);

LABELSET *labelset_create(const char *sName);
void labelset_enable(LABELSET *labelset, BOOL bState);
LABEL *labelset_add_label(LABELSET *, const char *sName, Z80_WORD Address);
LABELSET *labelset_find_by_name(const char *sName);

void label_set_name(LABEL *label, const char *sName);
void label_enable(LABEL *label, BOOL bState);
LABELSET *labelset_get_first(void);

LABEL *labelsets_find_label_by_name(const char *sName);
LABEL *labelsets_find_label_by_exact_address(Z80_WORD Address);

#endif
