// Plugins for VCV Rack by iggy.labs

#include <vector>
#define DR_WAV_IMPLEMENTATION
#include "../../lib/dr_wav.h"


namespace Wavetable {

    static const int maxFrameCount = 256;

    struct Table {
        // Wavetable::Table - The lookup table based on a waveform

        // A wavetable has N frames where each frame/cycle contains M samples
        float lookupTable[maxFrameCount][2048] = {{0}};  // Possible optimization for memory here

        // Default to having 256 samples in a frame
        int frameSize = 256;  // Frame (cycle) length in number of samples
        const std::vector<int> frameSizes{ 256, 512, 1024, 2048 };

        int frameCount = maxFrameCount;  // wavetable size, N

        bool loading = false;


        void setFrameSize(int fs) {
            // Only update `frameSize` if the frame size is valid.
            for (int i = 0; i < (int) frameSizes.size(); i++) {
                if (fs == frameSizes[i]) {
                    frameSize = fs;
                }
            }
        }


        void loadWavetable(std::string path) {
            // Load the file
            // std::vector<float> buffer = loadWavFile(path);

            std::vector<float> buffer;
            unsigned int channels;
            unsigned int sampleRate;
            drwav_uint64 totalSampleCount;

            loading = true;

            float* sampleData;
            sampleData = drwav_open_and_read_file_f32(path.c_str(), &channels, &sampleRate, &totalSampleCount);

            if (sampleData != NULL) {
                buffer.clear();

                // Read the first audio channel only
                for (unsigned int i = 0; i < totalSampleCount; i = i + channels) {
                    buffer.push_back(sampleData[i]);
                }

                totalSampleCount = buffer.size();
                drwav_free(sampleData);

                frameCount = totalSampleCount / frameSize;

                // Cut off the wavetable if there are too many samples
                if (frameCount > maxFrameCount) {
                    frameCount = maxFrameCount;
                }

            }

            // Populate 2D lookup table based on 1D sample
            for (int i = 0; i < frameCount; i++) {
                for (int j = 0; j < frameSize; j++) {
                    lookupTable[i][j] = buffer[i * frameSize + j];
                }
            }

            loading = false;
        }


        float getSample(float frameIndex, float n) {
            // To get the sample, we have to interpolate in two dimensions:
            //     1) within the frame itself
            //     2) between frames in the table
            // Terminology is as follows:
            //
            //     table top, frame left |-------------| table top, frame right
            //                           |             |
            //                           |             |
            //                           |             |
            //  table bottom, frame left |-------------| table bottom, frame right

            if (loading) {
                return 0.f;
            }

            // Position in the frame
            int framePosLeft = (int) n;
            int framePosRight;
            if (framePosLeft == frameSize - 1) { 
                // End of frame, so wrap to beginning
                framePosRight = (int) 0;
            } else {
                framePosRight = framePosLeft + 1;
            }
            float framePosFrac = n - (float) framePosLeft;

            // Position in the table
            float tablePos = frameIndex * (frameCount - 1);  // [0..tableSize]
            int tablePosBottom = floor(tablePos);
            int tablePosTop = ceil(tablePos);
            float tablePosFrac = tablePos - tablePosBottom;  // [0..1]

            // Look up nearest samples
            float bottomLeft = lookupTable[tablePosBottom][framePosLeft];
            float bottomRight = lookupTable[tablePosBottom][framePosRight];
            float topLeft = lookupTable[tablePosTop][framePosLeft];
            float topRight = lookupTable[tablePosTop][framePosRight];

            // Interpolate within both of the two above and below frames
            float above = bottomLeft + framePosFrac * (bottomRight - bottomLeft);
            float below = topLeft + framePosFrac * (topRight - topLeft);

            // Interpolate between frames and return
            return below + tablePosFrac * (above - below);
        }
    };


    struct Oscillator {
        // Wavetable::Oscillator - Oscillator using a lookup table
        Table* table;

        float currentIndex = 0.f;
        float stepSize = 0.f;
        
        float phase = 0.f;
        float freq = 0.f;
        
        float prevPitch = 90000.f;


        Oscillator() {
            table = new Table();
        }


        void setPitch(float pitch, float sampleRate) {
            if (pitch != prevPitch) {
                float frequency = dsp::FREQ_C4 * dsp::approxExp2_taylor5(pitch + 30) / 1073741824;

                // https://en.wikibooks.org/wiki/Sound_Synthesis_Theory/Oscillators_and_Wavetables#Using_wavetables
                // S = N * \frac{f}{F_s}
                stepSize = table->frameCount * frequency / sampleRate;
                prevPitch = pitch;
            }
        }


        float getSample(float y) {
            if (table == nullptr) {
                return 0.f;
            } else {
                return table->getSample(y, currentIndex);
            }
        }


        void step() {
            if (table == nullptr) {
                return;
            }

            if (!table->loading) {
                currentIndex += stepSize;
                
                if (currentIndex >= table->frameCount) {
                    // > It is important to note that because the step size is being altered, the read pointer may not
                    // > land exactly on the final table value N, and so it must "wrap around" in the same fashion as the
                    // > functionally generated waveforms in the earlier section. This can be done by subtracting the size
                    // > of the table from the current pointer value if it exceeds N
                    // https://en.wikibooks.org/wiki/Sound_Synthesis_Theory/Oscillators_and_Wavetables#Using_wavetables
                    
                    currentIndex -= table->frameCount;
                }
            }
        }
    };
}
