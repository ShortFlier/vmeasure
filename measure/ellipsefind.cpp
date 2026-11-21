#include "ellipsefind.h"
#include "log.h"

namespace {
double degrees(double radiansValue) {
    return radiansValue * 180.0 / CV_PI;
}

double radians(double degreesValue) {
    return degreesValue * CV_PI / 180.0;
}

cv::Point2d ellipsePoint(const cv::Point2d& center, float xRadius,
                         float yRadius, float angleDeg, double param) {
    const double phi = radians(angleDeg);
    const double cosPhi = std::cos(phi);
    const double sinPhi = std::sin(phi);
    const double xLocal = xRadius * std::cos(param);
    const double yLocal = yRadius * std::sin(param);
    return cv::Point2d(
        center.x + xLocal * cosPhi - yLocal * sinPhi,
        center.y + xLocal * sinPhi + yLocal * cosPhi);
}

cv::Point2d ellipseNormal(float xRadius, float yRadius,
                          float angleDeg, double param) {
    const double phi = radians(angleDeg);
    const double cosPhi = std::cos(phi);
    const double sinPhi = std::sin(phi);
    const double dx = -xRadius * std::sin(param) * cosPhi
                      - yRadius * std::cos(param) * sinPhi;
    const double dy = -xRadius * std::sin(param) * sinPhi
                      + yRadius * std::cos(param) * cosPhi;
    return cv::Point2d(dy, -dx);
}
}

EllipseFind::EllipseFind(const cv::Point2d center, float radius, int num)
    : center(center), xRadius(radius), yRadius(radius), angle(0.0f),
      radius(radius), num(num) {
}

EllipseFind::EllipseFind(const cv::Point2d center, float xRadius,
                         float yRadius, float angle, int num)
    : center(center), xRadius(xRadius), yRadius(yRadius), angle(angle),
      radius((xRadius + yRadius) * 0.5f), num(num) {
}

void EllipseFind::setEllipse(Ellipse e) {
    center = e.center;
    xRadius = e.xRadius;
    yRadius = e.yRadius;
    angle = e.angle;
    radius = (e.xRadius + e.yRadius) * 0.5f;
}

EllipseFind::SearchDir EllipseFind::searchDirInt(int i) {
    return i == 0 ? OUTER2INNER : INNER2OUTER;
}

EllipseFind::Ellipse EllipseFind::measure(const cv::Mat& mat, int idir,
                                           int polar, int edgeLength,
                                           int threshold, int projectLen,
                                           int searchLen, int outputMode) {
    _res.clear();
    if (mat.empty()) {
        log_error("EllipseFind::measure 传入图片为空!");
        return _res;
    }
    if (mat.type() != CV_8UC1) {
        log_error("EllipseFind::measure 需要传入8位单通道图像!");
        return _res;
    }
    if (xRadius <= 0 || yRadius <= 0) {
        log_error("EllipseFind::measure 需要有效的初始椭圆!");
        return _res;
    }

    _dir = searchDirInt(idir);
    _calipers.clear();
    const CaliperTool::Polar cpolar = CaliperTool::polarInt(polar);
    const CaliperTool::OutputMode cmode = CaliperTool::outputModeInt(outputMode);
    _calipers = std::vector<CaliperTool>(
        num, CaliperTool(cpolar, edgeLength, threshold, 1, cmode));

    _res.points = std::vector<cv::Point2d>(num);
    for (int i = 0; i < num; ++i) {
        const cv::Point2d samplePoint = getCaliperPoint(i);
        float caliperAngle = getCaliperAngle(i);
        if (_dir == OUTER2INNER) {
            caliperAngle += 180.0f;
        }

        const auto points = _calipers[i].measure(
            mat, samplePoint, static_cast<int>(std::round(caliperAngle)),
            projectLen, searchLen);
        _res.points[i] = points.empty()
            ? cv::Point2d(-1, -1)
            : points[0];
    }

    std::vector<cv::Point2d> validPoints;
    validPoints.reserve(_res.points.size());
    for (const auto& point : _res.points) {
        if (point.x > 0 && point.y > 0) {
            validPoints.push_back(point);
        }
    }
    _res = fitValidPoints(validPoints);
    return _res;
}

cv::Point2d EllipseFind::getCaliperPoint(int i) const {
    const double param = i * 2.0 * CV_PI / num;
    return ellipsePoint(center, xRadius, yRadius, angle, param);
}

float EllipseFind::getCaliperAngle(int i) const {
    const double param = i * 2.0 * CV_PI / num;
    const cv::Point2d normal =
        ellipseNormal(xRadius, yRadius, angle, param);
    return static_cast<float>(degrees(std::atan2(normal.y, normal.x)));
}

EllipseFind::Ellipse EllipseFind::fitValidPoints(
    const std::vector<cv::Point2d>& validPoints) {
    Ellipse result;
    if (validPoints.size() < 5) {
        log_warn("EllipseFind::measure有效点不足，无法拟合椭圆");
        return result;
    }

    std::vector<cv::Point2f> floatPoints;
    floatPoints.reserve(validPoints.size());
    for (const auto& point : validPoints) {
        floatPoints.emplace_back(static_cast<float>(point.x),
                                 static_cast<float>(point.y));
    }
    try {
        const cv::RotatedRect fitted = cv::fitEllipse(floatPoints);
        result.center = fitted.center;
        result.xRadius = fitted.size.width / 2.0f;
        result.yRadius = fitted.size.height / 2.0f;
        result.angle = fitted.angle;
        result.radius = (result.xRadius + result.yRadius) * 0.5f;
        result.points = _res.points;
    }
    catch (const cv::Exception& e) {
        log_warn(std::string("EllipseFind::measure cv::fitEllipse失败: ") + e.what());
    }
    return result;
}

void EllipseFind::drawRes(cv::Mat& inputoutput, cv::Scalar color, int thickness) {
    const cv::Scalar inverseColor(
        255 - color[0], 255 - color[1], 255 - color[2]);

    if (xRadius > 0 && yRadius > 0) {
        cv::RotatedRect initialGuess(
            center, cv::Size2f(xRadius * 2.0f, yRadius * 2.0f), angle);
        cv::ellipse(inputoutput, initialGuess, inverseColor, thickness);
    }

    if (!_calipers.empty()) {
        _calipers[0].drawRes(inputoutput, color, thickness);
    }
    for (size_t i = 1; i < _calipers.size(); ++i) {
        _calipers[i].drawRes(inputoutput, inverseColor, thickness);
    }

    if (validEllipse(_res)) {
        cv::RotatedRect fitted(
            _res.center, cv::Size2f(_res.xRadius * 2.0f,
                                    _res.yRadius * 2.0f),
            _res.angle);
        cv::ellipse(inputoutput, fitted, color, thickness);
        cv::drawMarker(inputoutput, _res.center, color,
                       cv::MARKER_CROSS, thickness * 4, thickness);
        for (const auto& point : _res.points) {
            if (point.x > 0 && point.y > 0) {
                cv::drawMarker(inputoutput, point, color,
                               cv::MARKER_TILTED_CROSS,
                               thickness * 2, thickness);
            }
        }
    }
    else {
        log_warn("EllipseFind::drawRes无效的结果!");
    }
}
