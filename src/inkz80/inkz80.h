/*
			__      ________     __       __
 __        /\ \    /\_____  \  /'_ `\   /'__`\
/\_\    ___\ \ \/'\\/____//'/'/\ \L\ \ /\ \/\ \
\/\ \ /' _ `\ \ , <     //'/' \/_> _ <_\ \ \ \ \
 \ \ \/\ \/\ \ \ \\`\  //'/'___ /\ \L\ \\ \ \_\ \
  \ \_\ \_\ \_\ \_\ \_\/\_______\ \____/ \ \____/
   \/_/\/_/\/_/\/_/\/_/\/_______/\/___/   \/___/

*/
/////////////////////////////////////////////////////////////////////////////
// Name:        inkz80.h
// Purpose:     InkZ80 Z80 Emulator
// Author:      Mark Incley
// Created:     30/10/2007
// Copyright:   Mark Incley
// Licence:     OpenSource
/////////////////////////////////////////////////////////////////////////////


#ifndef _INKZ80_H
#define	_INKZ80_H

#include <string>
#include <vector>
#include <set>
#include <array>

#ifndef _INKZ80_HAVE_BASIC_TYPES
typedef	unsigned char	BYTE;
typedef	signed   char	SBYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;
#endif

typedef unsigned int	TStates;

enum Z80BreakpointType
{
	BreakpointNone				= 0,
	BreakpointMemoryRead		= 1,
	BreakpointMemoryWrite		= 2,
	BreakpointMemoryReadWrite	= BreakpointMemoryRead + BreakpointMemoryWrite,
	BreakpointPortRead			= 4,
	BreakpointPortWrite			= 8,
	BreakpointPortReadWrite		= BreakpointPortRead + BreakpointPortWrite,
	BreakpointOpcodeRead		= 16,
	BreakpointRCountdown		= 32,
	BreakpointIntAccepted		= 64,
	BreakpointDIHalt			= 128,
	BreakpointExitExecution		= 256,		// Used by RequestEndExecution()
	BreakpointUser				= 512		// Supplied by client - e.g. break on an event firing
};


enum Z80BreakpointField
{
	BreakpointFieldType,
	BreakpointFieldId,
	BreakpointFieldHitCount,
	BreakpointFieldHitInstructionCountWhenHit,
	BreakpointFieldUserData,
	BreakpointFieldAddress,
	BreakpointFieldAddressMask,
	BreakpointFieldEnabled,
	BreakpointFieldOneShot
};


enum class Z80BreakpointHitMode
{
	Always,
	WhenEqual,
	WhenGreaterOrEqual,
	WhenMultipleOf
};


struct Z80Breakpoint
{
	Z80Breakpoint() :
		type(BreakpointOpcodeRead),
		id(0),
		hitCountMode(Z80BreakpointHitMode::Always),
		hitCount(0),
		hitCountParam(1),
		instructionCountWhenHit(-1),
		userData(NULL),
		address(0),
		addressMask(0xFFFF),
		enabled(true),
		oneShot(false),
		hidden(false)
	{
	}

	bool HasCondition() const			{return !condition.empty();}

	Z80BreakpointType	type;
	unsigned			id;
	Z80BreakpointHitMode hitCountMode;
	unsigned			hitCount;
	unsigned			hitCountParam;
	unsigned			instructionCountWhenHit;
	void *				userData; 
	WORD				address;				// Port or memory address we're interested in
	WORD				addressMask;			
	bool				enabled;
	bool				oneShot;				// Self-deleting once hit
	bool				hidden;					// Not used by InkZ80 but useful for clients to hide step-over breakpoints, etc.
	std::string			condition;				// e.g. "hl=123", which has to be evaluated by the derived class
};


class Z80
{
public:
	// Few constants
	static const size_t		AddressableMemorySize = 0x10000;
	static const unsigned	BreakpointIdNone = 0;

