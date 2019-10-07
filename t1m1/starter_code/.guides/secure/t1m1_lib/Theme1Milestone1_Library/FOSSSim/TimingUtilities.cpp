#include "TimingUtilities.h"

namespace timingutils 
{
    
    double seconds()
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        double time = ((double)tv.tv_sec) + ((double)tv.tv_usec)*1.0e-6;
        return time;
    }
    
}

