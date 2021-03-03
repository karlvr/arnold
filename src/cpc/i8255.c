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
#include "i8255.h"
#include "cpcglob.h"
#include "cpc.h"

/**************************************************************************/
/* PPI */
/*
 PPI_control bit usage

 Bit            Function

 7              Function to perform (1: mode set, 0: bit set/reset)
 6...0          Used depending on function

 Bit            Function

 7              1: mode set
 6,5            Mode selection for Port A and Port C (upper)
 4              Port A control (1: input, 0:output)
 3              Port C (upper) control (1: input, 0:output)
 2              Mode selection for Port B and Port C (lower)
 1              Port B control (1: input, 0:output)
 0              Port C (lower) control (1: input, 0:output)

 Mode selection for Port A and Port C (upper)

 Bit 6          bit 5

 0              0               Mode 0
 0              1               Mode 1
 1              0               Mode 2
 1              1               Mode 2

 Mode selection for Port B and Port C (lower)

 0      Mode 0
 1      Mode 1

 Bit            Function

 7              0: bit set/reset flag
 6..4           Not used (doesnt matter what value is)
 3..1           Bit to set/reset
 0              Bit function (1:set, 0:reset)
*/
#define PPI_CONTROL_BYTE_FUNCTION               0x080
#define PPI_CONTROL_PORT_A_CU_MODE1             0x040
#define PPI_CONTROL_PORT_A_CU_MODE0             0x020
#define PPI_CONTROL_PORT_A_STATUS               0x010
#define PPI_CONTROL_PORT_C_UPPER_STATUS 0x008
#define PPI_CONTROL_PORT_B_CL_MODE              0x004
#define PPI_CONTROL_PORT_B_STATUS               0x002
#define PPI_CONTROL_PORT_C_LOWER_STATUS 0x001

void ppi_refresh_intr();

/* Note: When a port is set to output, when data is read from it the result is 0's */

/* when port is set to input, the output of that port is 0x0ff */

typedef struct
{
	/* latched data written to outputs */
	unsigned char latched_outputs[4];
	unsigned char latches[4];
	unsigned char bit_set_reset_mask;
	unsigned char status_mask;
	unsigned char write_mask[4];
	unsigned char status;

	unsigned char final_outputs[4];
	/* will be 1 if PPI is driving this port as output */
	/* otherwise it will be 0 */
	unsigned char final_outputs_mask[4];

	/* current inputs */
	unsigned char	inputs[4];

	/* masks for input/output. 0x0ff = keep input, 0x00 = keep output */
	unsigned char	io_mask[4];
	/* control information */
	unsigned char	control;

	unsigned char   inte;
} PPI_8255;

static PPI_8255	ppi8255;


void    PPI_Reset(void)
{
	/* Intel 8255 docs say all ports will be in input mode */
	/* looks like all will be mode 0 too */
	/*  as per MTM docs. */
	PPI_WriteControl(0x09b);
}


void ppi_refresh_input(int nPort, int nData)
{
	ppi8255.inputs[nPort] = nData;

	switch (nPort)
	{
			/* control port in mode 1 or 2 */
		case 2:
		{
			if ((ppi8255.control & (1<<6))!=0)
			{
				/* group A, mode 2 */

				/* stb input is low */
				if (ppi8255.inputs[nPort] & (1<<4))
				{
					/* latch data into port A */
					ppi8255.latches[0] = ppi8255.inputs[0];

					/* ibf goes high */
					ppi8255.status|=(1<<5);

					/* refresh interrupts */
					ppi_refresh_intr();
				}
			}
			else
			{
				BOOL bRefreshIntr = FALSE;

				if ((ppi8255.control & (1<<5))!=0)
				{
					/* group A mode 1 */
					if (ppi8255.inputs[nPort] & (1<<4))
					{
						/* latch data into port A */
						ppi8255.latches[0] = ppi8255.inputs[0];

						/* ibf goes high */
						ppi8255.status|=(1<<5);
						bRefreshIntr = TRUE;
					}
				}

				if ((ppi8255.control & (1<<2))!=0)
				{
					/* group B mode 1 */
					if (ppi8255.inputs[nPort] & (1<<2))
					{
						/* store data in latch */
						ppi8255.latches[1] = ppi8255.inputs[1];

						/* ibf goes high */
						ppi8255.status|=(1<<1);
						bRefreshIntr = TRUE;
					}
				}
				if (bRefreshIntr)
				{
					ppi_refresh_intr();
				}
			}				
		}
		break;

		default
				:
			break;
	}
}

