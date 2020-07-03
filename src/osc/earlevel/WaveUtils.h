#ifndef WaveUtils_h
#define WaveUtils_h

#include "WaveTableOsc.h"

int fillTables(WaveTableOsc* osc, double* freqWaveRe, double* freqWaveIm, int numSamples);
int fillTables2(WaveTableOsc* osc, double* freqWaveRe, double* freqWaveIm, int numSamples, double minTop = 0.4, double maxTop = 0);
float makeWaveTable(WaveTableOsc* osc, int len, double* ar, double* ai, double scale, double topFreq);

WaveTableOsc* sawOsc(void);
WaveTableOsc* waveOsc(double* waveSamples);

#endif