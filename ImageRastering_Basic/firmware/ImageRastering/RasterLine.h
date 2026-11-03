#ifndef RASTER_LINE_H
#define RASTER_LINE_H
#include <math.h>

// Local scan-frame lanes are horizontal in both travel directions. This corrects estimated cross-track error, not wheel slip.
inline float rasterLineHeadingError(float y, float targetY,
                                    float laneHeading, float heading) {
  const float crossTrack=(y-targetY)*cosf(laneHeading);
  float offset=-atanf(crossTrack/40.0f);
  const float maxOffset=0.05235988f; // At most 3 degrees towards the lane.
  if(offset>maxOffset) offset=maxOffset;
  if(offset<-maxOffset) offset=-maxOffset;
  return atan2f(sinf(laneHeading+offset-heading),cosf(laneHeading+offset-heading));
}
#endif
