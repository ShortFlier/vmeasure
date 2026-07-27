#pragma once


/*
*将measure中的测量可视化在imagelabelview中
*/

#include "imagelabelview.h"

#include "calipertool.h"
#include "circlefind.h"
#include "ellipsefind.h"



//绘制卡尺工具结果
void drawCaliperToolView(ImageLabelView* view, const CaliperTool& caliperTool, const QColor& color, int thickness);



//绘制圆检测结果
void drawCircleFindView(ImageLabelView* view, const CircleFind& circleFind, const QColor& color, int thickness);


//绘制椭圆检测结果
void drawEllipseFindView(ImageLabelView* view, const EllipseFind& ellipseFind, const QColor& color, int thickness);


