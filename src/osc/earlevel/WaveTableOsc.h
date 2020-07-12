//  WaveTableOsc.h
//
// Modified by Isabel Kaspriskie - July 2020
//
//  Created by Nigel Redmon on 2018-10-05
//  EarLevel Engineering: earlevel.com
//  Copyright 2018 Nigel Redmon
//
//  For a complete explanation of the wavetable oscillator and code,
//  read the series of articles by the author, starting here:
//  www.earlevel.com/main/2012/05/03/a-wavetable-oscillator—introduction/
//
//  This version has optimizations described here:
//  www.earlevel.com/main/2019/04/28/wavetableosc-optimized/
//
//  License:
//
//  This source code is provided as is, without warranty.
//  You may copy and distribute verbatim copies of this document.
//  You may modify and use this source code to create binary code for your own purposes, free or commercial.
//

#ifndef WaveTableOsc_h
#define WaveTableOsc_h

class WaveTableOsc {
public:
    WaveTableOsc(void) {
        for (int idx = 0; idx < numWaveTableSlots; idx++) {
            mWaveTables[idx].topFreq = 0;
            mWaveTables[idx].waveTableLen = 0;
            mWaveTables[idx].waveTable = 0;
        }
    }
    ~WaveTableOsc(void) {
        for (int idx = 0; idx < numWaveTableSlots; idx++) {
            float *temp = mWaveTables[idx].waveTable;
            if (temp != 0)
                delete [] temp;
        }
    }

    float GetOut(double phasor, double freq, double sampleRate) {
        double freqNormal = freq / sampleRate;  // Phase increment

        // Set frequency
        int curWaveTable = 0;
        while ((freqNormal >= mWaveTables[curWaveTable].topFreq) && (curWaveTable < (mNumWaveTables - 1))) {
            ++curWaveTable;
        }

        waveTable *waveTable = &mWaveTables[curWaveTable];

        // Linear interpolation
        if (waveTable->waveTableLen == 0) {
            return 0.f;
        }

        float temp = phasor * waveTable->waveTableLen;
        int intPart = temp;
        float fracPart = temp - intPart;
        float samp0 = waveTable->waveTable[intPart];
        float samp1 = waveTable->waveTable[intPart + 1];

        return samp0 + (samp1 - samp0) * fracPart;
    }


    // AddWaveTable
    //
    // add wavetables in order of lowest frequency to highest
    // topFreq is the highest frequency supported by a wavetable
    // wavetables within an oscillator can be different lengths
    //
    // returns 0 upon success, or the number of wavetables if no more room is available
    //
    int AddWaveTable(int len, float *waveTableIn, double topFreq) {
        if (mNumWaveTables < numWaveTableSlots) {
            float *waveTable = mWaveTables[mNumWaveTables].waveTable = new float[len + 1];
            mWaveTables[mNumWaveTables].waveTableLen = len;
            mWaveTables[mNumWaveTables].topFreq = topFreq;
            ++mNumWaveTables;

            // fill in wave
            for (long idx = 0; idx < len; idx++)
                waveTable[idx] = waveTableIn[idx];
            waveTable[len] = waveTable[0];  // duplicate for interpolation wraparound

            return 0;
        }
        return mNumWaveTables;
    }

protected:
    struct waveTable {
        double topFreq;
        int waveTableLen;
        float *waveTable;
    };

    int mNumWaveTables = 0;     // number of wavetable slots in use
    static constexpr int numWaveTableSlots = 40;    // simplify allocation with reasonable maximum
    waveTable mWaveTables[numWaveTableSlots];
};

#endif