	static const BYTE		Bit7 =		0x80;
	static const BYTE		Bit6 =		0x40;
	static const BYTE		Bit5 =		0x20;
	static const BYTE		Bit4 =		0x10;
	static const BYTE		Bit3 =		0x08;
	static const BYTE		Bit2 =		0x04;
	static const BYTE		Bit1 =		0x02;
	static const BYTE		Bit0 =		0x01;

	// Z80's Flags
	static const BYTE		FLAG_S =	Bit7;		// Sign
	static const BYTE		FLAG_Z =	Bit6;		// Zero
	static const BYTE		FLAG_BIT5 =	Bit5;		// Depends on instruction
	static const BYTE		FLAG_H =	Bit4;		// Half-carry
	static const BYTE		FLAG_BIT3 =	Bit3;		// Depends on instruction
	static const BYTE		FLAG_P =	Bit2;		// Parity / oVerflow
	static const BYTE		FLAG_N =	Bit1;		// Add/Substract
	static const BYTE		FLAG_C =	Bit0;		// Carry

	static const BYTE		FLAG_V	=	FLAG_P;		// Overflow
	static const BYTE		FLAGS_35 = FLAG_BIT3 | FLAG_BIT5;

	union reg
	{
		struct
		{
#ifdef INKZ80_ARCH_BIGENDIAN
			BYTE		h, l;
#else
			BYTE		l, h;
#endif
		}
		pair;
		WORD	w;
	};

	enum LineState
	{
		LineLow,
		LineHigh
	};

	enum FlipFlop
	{
		FlipFlopReset,
		FlipFlopSet
	};

	enum InterruptMode
	{
		IM0,
		IM1,
		IM2
	};

	enum InstructionPrefix
	{
		PrefixNone,
		PrefixIX,
		PrefixIY
	};

	enum ExecutionState
	{
		XSRunning,
		XSExhaustedTStates,
		XSInfiniteDDFDLoop,
		XSBreakpointHit,
		XSExitRequested
	};

	enum Flag
	{
		Flag_Carry,
		Flag_AddSubtract,
		Flag_ParityOverflow,
		Flag_Bit3,
		Flag_HalfCarry,
		Flag_Bit5,
		Flag_Zero,
		Flag_Sign
	};

	// C'tor, D'tor
	Z80();
	virtual ~Z80();

	static bool		IsBreakpointFieldUsedByType(Z80BreakpointType type, Z80BreakpointField field);
	
	//
	// Power on and reset
	//
	void			powerOn();
	void			reset();

	//
	// Register getters
	//
	BYTE			getRegisterI() const;
	BYTE			getRegisterR() const;
	WORD			getRegisterAF() const;
	WORD			getRegisterBC() const;
	WORD			getRegisterDE() const;
	WORD			getRegisterHL() const;
	WORD			getRegisterIX() const;
	WORD			getRegisterIY() const;
	WORD			getRegisterAFAlt() const;
	WORD			getRegisterBCAlt() const;
	WORD			getRegisterDEAlt() const;
	WORD			getRegisterHLAlt() const;
	WORD			getRegisterSP() const;
	WORD			getRegisterPC() const;
	WORD			getRegisterWZ() const;

	//
	// Register setters
	//
	void			setRegisterI(const BYTE i);
	void			setRegisterR(const BYTE r);
	void			setRegisterAF(const WORD af);
	void			setRegisterBC(const WORD bc);
	void			setRegisterDE(const WORD de);
	void			setRegisterHL(const WORD hl);
	void			setRegisterIX(const WORD ix);
	void			setRegisterIY(const WORD iy);
	void			setRegisterAFAlt(const WORD afAlt);
	void			setRegisterBCAlt(const WORD bcAlt);
	void			setRegisterDEAlt(const WORD deAlt);
	void			setRegisterHLAlt(const WORD hlAlt);
	void			setRegisterSP(const WORD sp);
	void			setRegisterPC(const WORD pc);
	void			setRegisterWZ(const WORD wz);

