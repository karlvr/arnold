/////////////////////////////////////////////////////////////////////////////
// Name:        tests.cpp
// Purpose:     InkZ80 Emulator Core Test using Fuse's Test files
// Author:      Mark Incley
// Created:     30/10/2007
// Copyright:   Mark Incley
// Licence:     Open Source
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstring>
#include <stdlib.h>
#include <assert.h>
#include "testz80.h"

using namespace std;

#define	FUSE_TEST_ENVVAR	"FUSE_TESTS"

enum ErrorLevels
{
	EL_PASSED,
	EL_FAILED,
	EL_ERROR
};


static const char * const eventTypes[] =
{
	"MR",	//Event_MemRead,
	"MW",	//Event_MemWrite,
	"MC",	//Event_MemContend,
	"PR",	//Event_PortRead,
	"PW",	//Event_PortWrite,
	"PC"	//Event_PortContend
};


// Globals
// Hold tests loaded from Fuse's tests.in
vector<Test>	Tests;
unsigned		LineNumber;
unsigned		TestsPassed;
unsigned		TestsFailed;


void DumpEvents(const std::string & text, const std::vector<Event> & events)
{
	cout << "  " << text << endl;

	size_t n=0;
	auto it = events.cbegin();
	while (it != events.cend())
	{
		cout << dec << "  #" << n << ". time: " << (*it).time << ", type: " << eventTypes[(*it).type] << ", address: " << hex << (*it).address << ", data: " << hex << (unsigned)(*it).data << endl;
		++it;
		n++;
	}

	cout << endl;
}


void DumpEvents(TestZ80 & z80)
{
	DumpEvents("Expected Events", z80.getTest()->expectedEvents);
	DumpEvents("Actual Events", z80.getTest()->actualEvents);
}


void Fail(TestZ80 & z80)
{
	Test *t = z80.getTest();
	if (t->passed)
	{
		t->passed = false;
		cout << "Fail on test " << t->name << ":" << endl;
	}
}


void CompareMemory(TestZ80 & z80, WORD addr, BYTE expected)
{
	if (z80.memoryReadX(addr) != expected)
	{
		Fail(z80);
		cout << "  Fail on memory contents at address " << addr << endl;
		cout << "    Actual: " << hex << (unsigned)z80.memoryReadX(addr) << ", expected: " << (unsigned)expected << endl;
	}
}


void CompareReg(TestZ80 & z80, WORD expected, WORD actual, const string & name)
{
	bool isFlags = (name == "AF" || name == "AF'");

	if (expected != actual)
	{
		Fail(z80);
		cout << "  Fail on register " << name << endl;
		cout << "    Actual: " << hex << actual << ", expected: " << expected;
		if (isFlags)
			cout << ", XOR'd difference: " << (actual ^ expected);
		cout << endl;
	}
}


void CompareEvents(TestZ80 & z80, const Event & expected, const Event & actual, size_t testNum)
{
	if (expected.time != actual.time)
	{
		Fail(z80);
		cout << "  Fail on event # " << testNum << " time" << endl;
		cout << "    Actual: " << dec << actual.time << ", expected: " << expected.time << endl;
	}

	if (expected.type != actual.type)
	{
		Fail(z80);
		cout << "  Fail on event # " << testNum << "  type" << endl;
		cout << "    Actual: " << dec << actual.type << ", expected: " << expected.type << endl;
	}

	if (expected.address != actual.address)
	{
		Fail(z80);
		cout << "  Fail on event # " << testNum << "  address" << endl;
		cout << "    Actual: " << hex << actual.address << ", expected: " << expected.address << endl;
	}

	if (expected.data != actual.data)
	{
		Fail(z80);
		cout << "  Fail on event # " << testNum << "  data" << endl;
		cout << "    Actual: " << hex << (unsigned)actual.data << ", expected: " << (unsigned)expected.data << endl;
	}
}


