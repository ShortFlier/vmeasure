#include "ellipsefind.h"
#include "log.h"

namespace {
    static double degrees(double radians) {
        return radians * 180.0 / CV_PI;
    }

    static double radians(double degrees) {
        return degrees * CV_PI / 180.0;
    }

    static cv::Point2d ellipsePoint(const cv::Point2d& center, float a, float b, float angleDeg, double param) {
        double phi = radians(angleDeg);
        double cosPhi = std::cos(phi);
        double sinPhi = std::sin(phi);
        double xLocal = a * std::cos(param);
        double yLocal = b * std::sin(param);
        return cv::Point2d(
            center.x + xLocal * cosPhi - yLocal * sinPhi,
            center.y + xLocal * sinPhi + yLocal * cosPhi
        );
    }

    static cv::Point2d ellipseNormal(float a, float b, float angleDeg, double param) {
        double phi = radians(angleDeg);
        double cosPhi = std::cos(phi);
        double sinPhi = std::sin(phi);

        double dx = -a * std::sin(param) * cosPhi - b * std::cos(param) * sinPhi;
        double dy = -a * std::sin(param) * sinPhi + b * std::cos(param) * cosPhi;
        return cv::Point2d(dy, -dx);
    }
}

EllipseFind::EllipseFind(const cv::Point2d center, float radius, int num)
    : center(center), semiMajor(radius), semiMinor(radius), angle(0.0f), num(num) {
}

EllipseFind::EllipseFind(const cv::Point2d center, float semiMajor, float semiMinor, float angle, int num)
    : center(center), semiMajor(semiMajor), semiMinor(semiMinor), angle(angle), num(num) {
}

void EllipseFind::setEllipse(Ellipse e) {
    center = e.center;
    semiMajor = e.semiMajor;
    semiMinor = e.semiMinor;
    angle = e.angle;
}

EllipseFind::SearchDir EllipseFind::searchDirInt(int i) {
    return (i == 0) ? OUTER2INNER : INNER2OUTER;
}

EllipseFind::Ellipse EllipseFind::measure(const cv::Mat& mat, int idir, int polar, int edgeLength, int threshold, int projectLen, int searchLen) {
    _res.clear();

    if (mat.empty()) {
        log_error("EllipseFind::measure input image is empty!");
        return _res;
    }
    if (mat.type() != CV_8UC1) {
        log_error("EllipseFind::measure requires 8-bit single-channel image!");
        return _res;
    }
    if (semiMajor <= 0 || semiMinor <= 0) {
        log_error("EllipseFind::measure requires a valid initial ellipse guess!");
        return _res;
    }

    _dir = searchDirInt(idir);
    _calipers.clear();
    CaliperTool::Polar cpolar = CaliperTool::polarInt(polar);
    _calipers = std::vector<CaliperTool>(num, CaliperTool(cpolar, edgeLength, threshold, 1));

    _res.points = std::vector<cv::Point2d>(num);
    for (int i = 0; i < num; ++i) {
        double param = i * 2.0 * CV_PI / num;
        cv::Point2d samplePoint = ellipsePoint(center, semiMajor, semiMinor, angle, param);
        cv::Point2d normal = ellipseNormal(semiMajor, semiMinor, angle, param);

        double normalAngle = degrees(std::atan2(normal.y, normal.x));
        if (_dir == OUTER2INNER) {
            normalAngle += 180.0;
        }

        auto p = _calipers[i].measure(mat, samplePoint, static_cast<int>(std::round(normalAngle)), projectLen, searchLen);
        if (!p.empty()) {
            _res.points[i] = p[0];
        }
        else {
            _res.points[i] = cv::Point2d(-1, -1);
        }
    }

    std::vector<cv::Point2d> validPoints;
    for (const auto& pt : _res.points) {
        if (pt.x > 0 && pt.y > 0) {
            validPoints.push_back(pt);
        }
    }

    if (validPoints.size() >= 5) {
        std::vector<cv::Point2f> floatPoints;
        floatPoints.reserve(validPoints.size());
        for (const auto& pt : validPoints) {
            floatPoints.emplace_back(static_cast<float>(pt.x), static_cast<float>(pt.y));
        }
        try {
            cv::RotatedRect fitted = cv::fitEllipse(floatPoints);
            _res.center = fitted.center;
            _res.semiMajor = fitted.size.width / 2.0f;
            _res.semiMinor = fitted.size.height / 2.0f;
            _res.angle = fitted.angle;
        }
        catch (const cv::Exception& e) {
            log_warn(std::string("EllipseFind::measure cv::fitEllipse failed: ") + e.what());
            _res.clear();
        }
    }
    else {
        log_warn("EllipseFind::measure not enough valid points to fit an ellipse.");
        _res.clear();
    }

    return _res;
}

void EllipseFind::drawRes(cv::Mat& inputoutput, cv::Scalar color, int thickness) {
    cv::Scalar displayColor(255 - color[0], 255 - color[1], 255 - color[2]);

    if (semiMajor > 0 && semiMinor > 0) {
        cv::RotatedRect initialGuess(center, cv::Size2f(semiMajor * 2.0f, semiMinor * 2.0f), angle);
        cv::ellipse(inputoutput, initialGuess, displayColor, thickness);
    }

    for (const auto& caliper : _calipers) {
        caliper.drawRes(inputoutput, displayColor, thickness);
    }

    if (validEllipse(_res)) {
        cv::RotatedRect fitted(_res.center, cv::Size2f(_res.semiMajor * 2.0f, _res.semiMinor * 2.0f), _res.angle);
        cv::ellipse(inputoutput, fitted, color, thickness);
        cv::drawMarker(inputoutput, _res.center, color, cv::MARKER_CROSS, thickness * 4, thickness);
        for (const auto& pt : _res.points) {
            if (pt.x > 0 && pt.y > 0) {
                cv::drawMarker(inputoutput, pt, color, cv::MARKER_TILTED_CROSS, thickness * 2, thickness);
            }
        }
    }
    else {
        log_warn("EllipseFind::drawRes invalid result.");
    }
}
