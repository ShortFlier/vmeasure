
#include <iostream>

#include "calipertool.h"
#include "circlefind.h"

using namespace std;

void CaliperToolTest(){

	CaliperTool::Polar polar=CaliperTool::DARK2BRIGHT;
	CaliperTool tool(polar, 3);

	// cv::Mat testMat=cv::Mat::zeros(400, 400, CV_8UC3);
	// //绘制一个200x200的白色矩形，边框15像素
	// cv::rectangle(testMat, cv::Point(100, 100), cv::Point(300, 300), cv::Scalar(255, 255, 255), 15);
	cv::Mat testMat=cv::imread(R"(C:\Users\qiang\Desktop\新建文件夹\新建 BMP 图像.bmp)", cv::IMREAD_COLOR);

	cv::Mat resMat=testMat.clone();

	//在矩形边缘上测量
	cv::Point2d from(175, 225);
	cv::Point2d to(225, 175);
	cv::Mat testMatGray;
	cv::cvtColor(testMat, testMatGray, cv::COLOR_BGR2GRAY);
	// cv::imshow("testMatGray", testMatGray);
	auto res = tool.measure(testMatGray, from, to, 30);
	std::cout << "测量结果：" << std::endl;
	for (const auto& pt : res) {
		std::cout << "点坐标: (" << pt.x << ", " << pt.y << ")" << std::endl;
	}


	cv::line(testMat, from, to, cv::Scalar(255, 0, 0), 2);
	cv::namedWindow("testMat", cv::WINDOW_NORMAL);
	cv::imshow("testMat", testMat);


	//绘制测量结果
	tool.drawRes(resMat, cv::Scalar(0, 0, 255), 2);
	cv::namedWindow("CaliperTool Test", cv::WINDOW_NORMAL);
	cv::imshow("CaliperTool Test", resMat);

}

void CircleFindTest(){
	const char* imgPath=R"(C:\Users\qiang\Desktop\新建文件夹\testCircle.bmp)";
	cv::Point2d center(210, 220);
	float radius=110;

	cv::Mat srcMat=cv::imread(imgPath, cv::IMREAD_COLOR);

	cv::Mat mat=srcMat.clone();
	cv::circle(mat, center, radius, cv::Scalar(0, 0, 255), 4);
	cv::circle(mat, center, 2, cv::Scalar(0, 0, 255), 2);
	cv::namedWindow("mat", cv::WINDOW_NORMAL);
	cv::imshow("mat", mat);

	cv::Mat grayMat;
	cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);
	CircleFind circleFind(center, radius, 8);
	std::cout<<"找圆\n";
	circleFind.measure(grayMat, CircleFind::INNER2OUTER, CaliperTool::BRIGHT2DARK, 1, 20, 30, 50);
	std::cout<<"绘制结果\n";
	circleFind.drawRes(srcMat, cv::Scalar(0, 255, 0), 2);
	cv::namedWindow("res", cv::WINDOW_NORMAL);
	cv::imshow("res", srcMat);
}

int main()
{
	//CaliperToolTest();
	CircleFindTest();

	cv::waitKey(0);
	return 0;
}