// Check the results
void CheckTestResults(TestZ80 & z80, Test & t)
{
	// Workaround for InkZ80's MEMPTR and accurate CCF/SCF Bit3/5 emulation (neither of which
	// Fuse has at the time of writing), which produces different results for the following tests
	if (t.name == "cb4e" || t.name == "cb5e" || t.name == "cb6e" || t.name == "cb76" || t.name == "37_1" || t.name == "3f")
	{
		WORD af = z80.getRegisterAF();
		af = (af & ~Z80::FLAGS_35) | (t.expectedRegisters.af & Z80::FLAGS_35);
		z80.setRegisterAF(af);
	}

	if (t.TStatesExecuted != t.expectedRegisters.tstates)
	{
		Fail(z80);
		cout << "  Fail on TStates executed" << endl;
		cout << "    Actual: " << dec << t.TStatesExecuted << ", expected: " << t.expectedRegisters.tstates << endl;
	}

	// Test the state of the Z80 against its expected state
	CompareReg(z80, t.expectedRegisters.af, z80.getRegisterAF(), "AF");
	CompareReg(z80, t.expectedRegisters.bc, z80.getRegisterBC(), "BC");
	CompareReg(z80, t.expectedRegisters.de, z80.getRegisterDE(), "DE");
	CompareReg(z80, t.expectedRegisters.hl, z80.getRegisterHL(), "HL");
	CompareReg(z80, t.expectedRegisters.af2, z80.getRegisterAFAlt(), "AF'");
	CompareReg(z80, t.expectedRegisters.bc2, z80.getRegisterBCAlt(), "BC'");
	CompareReg(z80, t.expectedRegisters.de2, z80.getRegisterDEAlt(), "DE'");
	CompareReg(z80, t.expectedRegisters.hl2, z80.getRegisterHLAlt(), "HL'");
	CompareReg(z80, t.expectedRegisters.i, z80.getRegisterI(), "I");
	CompareReg(z80, t.expectedRegisters.r, z80.getRegisterR(), "R");
	CompareReg(z80, t.expectedRegisters.iff1, z80.getIFF1(), "IFF1");
	CompareReg(z80, t.expectedRegisters.iff2, z80.getIFF2(), "IFF2");
	CompareReg(z80, t.expectedRegisters.im, z80.getInterruptMode(), "IM");
	CompareReg(z80, t.expectedRegisters.halted, z80.isHalted(), "HALTED");

	// Test the memory
	auto it = t.expectedMemBlocks.cbegin();
	while (it != t.expectedMemBlocks.cend())
	{
		WORD addr = it->memStart;
		size_t i = 0;
		while (i < it->memory.size())
			CompareMemory(z80, addr++, it->memory[i++]);
		++it;
	}

	// And the events
	// Since we have excluded port and memory contention events from our tests, we should be able to do a 1-2-1 comparison
	if (t.expectedEvents.size() != t.actualEvents.size())
	{
		Fail(z80);
		cout << "  Fail on number of events" << endl;
		cout << "    Actual: " << dec << t.actualEvents.size() << ", expected: " << t.expectedEvents.size() << endl;
	}
	else
	{
		auto ite = t.expectedEvents.cbegin();
		auto ita = t.actualEvents.cbegin();
		size_t n = 0;
		while (ite != t.expectedEvents.cend())
		{
			CompareEvents(z80, *ite, *ita, n);
			++ite;
			++ita;
			n++;
		}
	}

	// If the test failed, dump the
	if (!t.passed)
		DumpEvents(z80);
}


