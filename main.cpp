#include <iostream>

#include "calipertool.h"
#include "circlefind.h"
#include "ellipsefind.h"

#include "draw.h"

using namespace std;

#include <QApplication>

#include "imagelabelview.h"

std::string qtcvstring(const char* str){
	return QString(str).toLocal8Bit().toStdString();
}

void CaliperToolTest(){

	CaliperTool::Polar polar=CaliperTool::DARK2BRIGHT;
	CaliperTool tool(polar, 3);

	cv::Mat testMat=cv::imread(qtcvstring(R"(C:\Users\qiang\Desktop\竹子\新建 BMP 图像.bmp)"), cv::IMREAD_COLOR);

	QImage qimg=QImage(testMat.data, testMat.cols, testMat.rows, testMat.step, QImage::Format_BGR888).copy();

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
	// cv::namedWindow("testMat", cv::WINDOW_NORMAL);
	// cv::imshow("testMat", testMat);

	tool.drawRes(resMat, cv::Scalar(0, 0, 255), 2);
	// cv::namedWindow("CaliperTool Test", cv::WINDOW_NORMAL);
	// cv::imshow("CaliperTool Test", resMat);

	ImageLabelView* imageLabelView = new ImageLabelView();
	imageLabelView->setWindowTitle("CaliperTool Test");
	imageLabelView->resize(800, 600);
	imageLabelView->setImage(qimg);
	drawCaliperToolView(imageLabelView, tool, Qt::green, 2);
	imageLabelView->show();
}

void CircleFindTest(){
	const char* imgPath=R"(C:\Users\qiang\Desktop\竹子\testCircle.bmp)";
	cv::Point2d center(210, 220);
	float radius=110;

	cv::Mat srcMat=cv::imread(qtcvstring(imgPath), cv::IMREAD_COLOR);
	QImage qimg=QImage(srcMat.data, srcMat.cols, srcMat.rows, srcMat.step, QImage::Format_BGR888).copy();

	cv::Mat mat=srcMat.clone();
	cv::circle(mat, center, radius, cv::Scalar(0, 0, 255), 4);
	cv::circle(mat, center, 2, cv::Scalar(0, 0, 255), 2);
	// cv::namedWindow("mat", cv::WINDOW_NORMAL);
	// cv::imshow("mat", mat);

	cv::Mat grayMat;
	cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);
	CircleFind circleFind(center, radius, 8);
	std::cout<<"??\n";
	circleFind.measure(grayMat, CircleFind::INNER2OUTER, CaliperTool::BRIGHT2DARK, 1, 20, 30, 50);
	std::cout<<"????\n";
	circleFind.drawRes(srcMat, cv::Scalar(0, 255, 0), 2);
	// cv::namedWindow("res", cv::WINDOW_NORMAL);
	// cv::imshow("res", srcMat);

	ImageLabelView* imageLabelView = new ImageLabelView();
	imageLabelView->setWindowTitle("Circle Find Test");
	imageLabelView->resize(800, 600);
	imageLabelView->setImage(qimg);
	drawCircleFindView(imageLabelView, circleFind, Qt::green, 2);
	imageLabelView->show();
}

void EllipseFindTest(){
	const char* imgPath = R"(C:\Users\qiang\Desktop\竹子\testEllipes.bmp)";
	cv::Point2d approxCenter(336, 303); // placeholder center
	float approxRadius = 180.0f;      // placeholder radius
	int sampleCount = 16;

	cv::Mat srcMat = cv::imread(qtcvstring(imgPath), cv::IMREAD_COLOR);
	if (srcMat.empty()) {
		std::cerr << "Failed to load image: " << imgPath << std::endl;
		return;
	}

	QImage qimg = QImage(srcMat.data, srcMat.cols, srcMat.rows, srcMat.step, QImage::Format_BGR888).copy();

	cv::Mat grayMat;
	cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);

	EllipseFind firstPass(approxCenter, approxRadius, sampleCount);
	EllipseFind::Ellipse firstResult = firstPass.measure(grayMat, EllipseFind::INNER2OUTER, CaliperTool::BRIGHT2DARK, 1, 20, 30, 150);

	cv::Mat firstDraw = srcMat.clone();
	firstPass.drawRes(firstDraw, cv::Scalar(0, 255, 0), 2);
	cv::putText(firstDraw, "Ellipse from approximate circle", cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
	// cv::namedWindow("Ellipse First Pass", cv::WINDOW_NORMAL);
	// cv::imshow("Ellipse First Pass", firstDraw);

	if (!validEllipse(firstResult)) {
		std::cerr << "First ellipse fit failed or produced invalid result." << std::endl;
		return;
	}

	EllipseFind refinedPass(firstResult.center, firstResult.xRadius, firstResult.yRadius, firstResult.angle, sampleCount);
	EllipseFind::Ellipse refinedResult = refinedPass.measure(grayMat, EllipseFind::INNER2OUTER, CaliperTool::BRIGHT2DARK, 1, 20, 30, 50);

	cv::Mat refinedDraw = srcMat.clone();
	refinedPass.drawRes(refinedDraw, cv::Scalar(255, 0, 0), 2);
	cv::putText(refinedDraw, "Ellipse refined from first result", cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 0, 0), 2);
	// cv::namedWindow("Ellipse Refined Pass", cv::WINDOW_NORMAL);
	// cv::imshow("Ellipse Refined Pass", refinedDraw);

	ImageLabelView* imageLabelView = new ImageLabelView();
	imageLabelView->setWindowTitle("Ellipse Find Test");
	imageLabelView->resize(800, 600);
	imageLabelView->setImage(qimg);
	drawEllipseFindView(imageLabelView, refinedPass, Qt::blue, 2);
	imageLabelView->show();
}


void testImageLabelRect(){
	ImageLabelView* imageLabelView = new ImageLabelView();
	imageLabelView->setWindowTitle("Image Label View Test");
	imageLabelView->resize(800, 600);
	imageLabelView->show();

	cv::Mat testImage = cv::imread(qtcvstring(R"(C:\Users\qiang\Desktop\竹子\20260606_160050_629_001.jpg)"), cv::IMREAD_COLOR);

	if (testImage.empty()) {
		std::cerr << "Failed to load image." << std::endl;
		return;
	}

	imageLabelView->setImage(QImage(testImage.data, testImage.cols, testImage.rows, testImage.step, QImage::Format_BGR888).copy());

	imageLabelView->addLabel(new ImageRotatedRectLabel(QRect(200, 200, 400, 500), 30, Qt::green, 4));
}

int main()
{
	QApplication a(__argc, __argv);

	//CaliperToolTest();
	//CircleFindTest();
	EllipseFindTest();

	//testImageLabelRect();

	return a.exec();
}

