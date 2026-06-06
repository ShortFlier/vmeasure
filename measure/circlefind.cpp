#include "circlefind.h"
#include "circlefind.h"
#include "circlefind.h"

#include "log.h"

//返回圆上指定角度的一个点坐标
//角度制，逆时针为正
cv::Point2d circlePoint(cv::Point2d center, float radius, float angle){
    double rad = angle * CV_PI / 180.0;
    return cv::Point2d(
        center.x + radius * std::cos(rad),
        center.y + radius * std::sin(rad)
    );
}

//最小二乘法拟合圆
CircleFind::Circle fit_circle_least_squares(std::vector<cv::Point2d>& points){
    CircleFind::Circle result;
    if (points.size() < 3) {
        result.clear();
        log_warn("最小二乘法拟合圆传入点个数小于3，当前： "+std::to_string(points.size()));
        return result;
    }

    const int n = static_cast<int>(points.size());
    cv::Mat A(n, 3, CV_64F);
    cv::Mat b(n, 1, CV_64F);

    for (int i = 0; i < n; ++i) {
        double x = points[i].x;
        double y = points[i].y;
        A.at<double>(i, 0) = x;
        A.at<double>(i, 1) = y;
        A.at<double>(i, 2) = 1.0;
        b.at<double>(i, 0) = -(x * x + y * y);
    }

    cv::Mat params;
    if (!cv::solve(A, b, params, cv::DECOMP_SVD) || params.rows != 3 || params.cols != 1) {
        result.clear();
        log_warn("最小二乘拟合圆 (!cv::solve(A, b, params, cv::DECOMP_SVD) || params.rows != 3 || params.cols != 1)==true");
        return result;
    }

    double a = params.at<double>(0, 0);
    double c = params.at<double>(1, 0);
    double d = params.at<double>(2, 0);

    double centerX = -a * 0.5;
    double centerY = -c * 0.5;
    double radius2 = centerX * centerX + centerY * centerY - d;
    if (radius2 <= 0 || !std::isfinite(radius2)) {
        log_warn("最小二乘拟合圆 (radius2 <= 0 || !std::isfinite(radius2))==true");
        result.clear();
        return result;
    }

    result.center = cv::Point2d(centerX, centerY);
    result.radius = static_cast<float>(std::sqrt(radius2));
    return result;
}

CircleFind::CircleFind(const cv::Point2d center, float radius, int num)
:center(center), radius(radius), num(num){

}


void CircleFind::setCircle(Circle c)
{
    center = c.center;
    radius = c.radius;
}

CircleFind::SearchDir CircleFind::searchDirInt(int i)
{
    if (i == 0) {
        return OUTER2INNER;
    }
    else
    {
        return INNER2OUTER;
    }
}

CircleFind::Circle CircleFind::measure(const cv::Mat& mat, int idir, int polar, int edgeLength, int threshold, int projectLen, int searchLen){
    _res.clear();
    if(mat.empty()){
        log_error("CircleFind::measure 传入图片为空!");
        return _res;
    }
    if(mat.type()!=CV_8UC1){
        log_error("CircleFind::measure 需要传入8位单通道图像!");
        return _res;
    }

    SearchDir dir = searchDirInt(idir);

    //准备卡尺
    _calipers.clear();
    CaliperTool::Polar cpolar = CaliperTool::polarInt(polar);
    _calipers=std::vector<CaliperTool>(num, CaliperTool(cpolar, edgeLength, threshold, 1));

    std::vector<cv::Point2d> caliperCenters(num);
    std::vector<float> caliperAngles(num);
    for(int i=0; i<num; ++i){
        float angle=i*360.0/num;
        //方向上的圆周点
        caliperCenters[i]=circlePoint(center, radius, angle);
        //由外向内时反向
        if(dir==OUTER2INNER){    
            angle+=180;
        }
        caliperAngles[i]=angle;
    }

    //找点
    _res.points=std::vector<cv::Point2d>(num);
    for(int i=0; i<num; ++i){
        auto p=_calipers[i].measure(mat, caliperCenters[i], caliperAngles[i], projectLen, searchLen);
        if(!p.empty()){
            _res.points[i]=p[0];
        }else{
             _res.points[i]=cv::Point2d(-1, -1);
        }
    }

    //拟合圆
    //选择有效点
    std::vector<cv::Point2d> validPoints;
    for(int i=0; i<_res.points.size(); ++i){
        cv::Point2d& p=_res.points[i];
        if(p.x>0 && p.y>0){
            validPoints.push_back(p);
        }
    }

    CircleFind::Circle circle=fit_circle_least_squares(validPoints);
    _res.center=circle.center;
    _res.radius=circle.radius;

    return _res;
}

void CircleFind::drawRes(cv::Mat& inputoutput, cv::Scalar color, int thickness){
    //绘制圆
    cv::Scalar _color(255 - color[0], 255 - color[1], 255 - color[2]);
    cv::circle(inputoutput, center, radius, _color, thickness);

    //绘制卡尺
    //第一个卡尺用原色，后续卡尺用反色
    if(!_calipers.empty()){
        _calipers[0].drawRes(inputoutput, color, thickness);
    }
    for(int i=1; i<_calipers.size(); ++i){
        _calipers[i].drawRes(inputoutput, _color, thickness);
    }

    //绘制拟合圆
    if(_res.center.x>0 && _res.center.y>0){
        cv::circle(inputoutput, _res.center, _res.radius, color, thickness);
        //中心点
        cv::drawMarker(inputoutput, _res.center, color, cv::MARKER_CROSS, thickness*4, thickness);
    }else{
        log_warn("CircleFind::drawRes 无效的结果!");
        return;
    }
}