void RunTest(TestZ80 & z80, Test & t)
{
	// Assume test will pass
	t.passed = true;

	// Set the Z80 state
	z80.reset();
	z80.setRegisterAF(t.setupRegisters.af);
	z80.setRegisterBC(t.setupRegisters.bc);
	z80.setRegisterDE(t.setupRegisters.de);
	z80.setRegisterHL(t.setupRegisters.hl);
	z80.setRegisterAFAlt(t.setupRegisters.af2);
	z80.setRegisterBCAlt(t.setupRegisters.bc2);
	z80.setRegisterDEAlt(t.setupRegisters.de2);
	z80.setRegisterHLAlt(t.setupRegisters.hl2);
	z80.setRegisterIX(t.setupRegisters.ix);
	z80.setRegisterIY(t.setupRegisters.iy);
	z80.setRegisterSP(t.setupRegisters.sp);
	z80.setRegisterPC(t.setupRegisters.pc);
	z80.setRegisterI(t.setupRegisters.i);
	z80.setRegisterR(t.setupRegisters.r);
	z80.setIFF1(t.setupRegisters.iff1 ? Z80::FlipFlopSet : Z80::FlipFlopReset);
	z80.setIFF2(t.setupRegisters.iff2 ? Z80::FlipFlopSet : Z80::FlipFlopReset);
	z80.setInterruptMode(static_cast<Z80::InterruptMode>(t.setupRegisters.im));
	z80.setHalted(t.setupRegisters.halted);

	// Set MEMPTR to 0 to ensure tests cb46, cb56, cb66 and cb7e produce
	// the flag bit3 and 5 values that Fuse's results expect.
	z80.setRegisterWZ(0);

	// Set up the memory. Fuse fills the memory with 0xDEADBEEF, so let's do the same...
	BYTE *ram = z80.getRAM();
	for(unsigned i = 0; i < Z80::AddressableMemorySize; i += 4 )
	{
		ram[i]		= 0xDE;
		ram[i+1]	= 0xAD;
		ram[i+2]	= 0xBE;
		ram[i+3]	= 0xEF;
	}

	auto it = t.setupMemBlocks.cbegin();
	while (it != t.setupMemBlocks.cend())
	{
		WORD addr = it->memStart;
		size_t i = 0;
		while (i < it->memory.size())
			ram[addr++] = it->memory[i++];

		++it;
	}

	// Now run!
	z80.setTest(&t);
	t.TStatesExecuted = z80.executeTStates(t.setupRegisters.tstates);
	assert(z80.getExecutionState() == Z80::XSExhaustedTStates);
}


void RunFuseTests()
{
	if (!Tests.empty())
	{
		cout << "Running Fuse tests..." << endl;

		// Set up a Z80 to run the tests on
		TestZ80	z80;

		auto it = Tests.begin();
		while (it != Tests.end())
		{
			Test &t = (*it);
			RunTest(z80, t);
			CheckTestResults(z80, t);
			if (t.passed)
				TestsPassed++;
			else
				TestsFailed++;

			++it;
		}

		cout << endl;
		cout << "Fuse Test Results" << endl;
		cout << "=================" << endl;
		cout << "   Tests Run: " << dec << Tests.size() << endl;
		cout << "Tests Passed: " << TestsPassed << endl;
		cout << "Tests Failed: " << TestsFailed << endl;
		cout << endl;
	}
}


char HexCharToDec(char hexChar)
{
	if (hexChar >= '0' && hexChar <= '9')
		return hexChar - '0';
	hexChar = toupper(hexChar);
	if (hexChar <'A' || hexChar > 'F')
		throw runtime_error("Expected hex digit");
	return hexChar - ('A' - 10);
}


void SkipWhiteSpaces(const string & line, size_t & cursor)
{
	while (cursor < line.length() && line[cursor] == ' ')
		cursor++;
}


WORD HexWord(const string & line, size_t & cursor)
{
	SkipWhiteSpaces(line, cursor);

	WORD val = 0;
	while (cursor < line.length() && line[cursor] != ' ')
		val = (16 * val) + HexCharToDec(line[cursor++]);

	return val;
}


BYTE HexByte(const string & line, size_t & cursor)
{
	WORD val = HexWord(line, cursor);
	if (val > 255)
		throw("Hex BYTE exceeds 255");
	return static_cast<BYTE>(val);
}


void ExtractRegLine1(Registers & r, const string & line)
{
	if (line.length() < 59)
		throw runtime_error("Expected reg line 1 length to be >= 59 characters");

	size_t i = 0;
	r.af = HexWord(line, i);
	r.bc = HexWord(line, i);
	r.de = HexWord(line, i);
	r.hl = HexWord(line, i);
	r.af2 = HexWord(line, i);
	r.bc2 = HexWord(line, i);
	r.de2 = HexWord(line, i);
	r.hl2 = HexWord(line, i);
	r.ix = HexWord(line, i);
	r.iy = HexWord(line, i);
	r.sp = HexWord(line, i);
	r.pc = HexWord(line, i);
}


