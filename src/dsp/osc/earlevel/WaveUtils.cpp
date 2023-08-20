// Credit:
//
//  Created by Nigel Redmon on 2/18/13
//  EarLevel Engineering: earlevel.com
//  Copyright 2013 Nigel Redmon
//
//  For a complete explanation of the wavetable oscillator and code,
//  read the series of articles by the author, starting here:
//  www.earlevel.com/main/2012/05/03/a-wavetable-oscillator—introduction/

#include "WaveUtils.h"
#include "fft.cpp"


// fillTables:
//
// The main function of interest here; call this with a pointer to an new, empty oscillator,
// and the real and imaginary arrays and their length. The function fills the oscillator with
// all wavetables necessary for full-bandwidth operation, based on one table per octave,
// and returns the number of tables.
//
int fillTables(WaveTableOsc* osc, double* freqWaveRe, double* freqWaveIm, int numSamples) {
    int idx;
    
    // zero DC offset and Nyquist
    freqWaveRe[0] = freqWaveIm[0] = 0.0;
    freqWaveRe[numSamples >> 1] = freqWaveIm[numSamples >> 1] = 0.0;
    
    // determine maxHarmonic, the highest non-zero harmonic in the wave
    int maxHarmonic = numSamples >> 1;
    const double minVal = 0.000001; // -120 dB
    while ((fabs(freqWaveRe[maxHarmonic]) + fabs(freqWaveIm[maxHarmonic]) < minVal) && maxHarmonic) --maxHarmonic;

    // calculate topFreq for the initial wavetable
    // maximum non-aliasing playback rate is 1 / (2 * maxHarmonic), but we allow aliasing up to the
    // point where the aliased harmonic would meet the next octave table, which is an additional 1/3
    double topFreq = 2.0 / 3.0 / maxHarmonic;
    
    // for subsquent tables, double topFreq and remove upper half of harmonics
    double *ar = new double [numSamples];
    double *ai = new double [numSamples];
    double scale = 0.0;
    int numTables = 0;
    while (maxHarmonic) {
        // fill the table in with the needed harmonics
        for (idx = 0; idx < numSamples; idx++)
            ar[idx] = ai[idx] = 0.0;
        for (idx = 1; idx <= maxHarmonic; idx++) {
            ar[idx] = freqWaveRe[idx];
            ai[idx] = freqWaveIm[idx];
            ar[numSamples - idx] = freqWaveRe[numSamples - idx];
            ai[numSamples - idx] = freqWaveIm[numSamples - idx];
        }
        
        // make the wavetable
        scale = makeWaveTable(osc, numSamples, ar, ai, scale, topFreq);
        numTables++;

        // prepare for next table
        topFreq *= 2;
        maxHarmonic >>= 1;
    }
    return numTables;
}


