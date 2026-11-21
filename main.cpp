#include <iostream>

#include "calipertool.h"
#include "circlefind.h"

#include "draw.h"

using namespace std;

#include <QApplication>

#include "imagelabelview.h"

std::string qtcvstring(const char* str){
	return QString(str).toLocal8Bit().toStdString();
}

void CaliperToolTest(){
	

	cv::Mat testMat=cv::imread(qtcvstring(R"(testimg\linetest.bmp)"), cv::IMREAD_COLOR);
	QImage qimg=QImage(testMat.data, testMat.cols, testMat.rows, testMat.step, QImage::Format_BGR888).copy();

	cv::Mat grayMat;
	cv::cvtColor(testMat, grayMat, cv::COLOR_BGR2GRAY);

	//横向测量2卡尺
	CaliperTool::Params params;
	params.polar = CaliperTool::BRIGHT2DARK;
	params.threshold = 20;
	params.projectLen = 20;
	CaliperTool toolx1(params);
	qDebug()<<toolx1.measure(grayMat, cv::Point2d(110, 250), 180, 60).size();
	params.polar = CaliperTool::DARK2BRIGHT;
	params.projectLen = 10;
	CaliperTool toolx2(params);
	qDebug()<<toolx2.measure(grayMat, cv::Point2d(400, 300), cv::Point2d(350, 300)).size();

	//竖向测量2卡尺
	params.polar = CaliperTool::BRIGHT2DARK;
	CaliperTool tooly1(params);
	qDebug()<<tooly1.measure(grayMat, cv::Point2d(250, 110), -90 , 40).size();
	params.polar = CaliperTool::DARK2BRIGHT;
	CaliperTool tooly2(params);
	qDebug()<<tooly2.measure(grayMat, cv::Point2d(250, 400), cv::Point2d(250, 350)).size();

	ImageLabelView* imageLabelView = new ImageLabelView();
	imageLabelView->setWindowTitle("CaliperTool Test");
	imageLabelView->resize(800, 600);
	imageLabelView->show();

	imageLabelView->setImage(qimg);
	drawCaliperToolView(imageLabelView, toolx1, Qt::green, 2);
	drawCaliperToolView(imageLabelView, toolx2, Qt::green, 2);

	drawCaliperToolView(imageLabelView, tooly1, Qt::red, 2);
	drawCaliperToolView(imageLabelView, tooly2, Qt::red, 2);

}

void CircleFindTest(){
	const char* imgPath=R"(testimg\testCircle.bmp)";
	cv::Point2d center(250, 250);
	float radius=120;

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

	CaliperTool::Params params;
	params.polar = CaliperTool::BRIGHT2DARK;
	params.threshold = 20;
	params.projectLen = 5;
	params.outputMode = CaliperTool::BY_SEARCH_DIR;
	circleFind.measure(grayMat, CircleFind::INNER2OUTER, params, 50);

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
	const char* imgPath = R"(testimg\testEllipes.bmp)";
	cv::Point2d approxCenter(250, 250); // placeholder center
	float approxRadius = 145;      // placeholder radius
	int sampleCount = 16;

	cv::Mat srcMat = cv::imread(qtcvstring(imgPath), cv::IMREAD_COLOR);
	if (srcMat.empty()) {
		std::cerr << "Failed to load image: " << imgPath << std::endl;
		return;
	}

	QImage qimg = QImage(srcMat.data, srcMat.cols, srcMat.rows, srcMat.step, QImage::Format_BGR888).copy();

	cv::Mat grayMat;
	cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);

	CaliperTool::Params params;
	params.polar = CaliperTool::BRIGHT2DARK;
	params.threshold = 20;
	params.projectLen = 30;
	EllipseFind firstPass(approxCenter, approxRadius, sampleCount);
	EllipseFind::Ellipse firstResult = firstPass.measure(grayMat, EllipseFind::INNER2OUTER, params, 150);

	cv::Mat firstDraw = srcMat.clone();
	firstPass.drawRes(firstDraw, cv::Scalar(0, 255, 0), 2);
	cv::putText(firstDraw, "Ellipse from approximate circle", cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
	// cv::namedWindow("Ellipse First Pass", cv::WINDOW_NORMAL);
	// cv::imshow("Ellipse First Pass", firstDraw);

	if (!validEllipse(firstResult)) {
		std::cerr << "First ellipse fit failed or produced invalid result." << std::endl;
		return;
	}

	ImageLabelView* imageLabelView = new ImageLabelView();
	imageLabelView->setWindowTitle("Ellipse Find Test");
	imageLabelView->resize(800, 600);
	imageLabelView->setImage(qimg);
	drawEllipseFindView(imageLabelView, firstPass, Qt::blue, 2);
	imageLabelView->show();
}


int main()
{
	QApplication a(__argc, __argv);

	CaliperToolTest();
	CircleFindTest();
	EllipseFindTest();


	return a.exec();
}

