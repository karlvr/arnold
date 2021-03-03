#include "testz80.h"
#include "inkz80core.h"

#include <cstring>


TestZ80::TestZ80() :
	_ram(new BYTE[AddressableMemorySize]),
	_test(nullptr)
{
	wipeMemory();
}

TestZ80::~TestZ80()
{
	delete []_ram;
}


void TestZ80::applyMemoryContentionCompound(int ts, WORD addr)
{
	while(ts)
	{
		applyMemoryContention(1, addr);
		ts--;
	}
	return;
}


void TestZ80::applyMemoryContention(int ts, WORD addr)
{
	if (_test)
	{
		Event e(getElapsedTStates(), Event::Event_MemContend, addr, 0);
		_test->actualEvents.push_back(e);
	}
	burnTS(ts);
	return;
}


BYTE TestZ80::memoryReadRaw(WORD addr)
{
	if (_test)
	{
		Event e(getElapsedTStates(), Event::Event_MemRead, addr, _ram[addr]);
		_test->actualEvents.push_back(e);
	}
	return _ram[addr];
}


BYTE TestZ80::memoryReadX(WORD addr)
{
	return _ram[addr];
}


void TestZ80::memoryWriteRaw(WORD addr, BYTE val)
{
	if (_test)
	{
		Event e(getElapsedTStates(), Event::Event_MemWrite, addr, val);
		_test->actualEvents.push_back(e);
	}
	_ram[addr] = val;
}


void TestZ80::memoryWriteX(WORD addr, BYTE val)
{
	_ram[addr] = val;
	return;
}


//
// The port IO tests are ZX Spectrum specific and they mimic the Fuse test code so that
// the tests pass.
//
void TestZ80::prePortIO(WORD port)
{
	if ((port & 0xc000) == 0x4000)
	{
		Event e(getElapsedTStates(), Event::Event_PortContend, port, 0);
		_test->actualEvents.push_back(e);
	}
	burnTS(1);
}


void TestZ80::postPortIO(WORD port)
{
	if ((port & 1))
	{
		if ((port & 0xc000) == 0x4000)
		{
			for(int i=0; i < 3; i++)
			{
				Event e(getElapsedTStates(), Event::Event_PortContend, port, 0);
				_test->actualEvents.push_back(e);
				burnTS(1);
			}
		}
		else
		{
			burnTS(3);
		}
	}
	else
	{
		Event e(getElapsedTStates(), Event::Event_PortContend, port, 0);
		_test->actualEvents.push_back(e);
		burnTS(3);
	}
}
	

BYTE TestZ80::portRead(WORD port)
{
	// Fuse tests expect the port high byte to be returned as the value read
	BYTE portReadVal = port >> 8;
	if (_test)
	{
		prePortIO(port);
		Event e(getElapsedTStates(), Event::Event_PortRead, port, portReadVal);
		_test->actualEvents.push_back(e);
		postPortIO(port);
	}
	return portReadVal;
}


void TestZ80::portWrite(WORD port, BYTE val)
{
	if (_test)
	{
		prePortIO(port);
		Event e(getElapsedTStates(), Event::Event_PortWrite, port, val);
		_test->actualEvents.push_back(e);
		postPortIO(port);
	}
	return;
}


void TestZ80::wipeMemory(BYTE val /* = 0 */)
{
	memset(_ram, val, AddressableMemorySize);
}

