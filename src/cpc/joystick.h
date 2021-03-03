#ifndef __JOYSTICK_HEADER_INCLUDED__
#define __JOYSTICK_HEADER_INCLUDED__

#include "cpcglob.h"

/* every 2 seconds */
#define MAXAUTOFIRERATE 2*50
/* every 2 frames */
#define MINAUTOFIRERATE 2

#define CPC_JOYSTICK_NUM_AXES 2
#define CPC_JOYSTICK_NUM_BUTTONS 3

/* when a CPC joystick is simulated by keys on the host
this defines what those keys are */
typedef enum
{
    JOYSTICK_SIMULATED_UP_KEYID = 0,
    JOYSTICK_SIMULATED_DOWN_KEYID = 1,
    JOYSTICK_SIMULATED_LEFT_KEYID = 2,
    JOYSTICK_SIMULATED_RIGHT_KEYID = 3,
    JOYSTICK_SIMULATED_FIRE1_KEYID = 4,
    JOYSTICK_SIMULATED_FIRE2_KEYID = 5,
    JOYSTICK_SIMULATED_FIRE3_KEYID = 6,
    JOYSTICK_SIMULATED_LEFT_UP_KEYID = 7,
    JOYSTICK_SIMULATED_RIGHT_UP_KEYID = 8,
    JOYSTICK_SIMULATED_LEFT_DOWN_KEYID = 9,
    JOYSTICK_SIMULATED_RIGHT_DOWN_KEYID = 10,

    JOYSTICK_SIMULATED_KEYID_LAST
} JOYSTICK_SIMULATED_KEYID;


typedef enum
{
    /* assigned by movement */
    JOYSTICK_SIMULATED_UP_MOUSEID = 0,
    JOYSTICK_SIMULATED_DOWN_MOUSEID = 1,
    JOYSTICK_SIMULATED_LEFT_MOUSEID = 2,
    JOYSTICK_SIMULATED_RIGHT_MOUSEID = 3,

    /* assigned to mouse buttons generally */
    JOYSTICK_SIMULATED_FIRE1_MOUSEID = 4,
    JOYSTICK_SIMULATED_FIRE2_MOUSEID = 5,
    JOYSTICK_SIMULATED_FIRE3_MOUSEID = 6,

    JOYSTICK_SIMULATED_MOUSEID_LAST
} JOYSTICK_SIMULATED_MOUSEID;

/* how are the CPC joysticks represented? */
typedef enum
{
	/* joystick type is unknown */
    JOYSTICK_TYPE_UNKNOWN = 0,
	/* real joystick or joypad connected to computer */
	JOYSTICK_TYPE_REAL,
	/* joystick simulated by keyboard */
	JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD,
	/* joystick simulated by mouse */
	JOYSTICK_TYPE_SIMULATED_BY_MOUSE
} JOYSTICK_TYPE;

/* predefines for when joystick is simulated by keyboard */
typedef enum
{
    JOYSTICK_KEYSET_UNKNOWN = 0,
    JOYSTICK_KEYSET_CURSORS,
    JOYSTICK_KEYSET_NUMPAD,
    JOYSTICK_KEYSET_INSERT_HOME,
	JOYSTICK_KEYSET_CUSTOM,
	JOYSTICK_KEYSET_WASD
} JOYSTICK_KEYSET;

unsigned char Joystick_AdjustValueBasedOnHardware(int nJoystick, unsigned char Data);


void Joystick_SetSimulatedKeyID(int nID, int nKeyID, int nKeyCode);
int Joystick_GetSimulatedKeyID(int nID, int nKeyID);

void Joystick_SetSimulatedMouseID(int nID, int nKeyID, int nKeyCode);
int Joystick_GetSimulatedMouseID(int nID, int nKeyID);

void Joystick_KeyboardLine_Reset(void);
void Joystick_KeyboardLine_FromKeyboard(int nLine, int nBit, BOOL bState);
void Joystick_KeyboardLine_FromJoystick(int nLine, int nBit, BOOL bState);
void Joystick_KeyboardLine_Refresh(void);
unsigned char Joystick_KeyboardLine_Refresh2(int nJoystick, unsigned char KeyboardJoy);
unsigned char Joystick_KeyboardLine_Refresh3( int nJoystick, unsigned char KeyboardJoy, unsigned char JoystickData );

void Joystick_KeyStickActive(BOOL state);
BOOL Joystick_IsKeyStickActive(void);
BOOL	Joystick_IsKeyUsedByKeyStick(int Key);

/* Redefinition Axis; which axis controls X movement and which controls Y movement? */
int Joystick_GetAxisMappingPhysical(int nID,int nAxis);
void Joystick_SetAxisMappingPhysical(int nID,int nAxis, int nValue);

/* assumes 1:1 mapping between pad button and cpc button. We could change the code to support multiple pad
buttons mapped to 1 cpc button, but then it's hard to define that in the ui. It's easier to ask the user to press the button,
because it's not often easy to know the name of a button; what is reported on the actual buttons may not be what the 
code that handles the button reports */

/* map pad button to cpc button */
int Joystick_GetButtonMapping(int nID, int nPadButton);

/* find which pad button is assigned to a cpc button */
int Joystick_GetButtonMappingCPC(int nID, int nCPCPadButton);
void Joystick_SetButtonMappingCPC(int nID,int nCPCButton, int nButton);

/* set the mapping of pad button to cpc button */
void Joystick_SetButtonMapping(int nID, int nPadButton, int nCPCButton);

/* useful for you to set the joystick type */
void Joystick_SetType(int nID, int nType);
int Joystick_GetType(int nID);

/* based on physical joystick id, return CPC joystick ID. applies if joystick type is real */
int Joystick_PhysicalToCPC(int nPhysical);

int Joystick_PhysicalToID(int nPhysical);

int Joystick_GetKeySet(int nID);
void Joystick_SetKeySet(int nID, int nSet);

void Joystick_SetIndexDevice(int nID,short i);

/* applies if joystick type is real */
int Joystick_GetPhysical(int nID);
void Joystick_SetPhysical(int nID, int nPhysical);

void Joystick_SetSimulatedKeyIDState(int nID, int nKeyID, BOOL bState);
void Joystick_SetSimulatedMouseIDState(int nID, int nKeyID, BOOL bState);
void Joystick_Reset(int nID);
void Joystick_SetButton(int nID, int nButton, BOOL bState);
void Joystick_SetXRange(int nID, signed int minx, signed int maxx);
void Joystick_SetYRange(int nID, signed int miny, signed int maxy);
void Joystick_SetXMovement(int nID, signed int curx);
void Joystick_SetYMovement(int nID, signed int cury);

/* initialise default cpc joystick configuration */
void	Joystick_InitDefaultSettings(void);
/* update cpc joysticks from data provided by host */
void	Joystick_Update(void);

void Joystick_Activate(int nID, BOOL bState);
BOOL Joystick_IsActive(int nID);

void JoystickAF_Activate(int nID, BOOL bState);
BOOL JoystickAF_IsActive(int nID);
int JoystickAF_GetRate(int nID);
void JoystickAF_SetRate(int nID, int rate);

short ReturnButtonJoy(short IDjoy,short i);
void SetButtonJoy(short IDjoy,short i,short v);


#endif