void ppi_refresh_output(int nPort, int nData)
{



}


 static int ppi_read_port(int port_index)
{
	return ((ppi8255.inputs[port_index] & ppi8255.io_mask[port_index]) |
			(ppi8255.latched_outputs[port_index] & (~ppi8255.io_mask[port_index])));
}

 static void ppi_write_port(int port_index, int data)
{
	ppi8255.latched_outputs[port_index] = (data&ppi8255.write_mask[port_index])|(ppi8255.latched_outputs[port_index]&~ppi8255.write_mask[port_index]);

	ppi8255.final_outputs[port_index] = ((ppi8255.latched_outputs[port_index] & (~ppi8255.io_mask[port_index])) |
										 (0x0ff & ppi8255.io_mask[port_index]));
}

unsigned int PPI_ReadPort(int nPort)
{
	ppi8255.inputs[nPort] = PPI_GetPortInput(nPort);

	switch (nPort)
	{

		case 0:
		{
			if ((ppi8255.control & (1<<6))!=0)
			{
				/* group A, mode 2 */
				if (ppi8255.status&(1<<5))
				{
					/* stb is set high after read */
					ppi8255.status|=(1<<4);

					/* ibf is set low after read */
					ppi8255.status&=~(1<<5);

					ppi_refresh_intr();

					/* store data in latch */
					return ppi8255.latches[0];
				}
				else
				{
					/* if obf is low, return data in latch */
					return ppi8255.latches[0];
				}
			}
			else
				if (ppi8255.control & (1<<5))
				{
					/* mode 1 */
					if (ppi8255.status&(1<<5))
					{
						/* read */

						/* stb is set high after read */
						ppi8255.status|=(1<<4);
						/* if goes low after read */
						ppi8255.status&=~(1<<5);

						ppi_refresh_intr();

						/* store data in latch */
						return ppi8255.latches[0];
					}
					else
					{
/* if obf is low, return data in latch */

						return ppi8255.latches[0];
					}
				}
		}
		break;

		case 1:
		{
			if (ppi8255.control & (1<<2))
			{
				/* mode 1 */
				if (ppi8255.status&(1<<1))
				{
					/* read */

					/* stb is set high after read */
					ppi8255.status|=(1<<2);
					/* if goes low after read */
					ppi8255.status&=~(1<<1);

					ppi_refresh_intr();

					/* store data in latch */
					return ppi8255.latches[1];
				}
				else
				{
					/* write */
					/* if obf is low, return data in latch */
					return ppi8255.latches[1];
				}
			}

		}
		break;

		case 2:
		{
			return ((ppi8255.status & ppi8255.status_mask) | (ppi_read_port(nPort)&(~ppi8255.status_mask)));
		}
		break;
	}

	return ppi_read_port(nPort);
}

int PPI_GetOutputPort(int nPort)
{
	return ppi8255.final_outputs[nPort];
}

int PPI_GetOutputMaskPort(int nPort)
{
	return ppi8255.final_outputs_mask[nPort];
}

void ppi_refresh_intr()
{
	/* group A mode 2? */
	if ((ppi8255.control & (1<<6))!=0)
	{
		/* mode 2 */

		/* intra */
		ppi8255.status&=~(1<<3);

		/* inte1 = bit 6 (output) */
		/* inte2 = bit 4 (input) */
		if (
			/* inte1 & obfa */
			(
				(ppi8255.inte & (1<<6)) &&
				/* obf */
				(ppi8255.status & (1<<7)) &&
				/* ack */
				(ppi8255.status & (1<<6))
			) ||
			(
				/* inte2 & ibfa */
				(ppi8255.inte & (1<<4)) &&
				/* stb */
				(ppi8255.status & (1<<4)) &&
				/* ack */
				(ppi8255.status & (1<<6))
			)
		)
		{
			/* intra */
			ppi8255.status|=(1<<3);
		}
	}
	else
	{
		/* group A is mode 1? */

		if ((ppi8255.control & (1<<5))!=0)
		{
			/* mode 1 */

			ppi8255.status&=~(1<<3);

			if (ppi8255.control & (1<<4))
			{
				/* intea = pc4
				 stba = pc4
				 ibfa = pc5
				 intr = pc3
				*/

				/* input */
				if (
					/* inte */
					(ppi8255.inte & (1<<6)) &&
					/* ibf */
					(ppi8255.status & (1<<5)) &&
					/* stb */
					(ppi8255.status & (1<<4))
				)
				{
					ppi8255.status|=(1<<3);
				}
			}
			else
			{
				/*   obfa = pc7
				   acka = pc6
				   intra = pc3
				   intea = pc6
				*/
				/* output */
				if (
					/* inte */
					(ppi8255.inte & (1<<6)) &&
					/* ack */
					(ppi8255.status & (1<<6)) &&
					/* obf */
					(ppi8255.status & (1<<7))
				)
				{
					ppi8255.status|=(1<<3);
				}
			}
		}
	}
	/* group B is mode 1? */
	if ((ppi8255.control & (1<<2))!=0)
	{
		/* mode 1 */

		ppi8255.status&=~(1<<0);

		if (ppi8255.control & (1<<4))
		{
			/* inteb = pc2
			 stbb = pc2
			 ibfb = pc1
			 intr = pc0
			*/

			/* input */
			if (
				/* inte */
				(ppi8255.inte & (1<<2)) &&
				/* ibf */
				(ppi8255.status & (1<<1)) &&
				/* stb */
				(ppi8255.status & (1<<2))
			)
			{
				ppi8255.status|=(1<<0);
			}
		}
		else
		{
			/* obfb = pc1
			   ackb = pc2
			   intrb = pc0
			   inteb = pc2
			   */
			/* output */
			if (
				/* inte */
				(ppi8255.inte & (1<<2)) &&
				/* ack */
				(ppi8255.status & (1<<2)) &&
				/* obf */
				(ppi8255.status & (1<<1))
			)
			{
				ppi8255.status|=(1<<0);
			}
		}
	}
}

