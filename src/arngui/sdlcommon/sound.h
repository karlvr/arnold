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


class SDLCommon
{
public:
    static bool Init();         // init subsystem
    static void Shutdown();     // shutdown
	static void PauseSound(bool state);

    static bool InitAudio(int freq, int bit, int chan);
    static bool CloseAudio();
    static bool AudioPlaybackPossible(void);

	static int m_nActualFrequency;
	static int m_nActualBitsPerSample;
	static int m_nActualChannels;
	static bool IsResuming();
	static void UpdatePauseTimer(int Cycles);
	static void ResetPauseTimer();
	static void SilenceAudio();
	static void ResumeAudio();

	static unsigned long m_TicksAudioStart; // tick when we started audio (this is when hardware starts to play)
	static unsigned long m_LockBufferTicks; // tick when sdl buffer is locked (this is when emulator starts to write to buffer)
	static bool m_bIsFirstBufferWriteSinceStart; // true for first hardware buffer write
	static bool m_bIsFirstBufferLockSinceStart; // true for first audio lock since audio starts (get an idea how ahead or behind we are against the hardware)
	static bool m_bIsFirstBufferUnLockSinceStart;
protected:

	static bool m_bIsPaused;
	static bool m_bIsResuming;
	static int m_nPauseTimer;

	static bool m_bHasAudio;
	static bool m_bHasOpenedAudio;
};


void sdl_close_audio(void);
void test(void);