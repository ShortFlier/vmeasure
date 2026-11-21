#include "circlefind.h"
#include "log.h"

#include <cmath>

namespace {
double radians(double degreesValue) {
    return degreesValue * CV_PI / 180.0;
}

// 图片坐标系中，角度顺时针为正。
cv::Point2d circlePoint(const cv::Point2d& center, float radius, float angle) {
    const double rad = radians(angle);
    return cv::Point2d(
        center.x + radius * std::cos(rad),
        center.y + radius * std::sin(rad));
}

CircleFind::Circle fitCircleLeastSquares(
    const std::vector<cv::Point2d>& points) {
    CircleFind::Circle result;
    if (points.size() < 3) {
        result.clear();
        log_warn("最小二乘法拟合圆传入点个数小于3，当前：" +
                 std::to_string(points.size()));
        return result;
    }

    const int n = static_cast<int>(points.size());
    cv::Mat A(n, 3, CV_64F);
    cv::Mat b(n, 1, CV_64F);
    for (int i = 0; i < n; ++i) {
        const double x = points[i].x;
        const double y = points[i].y;
        A.at<double>(i, 0) = x;
        A.at<double>(i, 1) = y;
        A.at<double>(i, 2) = 1.0;
        b.at<double>(i, 0) = -(x * x + y * y);
    }

    cv::Mat params;
    if (!cv::solve(A, b, params, cv::DECOMP_SVD) ||
        params.rows != 3 || params.cols != 1) {
        result.clear();
        log_warn("最小二乘拟合圆失败");
        return result;
    }

    const double a = params.at<double>(0, 0);
    const double c = params.at<double>(1, 0);
    const double d = params.at<double>(2, 0);
    const double centerX = -a * 0.5;
    const double centerY = -c * 0.5;
    const double radius2 = centerX * centerX + centerY * centerY - d;
    if (radius2 <= 0 || !std::isfinite(radius2)) {
        result.clear();
        log_warn("最小二乘拟合圆得到无效半径");
        return result;
    }

    result.center = cv::Point2d(centerX, centerY);
    result.radius = static_cast<float>(std::sqrt(radius2));
    result.xRadius = result.radius;
    result.yRadius = result.radius;
    result.angle = 0.0f;
    return result;
}
}

CircleFind::CircleFind(const cv::Point2d center, float radius, int num)
    : EllipseFind(center, radius, num) {
}

void CircleFind::setCircle(Circle c) {
    center = c.center;
    radius = c.radius;
    xRadius = c.radius;
    yRadius = c.radius;
    angle = 0.0f;
}

CircleFind::SearchDir CircleFind::searchDirInt(int i) {
    return i == 0 ? OUTER2INNER : INNER2OUTER;
}

CircleFind::Circle CircleFind::measure(const cv::Mat& mat, int idir,
                                       int polar, int edgeLength,
                                       int threshold, int projectLen,
                                       int searchLen, int outputMode) {
    EllipseFind::Ellipse ellipseResult =
        EllipseFind::measure(mat, idir, polar, edgeLength, threshold,
                             projectLen, searchLen, outputMode);
    _res.clear();
    _res.center = ellipseResult.center;
    _res.radius = ellipseResult.radius;
    _res.xRadius = ellipseResult.xRadius;
    _res.yRadius = ellipseResult.yRadius;
    _res.angle = ellipseResult.angle;
    _res.points = ellipseResult.points;
    return _res;
}

cv::Point2d CircleFind::getCaliperPoint(int i) const {
    const float angleDeg = i * 360.0f / num;
    return circlePoint(center, radius, angleDeg);
}

float CircleFind::getCaliperAngle(int i) const {
    return i * 360.0f / num;
}

EllipseFind::Ellipse CircleFind::fitValidPoints(
    const std::vector<cv::Point2d>& validPoints) {
    EllipseFind::Ellipse result = fitCircleLeastSquares(validPoints);
    result.points = _res.points;
    return result;
}

void CircleFind::drawRes(cv::Mat& inputoutput, cv::Scalar color,
                         int thickness) {
    const cv::Scalar inverseColor(
        255 - color[0], 255 - color[1], 255 - color[2]);
    cv::circle(inputoutput, center, radius, inverseColor, thickness);

    if (!_calipers.empty()) {
        _calipers[0].drawRes(inputoutput, color, thickness);
    }
    for (size_t i = 1; i < _calipers.size(); ++i) {
        _calipers[i].drawRes(inputoutput, inverseColor, thickness);
    }

    if (validCircle(_res)) {
        cv::circle(inputoutput, _res.center, _res.radius, color, thickness);
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
        log_warn("CircleFind::drawRes无效的结果!");
    }
}
