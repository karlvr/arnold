/* flatscreen TV: type 1
ff reduced border left side
fd larger border left, and shift screen down a little
f9 display goes a bit dull (maybe half brightness?) if colour - set border to black
f8 duller
f6/f5 works


visible area:

R2=50 (left side has ~ half char border
R1=47 (right side has ~ half char border
R7=34 (top border half char)
R6=33 (bottom border half char)

sony trinitron crt  (similar)
ff reduced border left side
fd larger border left, and shift screen down a little
f9 display goes a bit dull (maybe half brightness?) if colour - set border to black
f8 duller
f6/f5 not stable

R2=49 (left side has ~ half char border)
R1=47 (right side has ~ half char border)
R7=34
R6=32

sony flatscreen tv (widescreen)
ff reduced border left side
fd larger border left, and shift screen down a little
f9 display goes a bit dull (maybe half brightness?) if colour - set border to black
f8 duller
f6/f5 not stable

R2=49 (left side has ~ half char border)
R1=47 (right side has ~ half char border)
R7=34
R6=32
*/

#ifndef __MONITOR_HEADER_INCLUDED__
#define __MONITOR_HEADER_INCLUDED__

#include "render.h"
/* between 0 and 100 */
void Monitor_SetBrightness(int);
/* between 0 and 100 */
void Monitor_SetContrast(int);
int Monitor_GetBrightness(void);
int Monitor_GetContrast(void);

void Monitor_DoHsyncStart(void);
void Monitor_DoHsyncEnd(void);
void Monitor_DoVsyncStart(void);
void Monitor_DoVsyncEnd(void);

void Monitor_AdjustRGBForDisplay(RGBCOLOUR *Colour);
int Monitor_AdjustLuminanceForDisplay(int );

void Monitor_Init(void);
void Monitor_Reset(void);

void Monitor_SetRendererReady(BOOL bState);

/* draw MONITOR sync */
void Monitor_EnableDrawSync(BOOL bEnableDrawSync);
BOOL Monitor_GetDrawSync(void);

BOOL Monitor_GetVsyncState(void);
BOOL Monitor_GetHsyncState(void);

BOOL Monitor_DrawSync(void);

/* 
visible area, unknown what actual frame height is
R2 max = 50
R6 max = 34 -> 34*8 = 272
R7 max = 35
R1 max = 48

34*8 = 
*/

#define MONITOR_HORIZONTAL_RATE 64
#define MONITOR_VERTICAL_RATE 312	/* change to cycles */
///#define MONITOR_VERTICAL_RATE 248	// vhold min arnold &f7 rather than &f8
//#define MONITOR_VERTICAL_RATE 427 // arnold &1aa rather than &1ab

#define MONITOR_WIDTH_CHARS 52

#define HHOLD_RANGE 4
#define MONITOR_H_MIN (MONITOR_HORIZONTAL_RATE-HHOLD_RANGE)
#define MONITOR_H_MAX (MONITOR_HORIZONTAL_RATE+HHOLD_RANGE)

#define VHOLD_RANGE 4
#define MONITOR_V_MIN (MONITOR_VERTICAL_RATE-VHOLD_RANGE)
#define MONITOR_V_MAX (MONITOR_VERTICAL_RATE+VHOLD_RANGE)

#define MONITOR_HTRACE_COUNT MONITOR_HORIZONTAL_RATE-MONITOR_WIDTH_CHARS
//#define MONITOR_HEIGHT_LINES 288
#define MONITOR_HEIGHT_LINES MONITOR_VERTICAL_RATE-24
#define MONITOR_VTRACE_COUNT 24*MONITOR_HORIZONTAL_RATE

typedef struct
{
	/* TRUE to draw monitor SYNC, false otherwise */
	BOOL bDrawSyncs;

	/* the sync inputs to the monitor; vertical and horizontal sync */
	unsigned char MonitorSyncInputs;

	/* active draw position */
	int     MonitorScanLineCount;
	int     MonitorHorizontalCount;

	int 		MonitorVCount;
	int		MonitorHCount;
	
	/* if monitor is actively vtracing and for how many scanlines */
	/* lines or cycles?? */
	BOOL	MonitorVTraceActive;
	int		MonitorVTraceCount;

	/* if monitor is actively htracing and for how many scanlines */
	BOOL	MonitorHTraceActive;
	int		MonitorHTraceCount;

	int		MonitorDestH;
	int		MonitorDestV;
	
	/* number of scan-lines since end of VSYNC */
	int LinesAfterVTrace;
//	int CharsAfterVSync;

	/* number of chars since end of hsync */
//	int CharsAfterHSync;
	int CharsAfterHTrace;

} MONITOR_INTERNAL_STATE;



