/////////////////////////////////////////////////////////////////////////////
// Name:        inkz80core.h
// Purpose:     InkZ80 Emulator Core
// Author:      Mark Incley
// Created:     30/10/2007
// Copyright:   Mark Incley
// Licence:     Open Source
/////////////////////////////////////////////////////////////////////////////

#ifndef _INKZ80_CORE_H
#define	_INKZ80_CORE_H

#define AF			_af.w
#define	F			_af.pair.l
#define	A			_af.pair.h

#define	BC			_bc.w
#define C			_bc.pair.l
#define B			_bc.pair.h

#define	DE			_de.w
#define E			_de.pair.l
#define D			_de.pair.h

#define	HL			_hl[0].w
#define L			_hl[0].pair.l
#define H			_hl[0].pair.h
	
#define	IX			_hl[1].w
#define IXL			_hl[1].pair.l
#define IXH			_hl[1].pair.h

#define	IY			_hl[2].w
#define IYL			_hl[2].pair.l
#define IYH			_hl[2].pair.h

#define AF_ALT		_af_alt.w
#define	F_ALT		_af_alt.pair.l
#define	A_ALT		_af_alt.pair.h

#define	BC_ALT		_bc_alt.w
#define C_ALT		_bc_alt.pair.l
#define B_ALT		_bc_alt.pair.h

#define	DE_ALT		_de_alt.w
#define E_ALT		_de_alt.pair.l
#define D_ALT		_de_alt.pair.h

#define	HL_ALT		_hl_alt.w
#define L_ALT		_hl_alt.pair.l
#define H_ALT		_hl_alt.pair.h

#define SP			_sp.w

#define WZ			_wz.w
#define	WZL			_wz.pair.l
#define	WZH			_wz.pair.h

#define	I			_i
#define	R			_r
#define	R2			_r2
#define PC			_pc
#define	IFF1		_iff1
#define	IFF2		_iff2

// IR register "pair" used for Spectrum memory contention
// NB we can't use the raw value of 'R' as we need to correct its top bit, hence getRegisterR()
#define IR			getRegisterR() + 256 * I

// Register selection for handling HL/IX/IY
#define	HLIXIY		_hl[_ddfdPrefix].w
#define	LIXLIYL		_hl[_ddfdPrefix].pair.l
#define	HIXHIYH		_hl[_ddfdPrefix].pair.h

#endif	// _INKZ80_CORE_H
