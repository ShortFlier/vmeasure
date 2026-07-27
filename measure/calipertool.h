#pragma once

#include <opencv2/opencv.hpp>

/*
* 卡尺工具，用于寻找边缘
*/
class CaliperTool {
public:

    //边缘极性
    enum Polar {
        ANY = 0,    //任何极性
        BRIGHT2DARK = 1,    //亮到暗
        DARK2BRIGHT = 2     //暗到亮
    };

    //i=1返回BRIGHT2DARK，i=2返回DARK2BRIGHT,其他情况返回ANY
    static Polar polarInt(int i);

	CaliperTool(Polar polar=ANY, int edgeLength=1, int threshold=0, int maxNum=1);

    /*
    * @param center卡尺中心点
    * @param angle角度制，逆时针方向为正
    * @param projectLen/searchLen 投影长度/搜索长度
    * @param mat输入图像，单通道
    */
    std::vector<cv::Point2d> measure(const cv::Mat& mat, const cv::Point2d& center, int angle, int projectLen, int searchLen);
    /*
    * @param from起始点
    * @param to结束点
    */
    std::vector<cv::Point2d> measure(const cv::Mat& mat, const cv::Point2d& from, const cv::Point2d& to, int projectLen);

    //绘制结果
    void drawRes(cv::Mat& inputoutput, cv::Scalar color, int thickness=1) const;

public:
    Polar polar=ANY;

    //边缘长度
    int edgeLength=1;

    //对比度阈值
    int threshold=0;

    //最大结果数
    int maxNum = 1;

public:

    //投影长度
    int _projectLength;
    //搜索长度
    int _searchLength;
    //起始、结束点
    cv::Point2d _from, _to;

    std::vector<cv::Point2d> _res;



private:

    std::vector<cv::Point2d> measure(const cv::Mat& mat);

    //验证图像合法性
    bool validateAndPrepare(const cv::Mat& mat);
    //采样点集
    void buildSampleGrid(std::vector<std::vector<cv::Point2d>>& points);
    //采样
    void sampleImage(const cv::Mat& mat, const std::vector<std::vector<cv::Point2d>>& points, cv::Mat& samples);
    //进行投影
    void computeProjection(const cv::Mat& samples, cv::Mat& projection);
    //执行滤波
    void computeGradientAndFilter(cv::Mat& projection, cv::Mat& gradient);
    //获取符合要求边缘点
    void selectEdges(const cv::Mat& gradient, std::vector<int>& indices);

};

