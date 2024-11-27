#ifndef FFMPEG_WRAPPER_FRAME_SUPPORT_H
#define FFMPEG_WRAPPER_FRAME_SUPPORT_H

#include <vector>

/**
 * @brief Represents a point in a 2D plane.
 *
 * This struct defines a point in a 2D plane using x and y coordinates.
 */
struct point {
  float x;  ///< The x-coordinate of the point.
  float y;  ///< The y-coordinate of the point.
};

/**
 * @brief Represents a polygonal area.
 *
 * This struct represents an area using a collection of points, which can be used
 * to specify a region for detection or analysis.
 */
struct area {
  std::vector<point> points;  ///< A collection of points that define the area.
};

#endif  // FFMPEG_WRAPPER_FRAME_SUPPORT_H