	FlipFlop		getIFF1() const						{return _iff1;}
	void			setIFF1(const FlipFlop iff1);
	FlipFlop		getIFF2() const						{return _iff2;}
	void			setIFF2(const FlipFlop iff2);
	bool			isFlagSet(Flag flag) const;
	bool			isAltFlagSet(Flag flag) const;
	void			setFlag(Flag flag, bool set);
	void			setAltFlag(Flag flag, bool set);
	bool			didLastInstructionModifyFlags () const			{return _lastInstructionModifiedFlags;}
	void			setLastInstructionModifiedFlags(bool modified)	{_lastInstructionModifiedFlags = modified;}

	InstructionPrefix getInstructionPrefix() const		{return _ddfdPrefix;}
	bool			isPostEI() const					{return _postEI;}
	void			setPostEI(const bool postEI);
	bool			isHalted() const					{return _isHalted;}
	void			setHalted(const bool halted);
	LineState		getM1Line() const					{return _M1Line;}
	LineState		getHALTLine() const					{return _HALTLine;}
	bool			isReadingOpCode() const				{return _M1Line == LineLow;}

	void			burnTS(const int ts)				{_TStateCount += ts;}

	// Interrupts
	void			setInterruptMode(const InterruptMode im);
	InterruptMode	getInterruptMode() const			{return _interruptMode;}
	bool			isNMIPending() const;
	LineState		getNMILine() const					{return _NMILine;}
	void			setNMILine(const LineState lineState);
	bool			isINTPending() const;
	LineState		getINTLine() const					{return _INTLine;}
	void			setINTLine(const LineState lineState);
	WORD			getINTVector() const;
	bool			checkIRQ();
	LineState		getMREQLine() const					{return _MREQLine;}
	LineState		getRDLine() const					{return _RDLine;}
	LineState		getWRLine() const					{return _WRLine;}
	LineState		getIORQLine() const					{return _IORQLine;}

	void			RequestEndExecution();
	TStates			executeSingleInstruction();
	TStates			executeTStates(const TStates ts);
	unsigned 		getElapsedTStates() const;
	TStates			getTStateCount() const				{return _TStateCount;}
	void			setTStateCount(TStates ts)			{_TStateCount = ts;}
	unsigned		getNumInstructionsExecuted() const	{return _numInstructionsExecuted;}

	void			generateNonMaskableInt();
	ExecutionState	getExecutionState() const			{return m_executionState;}


	//
	// Breakpoints
	//
	bool			GetBreakpoint(size_t idx, Z80Breakpoint & brk) const;
	bool			FindBreakpoint(unsigned id, Z80Breakpoint & brk) const;
	bool			FindBreakpointFromType(Z80BreakpointType type, WORD address, Z80Breakpoint & brk) const;
	void			MakeBreakpointAddressSet(Z80BreakpointType type, std::set<WORD> & brkMap) const;
	unsigned		AddBreakpoint(const Z80Breakpoint & brk);
	bool			UpdateBreakpoint(const Z80Breakpoint & brk);
	bool			DeleteBreakpoint(unsigned id);
	void			DeleteBreakpointsOfType(Z80BreakpointType type);
	void			DeleteAllBreakpoints(bool includeHidden);
	size_t			GetBreakpointCount() const			{return m_breakpoints.size();}
	void			SetRCountdown(int rChanges);
	Z80BreakpointType GetBreakpointTypeHit() const		{return m_breakpointTypeHit;}
	unsigned		GetBreakpointIdHit() const			{return m_breakpointIdHit;}	


	// Misc util
	int				getRRegisterCountdown() const		{return _RCountRemaining;}

	static const std::string & getVersionString();

private:
	TStates			ExecuteResume();
	void			ExecuteInstruction();

	BYTE			CalcFlagH(const WORD result, const BYTE op1, const BYTE op2);
	BYTE			CalcFlagVAdd(const WORD result, const BYTE op1, const BYTE op2);
	BYTE			CalcFlagVSub(const WORD result, const BYTE op1, const BYTE op2);