void    PPI_WritePort(int nPort, int Data)
{
	switch (nPort)
	{
		case 0:
		{
			if ((ppi8255.control & (1<<6))!=0)
			{
				/* mode 2 */

				/* store data in latch */
				ppi8255.latches[0]=Data;

				/* obf is set low after write */
				ppi8255.status&=~(1<<7);

				ppi_refresh_intr();
				return;
			}
			else
				if (ppi8255.control & (1<<5))
				{
					/* mode 1 */
					if ((ppi8255.control & (1<<4))==0)
					{
						/* obf is set low after write */
						ppi8255.status&=~(1<<7);

						ppi_refresh_intr();

						/* store data in latch */
						ppi8255.latches[0]=Data;
						return;
					}
				}
		}
		break;

		case 1:
		{
			if (ppi8255.control & (1<<2))
			{
				if ((ppi8255.control & (1<<1))==0)
				{

					/* mode 1 */

					/* obf is set low after write */
					ppi8255.status&=~(1<<1);

					ppi_refresh_intr();

					/* store data in latch */
					ppi8255.latches[1]=Data;
					return;
				}
			}

		}
		break;

		case 2:
		{

		}
		break;
	}

	ppi_write_port(nPort, Data);

	PPI_SetPortOutput(nPort, ppi8255.final_outputs[nPort]);
}

