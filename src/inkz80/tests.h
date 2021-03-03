#ifndef _TESTS_H
#define _TESTS_H

#include <list>
#include <vector>

class Event
{
public:
	enum Type
	{
		Event_MemRead,
		Event_MemWrite,
		Event_MemContend,
		Event_PortRead,
		Event_PortWrite,
		Event_PortContend
	};

	Event()
	{
		Clear();
	}

	Event(int ts, Type t, WORD addr, BYTE b):
		time(ts), type(t), address(addr), data(b)
	{
	}

	void Clear()
	{
		time = 0;
		type = Event_MemRead;
		address = 0;
		data = 0;
	}

	int			time;
	Type		type;
	WORD		address;
	BYTE		data;
};


class MemBlock
{
public:
	MemBlock()
	{
		Clear();
	}

	void Clear()
	{
		memStart = 0;
		memory.clear();
	}

	WORD				memStart;
	std::vector<BYTE>	memory;
};


class Registers
{
public:
	Registers()
	{
		Clear();
	}

	void Clear()
	{
		af = bc = de = hl = af2 = bc2 = de2 = hl2 = ix = iy = sp = pc = 0;
		i = r = iff1 = iff2 = im = 0;
		halted = false;
		tstates = 0;
	}
	
	WORD	af;
	WORD	bc;
	WORD	de;
	WORD	hl;
	WORD	af2;
	WORD	bc2;
	WORD	de2;
	WORD	hl2;
	WORD	ix;
	WORD	iy;
	WORD	sp;
	WORD	pc;
	BYTE	i;
	BYTE	r;
	BYTE	iff1;
	BYTE	iff2;
	BYTE	im;
	bool	halted;
	int		tstates;
};


class Test
{
public:
	Test()
	{
		Clear();
	};

	void Clear()
	{
		name.clear();
		setupMemBlocks.clear();
		setupRegisters.Clear();
		expectedRegisters.Clear();
		expectedEvents.clear();
		actualEvents.clear();
		passed = executed = false;
		TStatesExecuted = 0;
	}

	std::string			name;
	Registers			setupRegisters;
	std::vector<MemBlock>setupMemBlocks;
	Registers			expectedRegisters;
	std::vector<Event>	expectedEvents;
	std::vector<MemBlock>	expectedMemBlocks;
	std::vector<Event>	actualEvents;
	bool				passed;
	bool				executed;
	int					TStatesExecuted;
};

#endif
