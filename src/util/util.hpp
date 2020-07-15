#ifndef IGGYLABS_UTIL_HPP
#define IGGYLABS_UTIL_HPP

namespace iggylabs{
    namespace util {
        inline float rescale(float x, float inRangeLow, float inRangeMax, float outRangeMin, float outRangeMax) {
	        return outRangeMin + (x - inRangeLow) / (inRangeMax - inRangeLow) * (outRangeMax - outRangeMin);
        }
    }
}

#endif