	BYTE			CalcFlagH(const DWORD result, const WORD op1, const WORD op2);
	BYTE			CalcFlagVAdd(const DWORD result, const WORD op1, const WORD op2);
	BYTE			CalcFlagVSub(const DWORD result, const WORD op1, const WORD op2);

	void			IncR();
	void			SetF(BYTE f);
	void			UnHalt();

	// This group of read/writers all consume TS
	void			discardByte();
	void			discardWord();
	BYTE			ReadOpCode();
	BYTE			ReadByte();
	BYTE			ReadByte(WORD addr);
	WORD			ReadWord();
	WORD			ReadWord(WORD addr);
	void			WriteByte(WORD addr, BYTE val);
	void			WriteWord(WORD addr, WORD val);
	BYTE			ReadPort(WORD port);
	void			WritePort(WORD port, BYTE val);

	void			AddHL(WORD val16);
	void			AdcHL(WORD val16);
	void			SbcHL(WORD val16);

	WORD			GetEA();
	void			GetEAImm(WORD & addr, BYTE & val);

	void			Daa();
	void			Neg();
	void			Dec(BYTE & r);
	void			Inc(BYTE & r);

	void			Jr();
	void			Jr(bool conditionMet);
	void			Jp();
	void			Jp(bool conditionMet);
	void			Call();
	void			Call(bool conditionMet);
	void			Rst(WORD addr);
	void			Ret();
	void			Ret(bool conditionMet);
	void			Add(BYTE value);
	void			Adc(BYTE value);
	void			Sub(BYTE value);
	void			Sbc(BYTE value);
	void			Cp(BYTE value);
	void			And(BYTE value);
	void			Xor(BYTE value);
	void			Or(BYTE value);
	void			Push(WORD value);
	WORD			Pop();

	void			Retn();
	void			Reti();
	void			Im0();
	void			Im1();
	void			Im2();
	void			LdIA();
	void			LdRA();
	void			LdAI();
	void			LdAR();
	void			Rrd();
	void			Rld();

	void			LdNNRR(WORD & r16);
	void			LdRRNN(WORD & r16);
	void			InC(BYTE & r);
	void			InCF();
	void			OutC(BYTE r);

	void			Ldi();
	void			Cpi();
	void			Ini();
	void			Outi();
	void			Ldd();
	void			Cpd();
	void			Ind();
	void			Outd();

	// Block instructions
	void			Ldir();
	void			Cpir();
	void			Inir();
	void			Otir();
	void			Lddr();
	void			Cpdr();
	void			Indr();
	void			Otdr();

	void			OpCB();
	void			OpED();

	// Rotate ops
	// Use templates to specify the rotation functions so that they're inlined,
	// which can't be done when using member function pointers.
	template <BYTE (Z80::* FN)(BYTE)>
	void			RotShift(BYTE & r);
	template <BYTE (Z80::* FN)(BYTE)>
	void			RotShiftHL();

	// Rotate functions
	BYTE			Rlc(BYTE val);
	BYTE			Rrc(BYTE val);
	BYTE			Rl(BYTE val);
	BYTE			Rr(BYTE val);
	BYTE			Sla(BYTE val);
	BYTE			Sra(BYTE val);
	BYTE			Sll(BYTE val);
	BYTE			Srl(BYTE val);

	void			Bit(BYTE bitmask, BYTE val);
	void			BitHL(BYTE bitmask);
	void			Res(BYTE bitmask, BYTE & r);
	void			ResHL(BYTE bitmask);
	void			Set(BYTE bitmask, BYTE & r);
	void			SetHL(BYTE bitmask);

	void			EDNop(BYTE opcode);

	//
	// Hooks to allow emulation of memory contention
	//
	virtual void	applyMemoryContention(int ts, WORD addr);
	virtual void	applyMemoryContentionCompound(int ts, WORD addr);
	virtual void	acknowledgeINT(BYTE & busValue)			{return;}	// Alter busValue if you wish -  on return it's used for IM2 vector address calculation
	virtual void	acknowledgeNMI()						{return;}
	virtual void	OnIntTaken()							{return;}
	virtual void	OnNMITaken()							{return;}
	virtual void	acknowledgeEDNOP(WORD pc, BYTE opcode)	{return;}
	virtual void	OnExitReti()							{return;}
	