// Alternate version that allows you specify to minumum  maximum harmonic coverage.
// minTop: the minimum normalized frequency that all wave tables support
//      ex.: 18000/44100.0 ensures harmonics out to 18k (44.1kHz sample rate) at minimum
// maxTop: the maximum normalized freuqency that all wave tables support
//      ex.: 0.5 give full bandwidth without aliasing; 24000/44100.0 allows a top of 24k, some aliasing
// The function fills the oscillator with all wavetables necessary for full-bandwidth operation,
// based on the criteria, and returns the number of tables.
//
int fillTables2(WaveTableOsc *osc, double *freqWaveRe, double *freqWaveIm, int numSamples, double minTop, double maxTop) {
    // if top not set, assume aliasing is allowed down to minTop
    if (maxTop == 0.0)
        maxTop = 1.0 - minTop;

    // zero DC offset and Nyquist to be safe
    freqWaveRe[0] = freqWaveIm[0] = 0.0;
    freqWaveRe[numSamples >> 1] = freqWaveIm[numSamples >> 1] = 0.0;

    // for subsequent tables, double topFreq and remove upper half of harmonics
    double *ar = new double [numSamples];
    double *ai = new double [numSamples];
    double scale = 0.0;

    unsigned int maxHarmonic = numSamples >> 1; // start with maximum possible harmonic
    int numTables = 0;
    while (maxHarmonic) {
        // find next actual harmonic, and the top frequency it will support
        const double minVal = 0.000001; // -120 dB
        while ((abs(freqWaveRe[maxHarmonic]) + abs(freqWaveIm[maxHarmonic]) < minVal) && maxHarmonic) --maxHarmonic;
        double topFreq = maxTop / maxHarmonic;

        // fill the table in with the needed harmonics
        for (int idx = 0; idx < numSamples; idx++)
            ar[idx] = ai[idx] = 0.0;
        for (unsigned int idx = 1; idx <= maxHarmonic; idx++) {
            ar[idx] = freqWaveRe[idx];
            ai[idx] = freqWaveIm[idx];
            ar[numSamples - idx] = freqWaveRe[numSamples - idx];
            ai[numSamples - idx] = freqWaveIm[numSamples - idx];
        }

        // make the wavetable
        scale = makeWaveTable(osc, numSamples, ar, ai, scale, topFreq);
        numTables++;

        // topFreq is new base frequency, so figure how many harmonics will fit within maxTop
        unsigned int temp = minTop / topFreq + 0.5;  // next table's maximum harmonic
        maxHarmonic = temp >= maxHarmonic ? maxHarmonic - 1 : temp;
    }
    return numTables;
}


// example that builds a sawtooth oscillator via frequency domain
//
WaveTableOsc* sawOsc(void) {
    int tableLen = 2048;    // to give full bandwidth from 20 Hz
    int idx;
    double *freqWaveRe = new double [tableLen];
    double *freqWaveIm = new double [tableLen];
    
    // make a sawtooth
    for (idx = 0; idx < tableLen; idx++) {
        freqWaveIm[idx] = 0.0;
    }
    freqWaveRe[0] = freqWaveRe[tableLen >> 1] = 0.0;
    for (idx = 1; idx < (tableLen >> 1); idx++) {
        freqWaveRe[idx] = 1.0 / idx;                    // sawtooth spectrum
        freqWaveRe[tableLen - idx] = -freqWaveRe[idx];  // mirror
    }
    
    // build a wavetable oscillator
    WaveTableOsc *osc = new WaveTableOsc();
    fillTables(osc, freqWaveRe, freqWaveIm, tableLen);
    
    return osc;
}

// example that creates an oscillator from an arbitrary time domain wave
//
WaveTableOsc* waveOsc(double* waveSamples, int tableLen) {
    int idx;
    // int tableLen = waveSamples.size();
    double* freqWaveRe = new double [tableLen];
    double* freqWaveIm = new double [tableLen];
    
    // take FFT
    for (idx = 0; idx < tableLen; idx++) {
        freqWaveIm[idx] = waveSamples[idx];
        freqWaveRe[idx] = 0.0;
    }
    fft(tableLen, freqWaveRe, freqWaveIm);
    
    // build a wavetable oscillator
    WaveTableOsc* osc = new WaveTableOsc();
    fillTables(osc, freqWaveRe, freqWaveIm, tableLen);
    
    return osc;
}


// if scale is 0, auto-scales
// returns scaling factor (0.0 if failure), and wavetable in ai array
//
float makeWaveTable(WaveTableOsc *osc, int len, double *ar, double *ai, double scale, double topFreq) {
    fft(len, ar, ai);
    
    if (scale == 0.0) {
        // calc normal
        double max = 0;
        for (int idx = 0; idx < len; idx++) {
            double temp = fabs(ai[idx]);
            if (max < temp)
                max = temp;
        }
        scale = 1.0 / max * .999;        
    }
    
    // normalize
    float *wave = new float [len];
    for (int idx = 0; idx < len; idx++)
        wave[idx] = ai[idx] * scale;
        
    if (osc->AddWaveTable(len, wave, topFreq))
        scale = 0.0;
    
    return scale;
}