void ExtractRegLine2(Registers & r, const string & line)
{
	if (line.length() < 15)
		throw runtime_error("Expected reg line 2 length to be >= 15 characters");

	size_t i = 0;
	r.i = HexByte(line, i);
	r.r = HexByte(line, i);
	r.iff1 = HexByte(line, i);
	r.iff2 = HexByte(line, i);
	r.im = HexByte(line, i);
	r.halted = HexByte(line, i) != 0;
	r.tstates = atoi(line.substr(i).c_str());

	if (r.im > 2)
		throw runtime_error("Expected IM value to be <= 2");
	if (r.tstates == 0)
		throw runtime_error("Expected tstates value to be > 0");
}


void ExtractMemBlockLine(vector<MemBlock> & mblist, const string & line)
{
	if (line.length() < 10)
		throw runtime_error("Expected mem block line length to be >= 10 characters");

	MemBlock	mb;
	size_t i = 0;
	mb.memStart = HexWord(line, i);
	SkipWhiteSpaces(line, i);
	while ((i+1) < line.length())
	{
		string bytestr = line.substr(i, 2);
		if (bytestr == "-1")
			break;

		mb.memory.push_back(HexByte(line, i));
		SkipWhiteSpaces(line, i);
	}

	if (mb.memory.empty())
		throw runtime_error("Expected at least 1 memory byte!");

	mblist.push_back(mb);
}


void ExtractEvent(vector<Event> & le, const string & line)
{
	if (line.length() < 13)
		throw runtime_error("Expected event line length to be >= 13 characters");

	Event	e;
	e.time = atoi(line.substr(0, 5).c_str());
	Event::Type t;
	string eventStr = line.substr(6, 2);
	if (eventStr == "MR")
		t = Event::Event_MemRead;
	else if (eventStr == "MW")
		t = Event::Event_MemWrite;
	else if (eventStr == "MC")
		t = Event::Event_MemContend;
	else if (eventStr == "PR")
		t = Event::Event_PortRead;
	else if (eventStr == "PW")
		t = Event::Event_PortWrite;
	else if (eventStr == "PC")
		t = Event::Event_PortContend;
	else
		throw runtime_error("Unknown event type");

	e.type = t;
	size_t i = 9;
	e.address = HexWord(line, i);

	// Byte may be missing for contentions
	if (line.length() >= 16)
		e.data = HexByte(line, i);

	le.push_back(e);
}


Test * FindTestFromName(const string & testName)
{
	auto it = Tests.begin();
	while (it != Tests.end())
	{
		if (testName == it->name)
			return &(*it);

		++it;
	}
	return 0;
}


void LoadTests(const string & filename)
{
	enum State
	{
		loadName,
		loadRegLine1,
		loadRegLine2,
		loadMemBlock
	};

	LineNumber = 0;
	State state = loadName;
	string	line;
	Test	t;

	ifstream fs(filename.c_str());
	while(getline(fs, line, '\n'))
	{
		++LineNumber;
		switch(state)
		{
		case loadName:
			// Ignore blank lines at end of previous test
			if (!line.empty())
			{
				// Clear test ready for recieving new parameters
				t.Clear();
				t.name = line;
				state = loadRegLine1;
			}
			break;
		case loadRegLine1:
			ExtractRegLine1(t.setupRegisters, line);
			state = loadRegLine2;
			break;
		case loadRegLine2:
			ExtractRegLine2(t.setupRegisters, line);
			state = loadMemBlock;
			break;
		case loadMemBlock:
			if (line == "-1")
			{
				// End of test definition, so add it to the list and start again
				Tests.push_back(t);
				state = loadName;
			}
			else
			{
				ExtractMemBlockLine(t.setupMemBlocks, line);
			}
			break;
		}
	}
}


