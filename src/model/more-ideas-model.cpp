#include <bitset>
#include <math.h>
#include <vector>


namespace MoreIdeas {
    struct Binary {
        int integer;
        std::bitset<8> binaryArray;

        Binary(int num) {
            integer = num;
            setNumber(num);
        }

        void setNumber(int num) {
            integer = num;
            binaryArray = std::bitset<8>(num);
        }
    };
        

    struct Model {
        int bit;
        int note;
        int oct = 0;
        int semitone = 0;
        int low = 1;
        int high = 14;
        int scaleIndex = 0;
        Binary *seed = new Binary(0);
        Binary *generation = nullptr;
        Binary *rule = new Binary(0);
        
        enum ScaleEnum {
            IONIAN,
            AEOLIAN,
            DORIAN,
            PHRYGIAN,
            LYDIAN,
            MIXOLYDIAN,
            MAJOR_PENT,
            MINOR_PENT,
            SHANG,
            JIAO,
            ZHI,
            TODI,
            PURVI,
            MARVA,
            BHAIRAV,
            AHIRBHAIRAV,
            CHROMATIC,
            NUM_SCALES
        };

        std::string scaleNames[NUM_SCALES] = {
            "ionian",
            "aeolian",
            "dorian",
            "phrygian",
            "lydian",
            "mixolydian",
            "major pent",
            "minor pent",
            "shang",
            "jiao",
            "zhi",
            "todi",
            "purvi",
            "marva",
            "bhairav",
            "ahirbhairav",
            "chromatic"
        };

        int scales[NUM_SCALES][29] = {
            {0,2,4,5,7,9,11,12,14,16,17,19,21,23,24,26,28,29,31,33,35,36,38,40,41,43,45,47,48},    // Ionian
            {0,2,3,5,7,8,10,12,14,15,17,19,20,22,24,26,27,29,31,32,34,36,38,39,41,43,44,46,48},    // Aeolian
            {0,2,3,5,7,9,10,12,14,15,17,19,21,22,24,26,27,29,31,33,34,36,38,39,41,43,45,46,48},    // Dorian
            {0,1,3,5,7,8,10,12,13,15,17,19,20,22,24,25,27,29,31,32,34,36,37,39,41,43,44,46,48},    // Phrygian
            {0,2,4,6,7,9,11,12,14,16,18,19,21,23,24,26,28,30,31,33,35,36,38,40,42,43,45,47,48},    // Lydian
            {0,2,4,5,7,9,10,12,14,16,17,19,21,22,24,26,28,29,31,33,34,36,38,40,41,43,45,46,48},    // Mixolydian
            {0,3,5,7,10,12,15,17,19,22,24,27,29,31,34,36,39,41,43,46,48,51,53,55,58,60,63,65,67},  // Maj pent
            {0,2,4,7,9,12,14,16,19,21,24,26,28,31,33,36,38,40,43,45,48,50,52,55,57,60,62,64,67},   // Min pent
            {0,2,5,7,10,12,14,17,19,22,24,26,29,31,34,36,38,41,43,46,48,50,53,55,58,60,62,65,67},  // Shang
            {0,3,5,8,10,12,15,17,20,22,24,27,29,32,34,36,39,41,44,46,48,51,53,56,58,60,63,65,68},  // Jiao
            {0,2,5,7,9,12,14,17,19,21,24,26,29,31,33,36,38,41,43,45,48,50,53,55,57,60,62,65,67},   // Zhi
            {0,1,3,6,7,8,11,12,13,15,18,19,20,23,24,25,27,30,31,32,35,36,37,39,42,43,44,47,48},    // Todi
            {0,1,4,6,7,8,11,12,13,16,18,19,20,23,24,25,28,30,31,32,35,36,37,40,42,43,44,47,48},    // Purvi
            {0,1,4,6,7,9,11,12,13,16,18,19,21,23,24,25,28,30,31,33,35,36,37,40,42,43,45,47,48},    // Marva
            {0,1,4,5,7,8,11,12,13,16,17,19,20,23,24,25,28,29,31,32,35,36,37,40,41,43,44,47,48},    // Bhairav
            {0,1,4,5,7,9,10,12,13,16,17,19,21,22,24,25,28,29,31,33,35,36,37,40,41,43,45,47,48},    // Ahirbhairav
            {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28}         // Chromatic
        };

        // Scale seeds to the note pool and range selected
        int scaleSeedsToNotePool(int lo, int hi, int received) {
            float r = received;
            return floor((((r - 1.f) / (255.f)) * ((hi - lo) + lo)));
        }

        void setSeed(int _seed) {
            this->generation = nullptr;
            this->seed->setNumber(_seed);
        }

        void setGeneration(int _generation) {
            this->generation->setNumber(_generation);
        }
    
        void setRule(int _rule) {
            this->rule->setNumber(_rule);
        }

