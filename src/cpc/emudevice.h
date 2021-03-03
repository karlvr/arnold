#ifndef __EMU_DEVICE_HEADER__
#define __EMU_DEVICE_HEADER__

#include "cpc.h"

/* This code is used to register/unregister cpc devices at runtime.
e.g. A hardware device to save a game to disk */


/* TODO: Expose h/w registers/status to debugger
*/

#include "cpcglob.h"
#include "memrange.h"


#define MAX_EXPANSION_ROMS 256

typedef BOOL(*SetExpansionRom)(int nIndex, const unsigned char *, const unsigned long);
typedef void(*ClearExpansionRom)(int nIndex);

typedef struct
{
    /* pointer to rom data, this data is allocated; applies to generic rom boards
	where roms are added/removed */
    unsigned char *ExpansionRomData[MAX_EXPANSION_ROMS];

    /* true if rom is active or inactive */
    BOOL ExpansionRomActive[MAX_EXPANSION_ROMS];
    /* true if rom is available, i.e. device has this rom slot */
    BOOL ExpansionRomAvailable[MAX_EXPANSION_ROMS];

	SetExpansionRom m_SetExpansionRom; /* function for setting rom - this applies to flash rom or static rom */
	ClearExpansionRom m_ClearExpansionRom;/* function for clearing rom - this applies to flash rom or static rom */

} ExpansionRomData;

/* fix the max at 512KB because this is the max that we will write to the snapshot */
#define MAX_DKRAM_PAGES 512/16

typedef void(*RestoreFromSnapshot)(int Config);

typedef struct
{
	/* pointer to ram data, this data is allocated */
	unsigned char *Pages[MAX_DKRAM_PAGES];
	/* true if ram is available, i.e. device has this ram slot */
	BOOL PageAvailable[MAX_DKRAM_PAGES];
} DkRamData;


/* TODO: ram initialisation/free and backup if battery protected functions; with memory page registration,
*/

/* functions that ui can use on devices to get status and for setting device state */
const unsigned char *ExpansionRom_Get(ExpansionRomData *, int RomIndex);
const unsigned char *ExpansionRom_GetSafe(ExpansionRomData *, int RomIndex);

BOOL	ExpansionRom_IsActive(ExpansionRomData *, int RomIndex);
void	ExpansionRom_SetActiveState(ExpansionRomData *, int RomIndex, BOOL State);
void ExpansionRom_Remove(ExpansionRomData *, int RomIndex);
void    ExpansionRom_SetAvailableState(ExpansionRomData *pData, int RomIndex, BOOL State);
BOOL	ExpansionRom_IsAvailable(ExpansionRomData *, int RomIndex);
void ExpansionRom_Init(ExpansionRomData *, ClearExpansionRom pClearExpansionRom, SetExpansionRom pSetExpansionRom);
void ExpansionRom_Finish(ExpansionRomData *);

void EmuDevice_CopyRomData(unsigned char *pDest, unsigned long DestSize, const unsigned char *pSrc, unsigned long SrcSize);
void EmuDevice_ClearRomData(unsigned char *pDest, unsigned long DestSize);


/* expansion roms in "slots", these are here so the various eprom board emulations can use them and the UI can set them
in a more easy to understand way */
BOOL ExpansionRom_GetRomName(ExpansionRomData *pData, const int RomIndex, char **);
int    ExpansionRom_SetRomData(ExpansionRomData *pData, const unsigned char *RomData, const unsigned long RomDataSize, const int RomIndex);

typedef void (*DeviceInstall)(void);
typedef void (*DeviceRemove)(void);

/* the connection type; for filtering in the UI and for information */
#define CONNECTION_EXPANSION (1<<0)   /* connected to expansion */
#define CONNECTION_JOYSTICK (1<<1)    /* connected to joystick */
#define CONNECTION_PRINTER (1<<2)	/* connected to printer */
#define CONNECTION_INTERNAL (1<<3)	/* internal modification */

/* used for choosing, especially in snapshots */
#define DEVICE_FLAGS_HAS_EXPANSION_ROMS (1<<0)	/* device has rom slots that can be filled */
#define DEVICE_FLAGS_HAS_DKTRONICS_RAM (1<<1) /* device has dk'tronics compatible ram; for snapshot */
#define DEVICE_FLAGS_HAS_AUDIO (1<<2) /* device has audio */
#define DEVICE_FLAGS_HAS_PASSTHROUGH (1<<3) /* device has passthrough to other devices */
#define DEVICE_FLAGS_HAS_BATTERY_BACKED_RAM (1<<4) /* device ram is saved */
#define DEVICE_WORKING (1<<5)	/* device is working and useable */
#define DEVICE_FLAGS_FROM_SPECIFICATION (1<<6) /* device based on schematics and specifications */
#define DEVICE_FLAGS_TESTED (1<<7) /* device tested against real hardware */