void LoadResults(const string & filename)
{
	enum State
	{
		loadName,
		loadEvent,
		loadRegLine2,
		loadMemBlock
	};

	LineNumber = 0;
	State state = loadName;
	string	line;
	Test	*t = 0;

	ifstream fs(filename.c_str());
	while(getline(fs, line, '\n'))
	{
		++LineNumber;
		switch(state)
		{
		case loadName:
			// Ignore blank lines at end of previous test results
			if (!line.empty())
			{
				// Find the test from the test name
				t = FindTestFromName(line);
				if (!t)
					throw runtime_error("Unable to find test from name");
				state = loadEvent;
			}
			break;
		case loadEvent:
			if (line.empty())
				throw runtime_error("Unexpected empty event line");
			// Is this actually the start of registers?
			if (line[0] != ' ')
			{
				ExtractRegLine1(t->expectedRegisters, line);
				state = loadRegLine2;
			}
			else
			{
				ExtractEvent(t->expectedEvents, line);
			}
			break;
		case loadRegLine2:
			ExtractRegLine2(t->expectedRegisters, line);
			state = loadMemBlock;
			break;
		case loadMemBlock:
			if (line.empty())
			{
				// End of result definition, so add it to the list and start again
				state = loadName;
			}
			else
			{
				ExtractMemBlockLine(t->expectedMemBlocks, line);
			}
			break;
		}
	}
}


void LoadFuseTests()
{
	const char * testsFolder = getenv(FUSE_TEST_ENVVAR);
	if (testsFolder == nullptr)
	{
		cout << "Fuse tests not available: " FUSE_TEST_ENVVAR " environment variable is not set." << endl;
		return;
	}

	string filenameIn = testsFolder;
	filenameIn += "\\tests.in";
	string filenameExpected = testsFolder;
	filenameExpected += "\\tests.expected";

	try
	{
		cout << "Loading Fuse's tests file: " << filenameIn << endl;
		LoadTests(filenameIn);
	}
	catch(exception &e)
	{
		cout << "Error while loading the tests file: " << e.what() << "(line # " << LineNumber << ")" << endl;
		exit(EL_ERROR);
	}

	cout << Tests.size() << " tests successfully loaded." << endl << endl;

	try
	{
		cout << "Loading expected results file: " << filenameExpected << endl;
		LoadResults(filenameExpected);
	}
	catch(exception &e)
	{
		cout << "Error while loading the expected results file: " << e.what()  << "(line # " << LineNumber << ")" << endl;
		exit(EL_ERROR);
	}

	cout << "Expected results successfully loaded." << endl << endl;
}


void RunDIHaltTest()
{
	cout << "Running DI/Halt test...";

	TestZ80 z80;
	z80.wipeMemory();
	z80.reset();				// Put Z80 in DI state
	z80.memoryWriteX(0,0x76);	// Halt
	Z80Breakpoint brk;
	brk.type = BreakpointDIHalt;
	unsigned brId = z80.AddBreakpoint(brk);
	z80.executeTStates(8);
	Z80::ExecutionState reason = z80.getExecutionState();
	if (reason == Z80::XSBreakpointHit && z80.GetBreakpointTypeHit() == BreakpointDIHalt && z80.GetBreakpointIdHit() == brId)
	{
		cout << "passed";
		TestsPassed++;
	}
	else
	{
		cout << "failed";
		TestsFailed++;
	}
	cout << endl;
}


void Run64KEndlessLoopTest()
{
	cout << "Running 64k DD/FD test...";

	TestZ80 z80;
	z80.wipeMemory(0xDD);

	z80.reset();
	z80.executeSingleInstruction();
	Z80::ExecutionState reason = z80.getExecutionState();
	if (reason == Z80::XSInfiniteDDFDLoop)
	{
		cout << "passed";
		TestsPassed++;
	}
	else
	{
		cout << "failed";
		TestsFailed++;
	}
	cout << endl;
}


int main(int argc, char **argv)
{
	cout << "InkZ80 test suite, built with " << Z80::getVersionString() << endl << endl;

	LoadFuseTests();
	
	RunFuseTests();
	RunDIHaltTest();
	Run64KEndlessLoopTest();

	cout << "Press a key to exit...";
	cin.get();

	exit(TestsFailed ? EL_FAILED : EL_PASSED);

	return 0;
}