        // could be a lambda
        bool compHelp(int* seedPack, int* neighborhood) {
            return (seedPack[0] == neighborhood[0] && seedPack[1] == neighborhood[1] && seedPack[2] == neighborhood[2]);
        }

        int compare(int* seedPack, int lshift, int mask) {
            int neighborhoods[8][3] = {
                {1, 1, 1},
                {1, 1, 0},
                {1, 0, 1},
                {1, 0, 0},
                {0, 1, 1},
                {0, 1, 0},
                {0, 0, 1},
                {0, 0, 0}
            };
            
            for (int i = 0; i < 8; i++) {
                if (compHelp(seedPack, neighborhoods[i])) {
                    return ((this->rule->binaryArray[7 - i] << lshift) & mask);
                }
            }
            
            // Default
            return ((0 << lshift) & mask);
        }

        // Does the main logic of updating the seeds
        void updateSeeds() {
            int seedPack[8][3];
            if (generation == nullptr) {
                for (int i = 1; i < 7; i++) {
                    seedPack[i][0] = this->seed->binaryArray[8 - i + 1];
                    seedPack[i][1] = this->seed->binaryArray[8 - i];
                    seedPack[i][2] = this->seed->binaryArray[8 - i - 1];
                }
                seedPack[0][0] = this->seed->binaryArray[0];
                seedPack[0][1] = this->seed->binaryArray[7];
                seedPack[0][2] = this->seed->binaryArray[6];
                seedPack[7][0] = this->seed->binaryArray[1];
                seedPack[7][1] = this->seed->binaryArray[0];
                seedPack[7][2] = this->seed->binaryArray[7];
                this->generation = new Binary(0);
            } else {
                for (int i = 1; i < 7; i++) {
                    seedPack[i][0] = this->generation->binaryArray[8 - i + 1];
                    seedPack[i][1] = this->generation->binaryArray[8 - i];
                    seedPack[i][2] = this->generation->binaryArray[8 - i - 1];
                }
                seedPack[0][0] = this->generation->binaryArray[0];
                seedPack[0][1] = this->generation->binaryArray[7];
                seedPack[0][2] = this->generation->binaryArray[6];
                seedPack[7][0] = this->generation->binaryArray[1];
                seedPack[7][1] = this->generation->binaryArray[0];
                seedPack[7][2] = this->generation->binaryArray[7];
            }
            
            int out = 0;

            for (int i = 0; i < 8; i++) {
                out += compare(seedPack[7 - i], i, floor(pow(2, i)));
            }

            setGeneration(out);
        }

        // Main function that steps forward and updates the current notes
        void onTrigger() {
            updateSeeds();

            int noteInd;
            if (generation == nullptr) {
                noteInd = scaleSeedsToNotePool(low, high, this->seed->integer + 1);
            } else {
                noteInd = scaleSeedsToNotePool(low, high, this->generation->integer + 1);
            }

            this->note = 48 + this->oct + this->semitone + this->scales[this->scaleIndex][noteInd];
        }
    };


    struct CA {
        std::vector<std::vector<int>> cells;

        CA(int rule, int seed, int size) {
            this->cells = std::vector<std::vector<int>>();
            std::bitset<8> ruleset = std::bitset<8>(rule);
            std::bitset<8> seedset = std::bitset<8>(seed);

            this->cells.push_back({});
            for (int i = 0; i < size; i++) {

                // Make the middle 8 cells the seed bits
                int leftIndex = size / 2 - 3;
                int rightIndex = size / 2 + 4;
                if (i < leftIndex || i > rightIndex) {
                    this->cells[0].push_back(0);
                } else {
                    this->cells[0].push_back(seedset[i - leftIndex]);
                }

            }

            // Fill out the remaining generations
            for (int i = 1; i < size; i++) {
                this->cells.push_back({});

                // Leftmost wrap
                int left = this->cells[i-1][size];
                int center = this->cells[i-1][0];
                int right = this->cells[i-1][1];
                int index = (int) left * 4 + center * 2 + right;
                this->cells[i].push_back(ruleset[index]);


                // Center cells
                for (int j = 1; j < size - 1; j++) {
                    // Apply the ruleset
                    left = this->cells[i - 1][j - 1];
                    center = this->cells[i - 1][j];
                    right =  this->cells[i - 1][j + 1];
                    index = (int) left * 4 + center * 2 + right;

                    // Add to row of 2D matrix
                    this->cells[i].push_back(ruleset[index]);
                }

                // Rightmost wrap
                left = this->cells[i-1][size - 1];
                center = this->cells[i-1][size];
                right = this->cells[i-1][0];
                index = (int) left * 4 + center * 2 + right;
                this->cells[i].push_back(ruleset[index]);
            }

        }

        std::vector<std::vector<int>> getCells() {
            return cells;
        }

    };


}  // namespace MoreIdeas