void    PPI_WriteControl(int Data)
{
	if (Data & PPI_CONTROL_BYTE_FUNCTION)
	{
		/* Configuration control byte */
		unsigned char PPI_PortControl = (unsigned char)Data;

		/* on CPC and KCC, the PPI resets the port values when the
		configuration is set. On CPC+ it doesn't do this. */
		ppi8255.latched_outputs[0] = ppi8255.latched_outputs[1] = ppi8255.latched_outputs[2] = 0;

		ppi8255.control = PPI_PortControl;

		if (Data & PPI_CONTROL_PORT_A_STATUS)
		{
			/* port A is input */
			ppi8255.io_mask[0] = 0x0ff;
		}
		else
		{
			/* port A is output */
			ppi8255.io_mask[0] = 0x000;
		}
		ppi8255.write_mask[0] = 0x0ff;
		ppi8255.final_outputs_mask[0] = ppi8255.io_mask[0]^0x0ff;

		/* PORT B */
		if (Data & PPI_CONTROL_PORT_B_STATUS)
		{
			/* port B is in input mode */
			ppi8255.io_mask[1] = 0x0ff;
		}
		else
		{
			/* port B is in output mode, return data written to it */
			ppi8255.io_mask[1] = 0x000;
		}
		ppi8255.write_mask[1] = 0x0ff;
		ppi8255.final_outputs_mask[1] = ppi8255.io_mask[1]^0x0ff;

		ppi8255.io_mask[2] = 0;
		ppi8255.write_mask[2] = 0;
		ppi8255.bit_set_reset_mask=0;
		ppi8255.status_mask = 0;

		if (ppi8255.control & (1<<6))
		{
			/* port A mode 2 */
			/* no data bits of port C can be set/reset */

			/* these bits are inputs */
			ppi8255.io_mask[2] |= (1<<6)|(1<<4);
			ppi8255.status_mask |= (1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3);
		}
		else
			if (ppi8255.control & (1<<5))
			{
				/* port A mode 1 */
				if (ppi8255.control & (1<<4))
				{
					/* input */
					/* bit 7,6 data bits may be set with bit set/reset */
					ppi8255.bit_set_reset_mask |= 0x03<<6;
					/* bit 4 is an input */
					ppi8255.io_mask[2] |= (1<<4);
					ppi8255.status_mask |= (1<<3)|(1<<4)|(1<<5);

				}
				else
				{
					/* output */

					/* bit 5,4 data bits may be set with bit set/reset */
					ppi8255.bit_set_reset_mask |= 0x04<<6;
					/* bit 6 is an input */
					ppi8255.io_mask[2] |= (1<<6);
					ppi8255.status_mask |= (1<<3)|(1<<6)|(1<<7);
				}
			}
			else
			{

				/* port A mode 0 */
				if ((Data & PPI_CONTROL_PORT_C_UPPER_STATUS)==0)
				{
					/* upper part of port is in output mode */
				}
				else
				{
					/* upper part of port is in input mode */
					ppi8255.io_mask[2] |=0x0f0;
				}
				/* all 4 upper bits may be set with set/reset */
				ppi8255.bit_set_reset_mask |= 0x0f0;
				ppi8255.write_mask[2] |= 0x0f0;

			}


		if ((ppi8255.control & (1<<2))==0)
		{
			/* port B is mode 0 */

			if ((ppi8255.control & ((1<<6)|(1<<5)))!=0)
			{
				/* if port A is in mode 1 or mode 2 we loose bit 3 */
				ppi8255.bit_set_reset_mask|=0x07;
				ppi8255.write_mask[2] |= 0x007;

				/* port A mode 0 */
				if ((Data & PPI_CONTROL_PORT_C_LOWER_STATUS)==0)
				{
					/* lower part of port is in output mode */
				}
				else
				{
					ppi8255.io_mask[2] |=0x07;
				}
			}
			else
			{
				ppi8255.bit_set_reset_mask|=0x0f;
				ppi8255.write_mask[2] |= 0x00f;


				if ((Data & PPI_CONTROL_PORT_C_LOWER_STATUS)==0)
				{
					/* lower part of port is in output mode */
				}
				else
				{
					ppi8255.io_mask[2] |=0x0f;
				}
			}
		}
		else
		{
			/* port B is mode 1 */

			/* bit 2 is an input */
			ppi8255.io_mask[2] |=(1<<2);
			ppi8255.status_mask |= 0x07;
		}
		ppi8255.final_outputs_mask[2] = ppi8255.io_mask[2]^0x0ff;

		/* intel 8255 says all outputs are reset, including status flip flops */

		ppi8255.inte = 0;
		ppi8255.status = 0;
		ppi8255.latches[0] = 0;
		ppi8255.latches[1] = 0;
		ppi8255.latches[2] = 0;

		if ((ppi8255.control & (1<<6))!=0)
		{
			/* group A mode 2 */

			/* obf is set low after write */
			ppi8255.status|=(1<<7);
		}
		else
			if (ppi8255.control & (1<<5))
			{
				/* mode 1 */
				/* obf is set low after write */
				ppi8255.status|=(1<<7);
			}

		if (ppi8255.control & (1<<2))
		{
			/* mode 1 */

			ppi8255.status|=(1<<1);
		}

		ppi_write_port(0, ppi8255.latched_outputs[0]);
		ppi_write_port(1, ppi8255.latched_outputs[1]);
		ppi_write_port(2, ppi8255.latched_outputs[2]);

		/* do C first because AY control is driven from this */
		/* really output will be simultaneous */
		PPI_SetPortOutput(2,ppi8255.final_outputs[2]);
		PPI_SetPortOutput(0,ppi8255.final_outputs[0]);
		PPI_SetPortOutput(1, ppi8255.final_outputs[1]);

		ppi_refresh_intr();
	}
	else
	{
		/* Bit Set/Reset control bit */

		int     BitIndex = (Data>>1) & 0x07;

		if (Data & 1)
		{
			/* set bit */

			int     OrData;

			OrData = (1<<BitIndex);

			ppi8255.inte|=OrData;
			ppi8255.latched_outputs[2]|=(OrData&ppi8255.bit_set_reset_mask);
		}
		else
		{
			/* clear bit */

			int     AndData;


			AndData = (~((1<<BitIndex)));
			ppi8255.inte&=AndData;
			AndData = (~((1<<BitIndex)&ppi8255.bit_set_reset_mask));
			ppi8255.latched_outputs[2]&=AndData;
		}

		ppi_refresh_intr();

		ppi_write_port(2, ppi8255.latched_outputs[2]);
		PPI_SetPortOutput(2, ppi8255.final_outputs[2]);
	}
}

int PPI_ReadControl(void)
{
	/* default to value returned by CPC */
	return 0x0ff;
}



int PPI_GetControlForSnapshot(void)
{
	return ppi8255.control;
}

void    PPI_SetPortDataFromSnapshot(int nPort, int Data)
{
	ppi8255.inputs[nPort] = Data;
}

int PPI_GetPortDataForSnapshot(int nPort)
{
	return ppi8255.inputs[nPort];
}
