// Plugins for VCV Rack by iggy.labs

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

        int cycleLength;
        int numCycles;
        int cycleIndex;
        std::array<std::array<double, MAX_CYCLE_LENGTH>, MAX_CYCLE_COUNT> cycleBuffers;

        bool loading = false;
        bool loaded = false;

        // To create multiple positions for a 2D wavetable voice, 
        // make a single WaveTableOsc for each possible cycle.
        std::vector<WaveTableOsc*> wavetableOscillators;

        Wavetable() {
            wavetableOscillators.push_back(sawOsc());

            lastPath = "";
            cycleLength = MAX_CYCLE_LENGTH;
            numCycles = 1;
            cycleIndex = 0;
            loading = false;
            loaded = false;
        }

        void loadWavetable(std::string path, int cl) {
            // Only update `frameSize` if valid
            for (int i = 0; i < (int) cycleLengths.size(); i++) {
                if (cl == cycleLengths[i]) {
                    cycleLength = cl;
                }
            }

            // Loading the file
            unsigned int channels;
            unsigned int sampleRate;
            drwav_uint64 totalSampleCount;

            loading = true;

            float* sampleData;
            // TODO - check on the file type
            sampleData = drwav_open_and_read_file_f32(path.c_str(), &channels, &sampleRate, &totalSampleCount);

            if (sampleData != NULL) {
                lastPath = path.c_str();
                cycleBuffers = std::array<std::array<double, MAX_CYCLE_LENGTH>, MAX_CYCLE_COUNT>();

                for (unsigned int i = 0; i < totalSampleCount; i = i + channels) {
                    cycleBuffers[i / cycleLength][i % cycleLength] = sampleData[i];
                }
                drwav_free(sampleData);

                // totalSampleCount /= channels;
                int monoSampleCount = totalSampleCount / channels;

                // It is possible that the buffer has fewer samples than the frame size
                if (monoSampleCount < cycleLength) {
                    cycleLength = monoSampleCount;
                    numCycles = 1;
                } else {
                    numCycles = totalSampleCount / cycleLength;
                }

                // Cut off the wavetable if we populate more than the number of allowed cycles
                if (numCycles > MAX_CYCLE_COUNT) {
                    numCycles = MAX_CYCLE_COUNT;
                }

                // BUILD EACH CYCLE'S WAVETABLE NOW
                wavetableOscillators.clear();
                for (int i = 0; i < numCycles; i++) {
                    wavetableOscillators.push_back(waveOsc(cycleBuffers[i].data(), (int) cycleBuffers[i].size()));
                }
            }
            loading = false;
            loaded = true;
        }

        // TODO - Improve performance by only changing some of the oscillators' frequencies?
        // TODO - Morph between wavetable indices by using fractional cycle index
        // Frequency in units of Hertz
        void setFrequency(double freq, double sampleRate) {
            for (int i = 0; i < numCycles; i++) {
                wavetableOscillators[i]->SetFrequency(freq / sampleRate);
            }
        }

        void setPitch(double pitch, double sampleRate) {
            double freq = dsp::FREQ_C4 * powf(2.0f, pitch);
            setFrequency(freq, sampleRate);
        }

        void updatePhase() {
            for (int i = 0; i < numCycles; i++) {
                wavetableOscillators[i]->UpdatePhase();
            }
        }

        // TODO - Morph between wavetable indices by using fractional cycle index
        // Returns the output of the selected cycle's wavetable oscillator
        float getSample(int cycleIndex) {
            return wavetableOscillators[cycleIndex]->GetOutput();
        }
    };
    
} // namespace Wavetable