typedef void(*SetSystemRom)(const unsigned char *, const unsigned long);
typedef void(*ClearSystemRom)(void);

/* A ROM on a device */
typedef struct
{
    const char *sRomName;        /* name for gui */
	const char *sRomSaveName;	/* name for saving state */
	SetSystemRom m_SetSystemRom;        /* function to set the rom data (NULL means clear) */
	ClearSystemRom m_ClearSystemRom;	/* function to clear the rom data */
    size_t m_nRomSize;    /* size of the rom required by the device */
    uint32_t m_nCRC;  /* crc of the rom it was tested with */
} EmuDeviceRom;

typedef void (*PressButton)(void);

/* A press button on a device. */
typedef struct
{
    const char *sButtonName;        /* name for gui */
    PressButton m_Press;        /* function when button is pressed */
} EmuDeviceButton;

typedef void(*AllocRam)(void);
typedef void(*FreeRam)(void);

/* used to register functions to initialise and free 
memory for the device, can be ram or rom */
typedef struct
{
	unsigned char *pPtr; /* pointer to the data */
	unsigned long nSize; /* size of the data */
	AllocRam Alloc; /* allocate the data; e.g. device init on emulator start  */
	FreeRam Free; /* free the data; e.g. device shutdown on emulator exit */
} EmuDeviceRam;

typedef unsigned long (*GetStoreMemSize)(void);
typedef void(*StoreMem)(void *pBuffer, unsigned long nLength);
typedef void(*RestoreMem)(void *pData, unsigned long nLength);

/* used to register functions for restoring and storing
battery backed up ram/rom */
typedef struct
{
	GetStoreMemSize GetStoreSize; /* get size */
	StoreMem Store;	/* function for writing it to host */
	RestoreMem Restore; /* function for reading it to host */
} EmuDeviceBackedupMem;

typedef BOOL (*GetSwitchState)(void);
typedef void (*SetSwitchState)(BOOL);

typedef void(*DeviceInit)(void);
typedef void(*DeviceShutdown)(void);
typedef void(*DeviceEnable)(BOOL);

/* a 2 state switch on a device, it's on or off */
typedef struct
{
    const char *sSwitchName;        /* name for gui */
	const char *sSwitchSaveName;	/* name for saving in preferences */
    GetSwitchState m_GetState;      /* get state function for gui */
    SetSwitchState m_SetState;      /* set state function for gui */
} EmuDeviceSwitch;

