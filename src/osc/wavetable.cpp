// Plugins for VCV Rack by iggy.labs
#include <math.h>
#include <vector>
#define DR_WAV_IMPLEMENTATION
#include "../../lib/dr_wav.h"
#include "osc/earlevel/WaveUtils.cpp"

#define BASE_FREQUENCY 20    // Starting frequency of the first table, 20Hz
#define MAX_CYCLE_COUNT 256
#define MAX_CYCLE_LENGTH 2048


namespace Wavetable {

    static std::vector<int> cycleLengths { 256, 512, 1024, 2048 };

    struct Wavetable {
        std::string lastPath;
        std::array<std::array<double, MAX_CYCLE_LENGTH>, MAX_CYCLE_COUNT> cycleBuffers;

        int cycleLength;
        int numCycles;

        bool loading = false;
        bool loaded = false;

        std::array<double, 16> phasors;    // phase accumulator
        std::array<double, 16> phaseIncs;  // phase increment, aka normalized frequency

        // To create multiple positions for a 2D wavetable voice, 
        // make a single WaveTableOsc for each possible cycle.
        std::vector<WaveTableOsc*> wavetableOscillators;

        Wavetable() {
            wavetableOscillators.push_back(sawOsc());

            lastPath = "";
            cycleLength = MAX_CYCLE_LENGTH;
            numCycles = 1;
            loading = false;
            loaded = false;

            phasors.fill(0.f);
            phaseIncs.fill(0.f);
        }

        void loadWavetable(std::string path, int cl) {
            // Only update `frameSize` if valid
            for (int i = 0; i < (int) cycleLengths.size(); i++) {
                if (cl == cycleLengths[i]) {
                    this->cycleLength = cl;
                }
            }

            // Loading the file
            unsigned int channels;
            unsigned int sampleRate;
            drwav_uint64 totalSampleCount;

            loading = true;

            float* sampleData;
            sampleData = drwav_open_and_read_file_f32(path.c_str(), &channels, &sampleRate, &totalSampleCount);

            if (sampleData != NULL) {
                lastPath = path.c_str();
                cycleBuffers = std::array<std::array<double, MAX_CYCLE_LENGTH>, MAX_CYCLE_COUNT>();

                int monoSampleCount = totalSampleCount / channels;

                // Scenarios:
                // 1. We have too many samples to fit in (samples/cycle) * (number of cycles)
                // 2. We do not have enough samples to fit in a single cycle

                // Scenario 1: Adjust the number of samples if too high
                if (monoSampleCount > MAX_CYCLE_COUNT * this->cycleLength) {
                    monoSampleCount = MAX_CYCLE_COUNT * this->cycleLength;
                }

                // Scenario 2: Shrink cycle length if needed
                if (monoSampleCount < this->cycleLength) {
                    this->cycleLength = monoSampleCount;
                    this->numCycles = 1;
                } else {
                    this->numCycles = monoSampleCount / this->cycleLength;
                }

                // Scenario 1: Adjust the number of cycles if calculated is too high
                if (this->numCycles > MAX_CYCLE_COUNT) {
                    this->numCycles = MAX_CYCLE_COUNT;
                }
                
                // Now we can fill the buffers with the sample data
                for (int i = 0; i < monoSampleCount; i++) {
                    cycleBuffers[i / this->cycleLength][i % this->cycleLength] = sampleData[i];
                }
                drwav_free(sampleData);
                
                // Keep count of how many real cycles there are. Sometimes a sample is just 
                // all zeros, and the wavetable oscillator can't handle that.
                int nonZeroCycles = 0;

                // BUILD EACH CYCLE'S WAVETABLE NOW
                wavetableOscillators.clear();
                for (int i = 0; i < this->numCycles; i++) {

                    // We have to check every cycle snippet of the larger sample to make
                    // sure it isn't flat. Also, since the cycle lengths can be different,
                    // don't accidentally pad with all zeros. I'm still figuring out C++
                    // so there is almost certainly a more idiomatic way to do wavetable
                    // array allocation.
                    std::vector<double> temp;
                    double sum = 0.f;
                    for (int j = 0; j < this->cycleLength; j++) {
                        temp.push_back(cycleBuffers[i][j]);
                        sum += fabs(cycleBuffers[i][j]);
                    }

                    // For floating point arithmetic, just say that a cycle isn't zero
                    // if it's above a small threshold
                    if (sum > 0.1f) {
                        nonZeroCycles += 1;
                        wavetableOscillators.push_back(waveOsc(temp.data(), (int) temp.size()));
                    }

                    // Make sure we don't just build up the temp vector every iteration...
                    temp.clear();
                }

                // Now we _really_ know the number of cycles, so update accordingly
                this->numCycles = nonZeroCycles;
            }
            loading = false;
            loaded = true;
        }


        float process(int channel, float cycleIndex, double pitch, double sampleRate) {
            // Update phasor
            phasors[channel] += phaseIncs[channel];
            if (phasors[channel] >= 1.0) {
                phasors[channel] = phasors[channel] - 1.0;
            }

            // Set pitch
            double freq = dsp::FREQ_C4 * powf(2.f, pitch);
            phaseIncs[channel] = freq / sampleRate;

            float tablePos = cycleIndex * (this->numCycles - 1);  // [0..tableSize]
            int tablePosBottom = floor(tablePos);
            int tablePosTop = ceil(tablePos);
            float tablePosFrac = tablePos - (float) tablePosBottom;  // [0..1]

            float above = wavetableOscillators[tablePosTop]->GetOut(phasors[channel], freq, sampleRate);
            float below = wavetableOscillators[tablePosBottom]->GetOut(phasors[channel], freq, sampleRate);

            // Linear interpolation
            return below + tablePosFrac * (above - below);
        }
    };
    
} // namespace Wavetable