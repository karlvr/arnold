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
/* SPO-256 speech chip emulation */
/* on CPC input/output port is 0x0fbfe */
/* appears that when writing data lines hold address */
/* when reading bit 7 contains LRQ */

static int SPO_LRQ = 0;		/* 0 when value can be loaded, 1 when value cannot be loaded */
static int SPO_SBY = 1;		/* 1 indicates SPO is inactive, 0 indictates it is active */

#include "spo256.h"

unsigned char buffer[256];



void	SPO256_Initialise(void)
{
/*	FILE *fh;

	fh = fopen("currah.dat","r");

	fgets(buffer, sizeof(buffer), f);

	fgetc(buffer, sizeof(char), fh);


	if (fscanf(fin,"%D %D %D %D %D %d\n",
						&(allos[i].offset),
						&(allos[i].relrestart),
						&(allos[i].length),
						&(allos[i].pause),
						&(allos[i].playlength),
						&(allos[i].simple)
					  )!=6) {
*/


}

void	SPO256_SetAddress(unsigned char Addr)
{
	/* SPO address is 9-bits. Lower bit is always
	zero because word's are acccessed. Each word
	contains a coefficient value */

	/* accepted command, no others can be loaded */
	SPO_LRQ = 0x01;
}

unsigned char SPO256_GetLRQ()
{
    return SPO_LRQ;
}

unsigned char SPO256_GetSBY()
{
    return SPO_SBY;
}
