#include <vector>
#define DR_WAV_IMPLEMENTATION
#include "../../lib/dr_wav.h"

#define MAX_FRAME_COUNT 256


struct Wavetable {
    // A wavetable has N frames where each frame/cycle contains M samples
    float lookupTable[MAX_FRAME_COUNT][2048] = {{0}};

    int frameCount;  // TODO

    // Default to having 256 samples in a frame
    int frameSize = 256;
    const std::vector<int> frameSizes{ 256, 512, 1024, 2048 };

    bool loading = false;


    void setFrameSize(int fs) {
        // Only update `frameSize` if the frame size is valid.
        for (int i = 0; i < (int) frameSizes.size(); i++) {
            if (fs == frameSizes[i]) {
                frameSize = fs;
            }
        }
    }


    std::vector<float> loadWavFile(std::string path) {
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

            loading = false;
        }

        return buffer;
    }


    void loadWavetable(std::string path) {
        std::vector<float> buffer = loadWavFile(path);
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
        
        // Position in the frame
        int framePosLeft = (int) n;
        if (framePosLeft == frameSize - 1) { 
            // End of frame, so wrap to beginning
            int framePosRight = (int) 0;
        } else {
            int framePosRight = framePosLeft + 1;
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