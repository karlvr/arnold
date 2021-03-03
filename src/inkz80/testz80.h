#ifndef _TESTZ80_H
#define _TESTZ80_H

#include "inkz80.h"
#include "tests.h"

class TestZ80 : public Z80
{
public:
	TestZ80();
	~TestZ80();
	void	setTest(Test *test)	{_test = test;}
	Test *	getTest()			{return _test;}
	BYTE *	getRAM()			{return _ram;}
	void	wipeMemory(BYTE val = 0);

	// Z80
	BYTE	memoryReadRaw(WORD addr);
	void	memoryWriteRaw(WORD addr, BYTE val);
	void	applyMemoryContention(int ts, WORD addr);
	void	applyMemoryContentionCompound(int ts, WORD addr);
	BYTE	portRead(WORD port);
	void	portWrite(WORD port, BYTE val);
	
	// These don't add to the test results
	BYTE	memoryReadX(WORD addr);
	void	memoryWriteX(WORD addr, BYTE val);
		
protected:
	// Prevent copy/assignment for moment
	TestZ80(const TestZ80 &);
	TestZ80 & operator=(const TestZ80 &);

	void	prePortIO(WORD port);
	void	postPortIO(WORD port);

	BYTE *	_ram;
	Test *	_test;
};

#endif