/* a structure which defines a device; this is an expansion device */
typedef struct
{
	DeviceEnable m_Enable;
	DeviceInit m_Init;
	DeviceShutdown m_Shutdown;
	const char *sId;				/* an id - meant for snapshot, but this WILL change */
	const char *sSaveName; /* name for saving */
	const char *sDisplayName;		/* displayed named (in gui) */
	int m_Connection;    /* where does it connect? gui information */
	int m_nFlags;		/* flags */
  int         m_nPortRead;				/* number of read I/O ports  */
  CPCPortRead *m_pPortRead;			/* the read ports */
  int       m_nPortWrite;				/* number of write I/O ports */
  CPCPortWrite *m_pPortWrite;			/* the ports */
  int         m_nMemoryRead;				/* number of read I/O ports  */
  CPCPortRead *m_pMemoryRead;			/* the read ports */
  int       m_nMemoryWrite;				/* number of write I/O ports */
  CPCPortWrite *m_pMemoryWrite;			/* the ports */
  CPC_RESET_FUNCTION m_pResetFunction;	/* function to execute on reset */
  CPC_MEMORY_RETHINK_FUNCTION m_pMemoryRethink; /* function to handle ram/rom for cpu to see */
  CPC_POWER_FUNCTION m_pPowerFunction;	/* function to execute on power */
	int         m_nSwitches;            /* number of switches, a switch has two states */
	EmuDeviceSwitch *m_pSwitches;     /* list to switches */
	int         m_nButtons;            /* number of press buttons, a press button is pressed and then released */
	EmuDeviceButton *m_pButtons;     /* list to press buttons */
	int         m_nRoms;            /* number of on-board roms in the device (device uses these; normally not selectable using normal rom methods) */
	EmuDeviceRom *m_pRoms;     /* list of on-board roms)  */
	CPC_CURSOR_UPDATE_FUNCTION m_pCursorUpdateFunction; /* if device listens to crtc cursor output; this function handles it*/
	ExpansionRomData *m_ExpansionRoms;      /* generic rom slots for any expansion type ROM (selectable with CPC's expansion roms)*/
	CPC_PRINTER_UPDATE_FUNCTION m_PrinterUpdate; /* if device listens to printer output this function handles it*/
	CPC_JOYSTICK_READ_FUNCTION m_JoystickReadFunction; /* if device provides input to joystick port(s)  this function handles it*/
	int			m_nMemoryRanges;	/* number of ranges to expose to debugger */
	MemoryRange	*m_pMemoryRange; /* ranges exposed to debugger */
	void (*m_AudioUpdate)(void); /* for devices that produce sound use this function; TODO!!!!! */
	CPC_LIGHT_SENSOR_FUNCTION m_LightSensorFunction; /* for devices that contain a light sensor (light gun/light pen) and trigger CRTC's light pen input use this function */
	CPC_RETI_FUNCTION m_RetiFunction;   /* for devices that look for reti from cpu on the bus */
	CPC_ACK_MASKABLE_INTERRUPT_FUNCTION m_AckMaskableInterrupt; /* for when z80 acknowledges maskable interrupt - seen by CTC for example */
	DkRamData	*m_DkRamData;	/* for snapshot and ui, if device has dk'tronics compatible ram; this tells us what pages it exposes and how to access them */
	EmuDeviceRam *m_Ram; /* if device has on-board ram, use this to initialise it */
	EmuDeviceBackedupMem *m_Backup; /* if device has ram/rom/flash use this to store it's state */
	RestoreFromSnapshot m_RestorefromSnapshot;
} EmuDevice ;

/* bits 1,0 are page, bits 3,4,2 are bank */
BOOL EmuDevice_RespondsToDkRamSelection(int nDevice, int nSelection);
unsigned char *EmuDevice_GetDkRamSelection(int nDevice, int nSelection);

#define MAX_DEVICES 256

ExpansionRomData *EmuDevice_GetExpansionRomData(int nDevice);

// number of enabled devices
int EmuDevice_GetNumEnabled(void);
// get the id of the device at the slot
int EmuDevice_GetEnabledDeviceId(int nEnabledIndex);

int EmuDevice_GetDeviceByName(const char *);
void EmuDevice_RestoreFromSnapshot(int nDevice, int Config);

const char *EmuDevice_GetButtonName(int nDevice, int nButton);
int EmuDevice_GetNumButtons(int nButton);
void EmuDevice_PressButton(int nDevice, int nButton);

int EmuDevice_GetNumRoms(int nRom);
const char *EmuDevice_GetRomName(int nDevice, int nRom);
const char *EmuDevice_GetRomSaveName(int nDevice, int nRom);
void EmuDevice_SetRom(int nDevice, int nRom, const unsigned char *pRom, unsigned long RomLength);
void EmuDevice_ClearRom(int nDevice, int nRom);
size_t EmuDevice_GetRomSize(int nDevice, int nRom);
uint32_t EmuDevice_GetRomCRC(int nDevice, int nRom);

/* display name */
const char *EmuDevice_GetSwitchName(int nDevice, int nSwitch);
/* save name */
const char *EmuDevice_GetSwitchSaveName(int nDevice, int nSwitch);

int EmuDevice_GetNumSwitches(int nDevice);
BOOL EmuDevice_GetSwitchState(int nDevice, int nSwitch);
void EmuDevice_SetSwitchState(int nDevice, int nSwitch, BOOL bState);

const char *EmuDevice_GetName(int nDevice);
const char *EmuDevice_GetSaveName(int nDevice);
BOOL EmuDevice_IsEnabled(int nDevice);
BOOL EmuDevice_HasPassthrough(int nDevice);
int EmuDevice_ConnectedTo(int nDevice);
int EmuDevice_GetFlags(int nDevice);
int RegisterDevice(EmuDevice *pDevice);
void UnRegisterDevice(EmuDevice *pDevice);
void UnRegisterAllDevices(void);

void EmuDevice_Enable(int nDevice, BOOL bState);
int EmuDevice_GetNumDevices(void);

#endif
