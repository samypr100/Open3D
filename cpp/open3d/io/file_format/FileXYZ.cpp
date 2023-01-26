// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2023 www.open3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include <cstdio>
#include <iostream>

#include "open3d/io/FileFormatIO.h"
#include "open3d/io/PointCloudIO.h"
#include "open3d/utility/FileSystem.h"
#include "open3d/utility/Helper.h"
#include "open3d/utility/Logging.h"
#include "open3d/utility/ProgressReporters.h"

namespace open3d {
namespace io {

FileGeometry ReadFileGeometryTypeXYZ(const std::string &path) {
    return CONTAINS_POINTS;
}

bool ReadPointCloudFromXYZ(const std::string &filename,
                           geometry::PointCloud &pointcloud,
                           const ReadPointCloudOption &params) {
    try {
        utility::filesystem::CFile file;
        if (!file.Open(filename, "r")) {
            utility::LogWarning("Read XYZ failed: unable to open file: {}",
                                filename);
            return false;
        }
        utility::CountingProgressReporter reporter(params.update_progress);
        reporter.SetTotal(file.GetFileSize());

        pointcloud.Clear();
        int i = 0;
        double x, y, z;
        const char *line_buffer;
        while ((line_buffer = file.ReadLine())) {
            if (sscanf(line_buffer, "%lf %lf %lf", &x, &y, &z) == 3) {
                pointcloud.points_.push_back(Eigen::Vector3d(x, y, z));
            }
            if (++i % 1000 == 0) {
                reporter.Update(file.CurPos());
            }
        }
        reporter.Finish();

        return true;
    } catch (const std::exception &e) {
        utility::LogWarning("Read XYZ failed with exception: {}", e.what());
        return false;
    }
}

bool ReadPointCloudInMemoryFromXYZ(const std::vector<char> &bytes,
                                   geometry::PointCloud &pointcloud,
                                   const ReadPointCloudOption &params) {
    try {
        utility::CountingProgressReporter reporter(params.update_progress);
        reporter.SetTotal(static_cast<int64_t>(bytes.size()));
        std::istringstream ibs(std::string(bytes.begin(), bytes.end()));

        pointcloud.Clear();
        int i = 0;
        double x, y, z;
        for (std::string line; std::getline(ibs, line);) {
            if (sscanf(line.c_str(), "%lf %lf %lf", &x, &y, &z) == 3) {
                pointcloud.points_.push_back(Eigen::Vector3d(x, y, z));
            }
            if (++i % 1000 == 0) {
                reporter.Update(ibs.tellg());
            }
        }
        reporter.Finish();

        return true;
    } catch (const std::exception &e) {
        utility::LogWarning("Read XYZ failed with exception: {}", e.what());
        return false;
    }
}

bool WritePointCloudToXYZ(const std::string &filename,
                          const geometry::PointCloud &pointcloud,
                          const WritePointCloudOption &params) {
    try {
        utility::filesystem::CFile file;
        if (!file.Open(filename, "w")) {
            utility::LogWarning("Write XYZ failed: unable to open file: {}",
                                filename);
            return false;
        }
        utility::CountingProgressReporter reporter(params.update_progress);
        reporter.SetTotal(pointcloud.points_.size());

        for (size_t i = 0; i < pointcloud.points_.size(); i++) {
            const Eigen::Vector3d &point = pointcloud.points_[i];
            if (fprintf(file.GetFILE(), "%.10f %.10f %.10f\n", point(0),
                        point(1), point(2)) < 0) {
                utility::LogWarning(
                        "Write XYZ failed: unable to write file: {}", filename);
                return false;  // error happened during writing.
            }
            if (i % 1000 == 0) {
                reporter.Update(i);
            }
        }
        reporter.Finish();
        return true;
    } catch (const std::exception &e) {
        utility::LogWarning("Write XYZ failed with exception: {}", e.what());
        return false;
    }
}

bool WritePointCloudInMemoryToXYZ(std::vector<char> &bytes,
                                  const geometry::PointCloud &pointcloud,
                                  const WritePointCloudOption &params) {
    try {
        utility::CountingProgressReporter reporter(params.update_progress);
        reporter.SetTotal(pointcloud.points_.size());

        bytes.clear();
        for (size_t i = 0; i < pointcloud.points_.size(); i++) {
            const Eigen::Vector3d &point = pointcloud.points_[i];
            std::string line = utility::FormatString(
                    "%.10f %.10f %.10f\n", point(0), point(1), point(2));
            bytes.insert(bytes.end(), line.begin(), line.end());
            if (i % 1000 == 0) {
                reporter.Update(i);
            }
        }
        reporter.Finish();

        return true;
    } catch (const std::exception &e) {
        utility::LogWarning("Write XYZ failed with exception: {}", e.what());
        return false;
    }
}

}  // namespace io
}  // namespace open3d