	// Pure virtual functions for the client to supply
	virtual BYTE	memoryReadRaw(WORD addr) = 0;				// Raw memory read from the client (no contention applied)
	virtual void	memoryWriteRaw(WORD addr, BYTE val) = 0;	// Raw memory write from the client (no contention applied)

	// Because the ZX Spectrum (as one example) requires very specific handling of port IO, these two functions are pure virtual.
	// They must consume a minimum of 4 t-states each, in addition to any system specific contention or additional delay required.
	virtual BYTE	portRead(WORD port) = 0;
	virtual void	portWrite(WORD port, BYTE val) = 0;	// Registers

	// Debugger support
	virtual bool	TestBreakpointCondition(const Z80Breakpoint & bp)	{return false;}

	// Member vars
	reg				_af;
	reg				_bc;
	reg				_de;
	reg				_hl[3];					// HL = _hl[0], IX = _hl[1], IY = _hl[2]
	reg				_af_alt;
	reg				_bc_alt;
	reg				_de_alt;
	reg				_hl_alt;
	reg				_sp;
	reg				_wz;
	BYTE			_i;
	BYTE			_r;
	BYTE			_r2;					// r as last set by ld r,a to preserve bit7
	BYTE			_busValue;
	WORD			_pc;
	FlipFlop		_iff1;
	FlipFlop		_iff2;
	LineState		_M1Line;				// Together with MREQ this indicates an opcode fetch. Together with IORQ this indicates a maskable interrupt acknowledge.
	LineState		_MREQLine;			// Together with M1 this indicates an opcode fetch, otherwise a memory read or write.
	LineState		_HALTLine;			// When Z80 has executed a HALT
	LineState		_IORQLine;			// Together with M1 then an interrupt is being acknowledged, otherwise an i/o read or write.
	LineState		_RDLine;				// read memory or read i/o
	LineState		_WRLine;				// write memory or write i/o
	
	// Interrupts
	LineState		_NMILine;				// NMI line - note that it still has to wait for the DD/FD prefix to be cleared before acted upon
	LineState		_INTLine;				// The maskable interrupt request line. When low and EI, a maskable interrupt is generated
	
	InterruptMode	_interruptMode;
	bool			_isHalted;
	bool			_postEI;
	bool			_NMIPending;			// Whether an NMI is pending (i.e. the NMI line gone from high to low)
	bool			_lastInstructionModifiedFlags;

	InstructionPrefix	_ddfdPrefix;		// 0 for HL (i.e. no DD or FD prefix active), 1 for DD/IX, 2 for FD/IY

	// Execution
	TStates			_TStateCount;			// Ongoing t-state count
	TStates			_TStateCountUntil;		// Execute instructions until TState count reaches this value
	TStates			_TStateCountAtStart;	// Value of _TStateCount at start of Execute
	unsigned		_numInstructionsExecuted;
	char			_opCBDisplacement;
	bool			_newModifiedFlags;

	// Breakpoints
	void			RefreshBreakpointBitmap();
	bool			TestHitBreakpoint(Z80BreakpointType type);		// For breakpoints that don't need address (such as on IRQ, etc)
	bool			TestHitBreakpoint(Z80BreakpointType type, WORD address);
	bool			DoHitBreakpoint(Z80Breakpoint & bp);

	// R register countdown is supported via the breakpoint interface
	int					_RCountRemaining;

	// Breakpoints
	ExecutionState		m_executionState;
	std::vector<Z80Breakpoint> m_breakpoints;
	Z80BreakpointType	m_breakpointTypeHit;
	unsigned			m_breakpointIdHit;
	unsigned			m_breakpointIdSeed;
	unsigned			m_enabledBreakpointTypeBitmap;
};

#endif	//_INKZ80_H
