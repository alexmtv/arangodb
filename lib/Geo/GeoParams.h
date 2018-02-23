////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2017 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Simon Grätzer
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD_GEO_GEO_PARAMS_H
#define ARANGOD_GEO_GEO_PARAMS_H 1

#include <cmath>

#include <s2/s2region_coverer.h>

#include "Geo/ShapeContainer.h"

namespace arangodb {
namespace velocypack {
class Builder;
class Slice;
}  // namespace velocypack
namespace geo {
constexpr double kPi = std::acos(-1);
// Equatorial radius of earth.
// Source: http://nssdc.gsfc.nasa.gov/planetary/factsheet/earthfact.html
// Equatorial radius
// constexpr double kEarthRadiusInMeters = (6378.137 * 1000);
// Volumetric mean radius
constexpr double kEarthRadiusInMeters = (6371.008 * 1000);
constexpr double kMaxDistanceBetweenPoints = kPi * kEarthRadiusInMeters;

enum class FilterType {
  // no filter, only useful on a near query
  NONE,
  // Select documents with geospatial data that are located entirely within a
  // shape.
  // When determining inclusion, we consider the border of a shape to be part of
  // the shape,
  // subject to the precision of floating point numbers.
  CONTAINS,
  // Select documents whose geospatial data intersects with a specified GeoJSON
  // object.
  INTERSECTS
};

/// @brief contains parameters for s2 region coverer
struct RegionCoverParams {
  RegionCoverParams();
  RegionCoverParams(int mC, int wL, int bL)
      : maxNumCoverCells(mC), worstIndexedLevel(wL), bestIndexedLevel(bL) {
    TRI_ASSERT(mC > 0 && wL > 0 && bL > 0);
  }

 public:
  /// @brief read the options from a vpack slice
  void fromVelocyPack(velocypack::Slice const&);

  /// @brief add the options to an opened vpack builder
  void toVelocyPack(velocypack::Builder&) const;

  S2RegionCoverer::Options regionCovererOpts() const;

 public:
  // Should indicate the max number of cells generated by the S2RegionCoverer
  // is treated as a soft limit, only other params are fixed
  int maxNumCoverCells = 8;
  // Least detailed level used in coverings. Value between [0, 30]
  int worstIndexedLevel = 10;
  // Most detailed level used. Value between [0, 30]
  int bestIndexedLevel = 28;
};

struct QueryParams {
  QueryParams() noexcept
      : origin(geo::Coordinate::Invalid()),
        cover(queryMaxCoverCells, queryWorstLevel, queryBestLevel) {}

  /// This query only needs to support points no polygons etc
  // bool onlyPoints = false;

  // ============== Near Query Params =============

  /// @brief Min distance from centroid a result has to be
  double minDistance = 0.0;
  /// @brief is minimum an exclusive bound
  bool minInclusive = false;

  /// entire earth (halfaround in each direction),
  /// may not be larger than half earth circumference or larger
  /// than the bounding cap of the filter region (see _filter)
  double maxDistance = kEarthRadiusInMeters * M_PI;
  bool maxInclusive = false;

  /// @brief results need to be sorted by distance to centroid
  bool sorted = false;
  /// @brief Default order is from closest to farthest
  bool ascending = true;

  /// @brief Centroid from which to sort by distance
  geo::Coordinate origin;

  // ============= Filtered Params ===============

  FilterType filterType = FilterType::NONE;
  geo::ShapeContainer filterShape;

  // parameters to calculate the cover for index
  // lookup intervals
  RegionCoverParams cover;

 public:
  /// minimum distance
  double minDistanceRad() const noexcept;

  /// depending on @{filter} and @{region} uses maxDistance or
  /// maxDistance / kEarthRadius or a bounding circle around
  /// the area in region
  double maxDistanceRad() const noexcept;

  /// some defaults for queries
  static constexpr int queryWorstLevel = 2;
  static constexpr int queryBestLevel = 23;  // about 1m
  static constexpr int queryMaxCoverCells = 20;
};

}  // namespace geo
}  // namespace arangodb

#endif
