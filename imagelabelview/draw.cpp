
#include "draw.h"

//绘制卡尺工具结果
void drawCaliperToolView(ImageLabelView* view, const CaliperTool& caliperTool, const QColor& color, int thickness){
    //绘制矩形
    int h=caliperTool._projectLength;
    int w=caliperTool._searchLength;
    //夹角
    double angle=atan2(caliperTool._to.y-caliperTool._from.y, caliperTool._to.x-caliperTool._from.x)*180.0/M_PI;

    //中心点
    QPoint center((caliperTool._from.x+caliperTool._to.x)/2, (caliperTool._from.y+caliperTool._to.y)/2);

    view->addLabel(new ImageRotatedRectLabel(w, h, center, angle, color, thickness));

    //绘制方向箭头
    QPoint arrowStart(caliperTool._from.x, caliperTool._from.y);
    QPoint arrowEnd(caliperTool._to.x, caliperTool._to.y);
    view->addLabel(new ImageArrowLabel(arrowStart, arrowEnd, color, 1, 4));


    //绘制点
    //反色
    QColor invColor = QColor(255 - color.red(), 255 - color.green(), 255 - color.blue());
    for(const auto& pt: caliperTool._res){
        ImagePointLabel* label = new ImagePointLabel(QPoint(pt.x, pt.y), 8*thickness, invColor, thickness, angle);
        view->addLabel(label);
    }
}



//绘制圆检测结果
void drawCircleFindView(ImageLabelView* view, const CircleFind& circleFind, const QColor& color, int thickness){
    //绘制定位圆
    QPoint center(circleFind.center.x, circleFind.center.y);
    view->addLabel(new ImageEllipseLabel(center, circleFind.radius, color, thickness));

    //使用反色绘制测量圆
    QColor invColor = QColor(255 - color.red(), 255 - color.green(), 255 - color.blue());
    if(validCircle(circleFind._res)){
        center = QPoint(circleFind._res.center.x, circleFind._res.center.y);
        view->addLabel(new ImageEllipseLabel(center, circleFind._res.radius, invColor, thickness));
    }

    
    //绘制卡尺
    //第一个卡尺使用反色绘制
    if(!circleFind._calipers.empty()){
        drawCaliperToolView(view, circleFind._calipers[0], invColor, thickness);
        for(size_t i = 1; i < circleFind._calipers.size(); ++i){
            drawCaliperToolView(view, circleFind._calipers[i], color, thickness);
        }
    }

}


//绘制椭圆检测结果
void drawEllipseFindView(ImageLabelView* view, const EllipseFind& ellipseFind, const QColor& color, int thickness){
    //绘制定位椭圆
    QPoint center(ellipseFind.center.x, ellipseFind.center.y);
    view->addLabel(new ImageEllipseLabel(center, ellipseFind.xRadius, ellipseFind.yRadius, ellipseFind.angle, color, thickness));

    //使用反色绘制测量椭圆
    QColor invColor = QColor(255 - color.red(), 255 - color.green(), 255 - color.blue());
    if(validEllipse(ellipseFind._res)){
        center = QPoint(ellipseFind._res.center.x, ellipseFind._res.center.y);
        view->addLabel(new ImageEllipseLabel(center, ellipseFind._res.xRadius, ellipseFind._res.yRadius, ellipseFind._res.angle, invColor, thickness));
    }

    
    //绘制卡尺
    //第一个卡尺使用反色绘制
    if(!ellipseFind._calipers.empty()){
        drawCaliperToolView(view, ellipseFind._calipers[0], invColor, thickness);
        for(size_t i = 1; i < ellipseFind._calipers.size(); ++i){
            drawCaliperToolView(view, ellipseFind._calipers[i], color, thickness);
        }
    }
}