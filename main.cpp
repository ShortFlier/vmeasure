#include <iostream>

#include "calipertool.h"
#include "circlefind.h"
#include "ellipsefind.h"

using namespace std;

void CaliperToolTest(){

	CaliperTool::Polar polar=CaliperTool::DARK2BRIGHT;
	CaliperTool tool(polar, 3);

	cv::Mat testMat=cv::imread(R"(C:\Users\qiang\Desktop\?????\?? BMP ??.bmp)", cv::IMREAD_COLOR);

	cv::Mat resMat=testMat.clone();

	cv::Point2d from(175, 225);
	cv::Point2d to(225, 175);
	cv::Mat testMatGray;
	cv::cvtColor(testMat, testMatGray, cv::COLOR_BGR2GRAY);
	auto res = tool.measure(testMatGray, from, to, 30);
	std::cout << "????: " << std::endl;
	for (const auto& pt : res) {
		std::cout << "???: (" << pt.x << ", " << pt.y << ")" << std::endl;
	}

	cv::line(testMat, from, to, cv::Scalar(255, 0, 0), 2);
	cv::namedWindow("testMat", cv::WINDOW_NORMAL);
	cv::imshow("testMat", testMat);

	tool.drawRes(resMat, cv::Scalar(0, 0, 255), 2);
	cv::namedWindow("CaliperTool Test", cv::WINDOW_NORMAL);
	cv::imshow("CaliperTool Test", resMat);
}

void CircleFindTest(){
	const char* imgPath=R"(C:\Users\qiang\Desktop\testCircle.bmp)";
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
	std::cout<<"??\n";
	circleFind.measure(grayMat, CircleFind::INNER2OUTER, CaliperTool::BRIGHT2DARK, 1, 20, 30, 50);
	std::cout<<"????\n";
	circleFind.drawRes(srcMat, cv::Scalar(0, 255, 0), 2);
	cv::namedWindow("res", cv::WINDOW_NORMAL);
	cv::imshow("res", srcMat);
}

void EllipseFindTest(){
	const char* imgPath = R"(C:\Users\qiang\Desktop\testEllipes.bmp)";
	cv::Point2d approxCenter(336, 303); // placeholder center
	float approxRadius = 180.0f;      // placeholder radius
	int sampleCount = 16;

	cv::Mat srcMat = cv::imread(imgPath, cv::IMREAD_COLOR);
	if (srcMat.empty()) {
		std::cerr << "Failed to load image: " << imgPath << std::endl;
		return;
	}

	cv::Mat grayMat;
	cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);

	EllipseFind firstPass(approxCenter, approxRadius, sampleCount);
	EllipseFind::Ellipse firstResult = firstPass.measure(grayMat, EllipseFind::INNER2OUTER, CaliperTool::BRIGHT2DARK, 1, 20, 30, 150);

	cv::Mat firstDraw = srcMat.clone();
	firstPass.drawRes(firstDraw, cv::Scalar(0, 255, 0), 2);
	cv::putText(firstDraw, "Ellipse from approximate circle", cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
	cv::namedWindow("Ellipse First Pass", cv::WINDOW_NORMAL);
	cv::imshow("Ellipse First Pass", firstDraw);

	if (!validEllipse(firstResult)) {
		std::cerr << "First ellipse fit failed or produced invalid result." << std::endl;
		return;
	}

	EllipseFind refinedPass(firstResult.center, firstResult.semiMajor, firstResult.semiMinor, firstResult.angle, sampleCount);
	EllipseFind::Ellipse refinedResult = refinedPass.measure(grayMat, EllipseFind::INNER2OUTER, CaliperTool::BRIGHT2DARK, 1, 20, 30, 50);

	cv::Mat refinedDraw = srcMat.clone();
	refinedPass.drawRes(refinedDraw, cv::Scalar(255, 0, 0), 2);
	cv::putText(refinedDraw, "Ellipse refined from first result", cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 0, 0), 2);
	cv::namedWindow("Ellipse Refined Pass", cv::WINDOW_NORMAL);
	cv::imshow("Ellipse Refined Pass", refinedDraw);
	}

int main()
{
	//CaliperToolTest();
	//CircleFindTest();
	EllipseFindTest();

	cv::waitKey(0);
	return 0;
}