typedef struct
{
	float R;
	float G;
	float B;
} RGB_FLOAT;


typedef struct
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char pad0;
} RGB_CHAR;


void	Monitor_Cycle(void);

void CRTC_SetRenderFunction2(void(*pRenderFunction)(void));


#endif



















#if 0


#ifndef __MONITOR_HEADER_INCLUDED__
#define __MONITOR_HEADER_INCLUDED__

void Monitor_Init(void);
void Monitor_Reset(void);

void Monitor_DoHsyncStart(void);
void Monitor_DoHsyncEnd(void);
void Monitor_DoVsyncStart(void);
void Monitor_DoVsyncEnd(void);
void    CRTC_SetRenderFunction(int RenderMode);
void CRTC_SetRenderFunction2(void (*pRenderFunction)(void));
void	Render_SetRenderFunction(int RenderMode);
void	Render_SetRenderFunctionPLUS(unsigned long SpriteMask);
void	Render_SetRenderState(BOOL State);
void	Render_SetTrueColourRender(BOOL State);
void Graphics_Update(void);

#define RENDER_MODE_STANDARD		0x001
#define RENDER_MODE_ASIC_FEATURES	0x002

typedef enum
{
	CRTC_RENDER_GRAPHICS = 1,
	CRTC_RENDER_BORDER,
	CRTC_RENDER_SYNC
} CRTC_RENDER_TYPE;


#if 0
#define CRTC_RENDER_STAGE_START_NEW_LINE 0x0001
#define CRTC_RENDER_STAGE_START_NEW_FRAME 0x0002
typedef struct
{
	unsigned long MA;
	unsigned long Count;
	unsigned long RenderType;
	unsigned long Flags;
} CRTC_RENDER_STAGE;
#endif
/*
static void     CRTC_SetRenderingFunction(void);
*/

BOOL Monitor_GetVsyncState(void);
BOOL Monitor_GetHBlank(void);
BOOL Monitor_GetVBlank(void);
BOOL Monitor_GetHsyncState(void);
int Monitor_GetHorizontalPosition(void);
int Monitor_GetVerticalPosition(void);
int Monitor_GetHorizontalRetraceCount(void);
int Monitor_GetVerticalRetraceCount(void);
BOOL Monitor_SeeHsync(void);
int Monitor_CountAfterHsync(void);
int Monitor_VertStartAdjustment(void);
int Monitor_HorzStartAdjustment(void);
BOOL Monitor_SeeVsync(void);
int Monitor_CountAfterVsync(void);
BOOL Monitor_DrawSync(void);

typedef struct
{
	unsigned char MonitorFlags;
    int     MonitorHSyncCount;
    int     MonitorVSyncCount;
    int     MonitorHorizontalCount;
    int     MonitorScanLineCount;
BOOL bHorizontalCounterActive;

    BOOL    bDrawSync;
    BOOL    bSeenHorzInputSync;
    int     HorzSyncAfterCount;
    int     HorzStartAdjustment;
BOOL bMonitorTookVsync;
  BOOL bVerticalLineCounterActive;

    BOOL    bSeenVertInputSync;
    int     VertStartAdjustment;
    int     VertSyncAfterCount;


	unsigned char PrevMonitorSyncInputs;
	unsigned char MonitorSyncInputs;
	BOOL bDispEnable;

    /* vhold/hhold */
	unsigned int VHoldControl;
	unsigned int HHoldControl;

	/* brightness */
	unsigned int BrightnessControl;

#if 0

	/* number of scan-lines after start of VSYNC */
	int LinesAfterVsync;
	/* number of chars after start of HSYNC */
	int CharsAfterHsync;
	int				MonitorScanLineCount;

	int				VSyncRetraceCount;
	int				HSyncRetraceCount;

	/* these are used to render black on the screen, if a sync
	falls within screen area */
	unsigned char MonitorVsyncCount;
#endif
#if 0
	/* "monitor" counts */
	unsigned char MonitorFlags;
	unsigned char pad;
	unsigned char MonitorHorizontalCount;
#endif

	int Pixels[16];
} MONITOR_INTERNAL_STATE;


typedef struct
{
        float R;
        float G;
        float B;
} RGB_FLOAT;


typedef struct
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char pad0;
} RGB_CHAR;

#endif

#endif

