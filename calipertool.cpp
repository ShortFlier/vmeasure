
//

#include "CaliperTool.h"

//返回同一直线上距离点p0为d的两个点
//返回的向量(p1-p0)方向和dir相同
std::pair<cv::Point2d, cv::Point2d> pointsAlongLine(
	const cv::Point2d& p0,
	const cv::Vec2d& dir,
	double d
) {
	cv::Vec2d unit = dir / cv::norm(dir);  // 归一化
	cv::Vec2d offset = d * unit;           // 位移向量

	return {
		p0 - cv::Point2d(offset[0], offset[1]),
		p0 + cv::Point2d(offset[0], offset[1])
	};
}


std::pair<cv::Point2d, cv::Point2d> pointsAlongLine(
	const cv::Point2d& p0,
	double angle,
	double d
) {
	double rad = angle * CV_PI / 180.0;
	cv::Vec2d dir(std::cos(rad), std::sin(rad));
	return pointsAlongLine(p0, dir, d);
}




CaliperTool::CaliperTool(Polar polar, int edgeLength, int threshold, int maxNum)
:polar(polar), edgeLength(edgeLength), threshold(threshold), maxNum(maxNum){}

std::vector<cv::Point2d> CaliperTool::measure(const cv::Mat& mat, const cv::Point2d& center, int angle, int projectLen, int searchLen)
{
	_projectLength = projectLen;
	_searchLength = searchLen;
	//计算起始，结束点
	auto points = pointsAlongLine(center, angle, searchLen / 2.0);
	_from = points.first;
	_to = points.second;

	return measure(mat);
}

std::vector<cv::Point2d> CaliperTool::measure(const cv::Mat& mat, const cv::Point2d& from, const cv::Point2d& to, int projectLen)
{
	_from = from;
	_to = to;
	_projectLength = projectLen;
	_searchLength = std::ceil(cv::norm(to - from));
	return measure(mat);
}

/*
	获取点集
	图片中采样
	投影
	边缘梯度
	输出点
*/
std::vector<cv::Point2d> CaliperTool::measure(const cv::Mat& mat)
{
	_res = std::vector<cv::Point2d>();

	if(mat.empty()){
		std::cerr << "Input image is empty." << std::endl;
		return _res;
	}
	//限制输入为8位单通道图像
	if(mat.type() != CV_8UC1){
		std::cerr << "Input image must be 8-bit single-channel." << std::endl;
		return _res;
	}

	//设置_projectLength为奇数
	if (_projectLength % 2 == 0) {
		_projectLength++;
	}

	/*
	获取点集，_from到_to上的点，每个距离为1
	同时获取直线(_from, _to)上/下平行的_projectLength/2条上的点，两条线距离为1
	*/
	std::vector<std::vector<cv::Point2d>> points(_projectLength, std::vector<cv::Point2d>(_searchLength));

	//填充中线
	std::vector<cv::Point2d>& centerLine = points[_projectLength / 2];
	cv::Vec2d dir = _to - _from;
	centerLine[0] = _from;
	for(int i=1; i<_searchLength; ++i){
		auto pair=pointsAlongLine(centerLine[i-1], dir, 1);
		centerLine[i] = pair.second;
	}

	//填充平行线
	cv::Vec2d perDir(dir[1], -dir[0]); // 顺时针旋转90度
	for(int i=0; i<_searchLength; ++i){
		int n=_projectLength/2;
		for(int j=0; j<n; ++j){
			auto pair=pointsAlongLine(centerLine[i], perDir, j+1);
			points[n-1-j][i] = pair.second;
			points[n+1+j][i] = pair.first;
		}
	}


	/*
	根据点集在图片中采样
	*/
	cv::Mat samples(_projectLength, _searchLength, CV_8U);
	for(int i=0; i<_projectLength; ++i){
		for(int j=0; j<_searchLength; ++j){
			cv::Point2d p = points[i][j];
			cv::Mat subPix;
			try{
				cv::getRectSubPix(mat, cv::Size(1, 1), p, subPix);
				samples.at<uchar>(i, j) = subPix.at<uchar>(0, 0);
			}
			catch(const cv::Exception& e){//越界设置点为0
				std::cout<<e.what()<<std::endl;
				samples.at<uchar>(i, j) = 0;
			}
		}
	}

	/*
	投影
	*/
	cv::Mat projection;
	cv::reduce(samples, projection, 0, cv::REDUCE_AVG);


	/*
	梯度计算
	*/
	cv::Mat gradient;
	//滤波核(-1,-1,0,1,1)，长度为edgeLength*2+1
	cv::Mat kernel(1, edgeLength*2+1, CV_32F);
	for(int i=0; i<edgeLength; ++i){
		kernel.at<float>(0, i) = -1;
		kernel.at<float>(0, edgeLength) = 0;
		kernel.at<float>(0, edgeLength+1+i) = 1;
	}
	cv::filter2D(projection, gradient, CV_32F, kernel);

	//极性过滤
	for(int i=0; i<gradient.cols; ++i){
		//ANY保留所有梯度，BRIGHT2DARK保留负梯度，DARK2BRIGHT保留正梯度
		if(polar==ANY){
			gradient.at<float>(0, i) = std::abs(gradient.at<float>(0, i));
		}else if(polar==BRIGHT2DARK){
			if(gradient.at<float>(0, i) > 0){
				gradient.at<float>(0, i) = 0;
			}else{
				gradient.at<float>(0, i) = std::abs(gradient.at<float>(0, i));
			}
		}else if(polar==DARK2BRIGHT){
			if(gradient.at<float>(0, i) < 0){
				gradient.at<float>(0, i) = 0;
			}
		}else{
			std::cerr << "Invalid polar type. Using ANY." << std::endl;
			gradient.at<float>(0, i) = std::abs(gradient.at<float>(0, i));
		}
	}

	
	//输出点（阈值大于0）、maxNum个
	cv::Mat gmat;
	gradient.convertTo(gmat, CV_8U);

	//降序排序，最多maxNum个
	std::vector<std::pair<float, int>> gradIndex;
	for(int i=0; i<gradient.cols; ++i){
		gradIndex.emplace_back(gradient.at<float>(0, i), i);
	}
	std::sort(gradIndex.begin(), gradIndex.end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b){
		return a.first > b.first;
	});
	std::vector<int> indices;
	for(int i=0; i<std::min(maxNum, static_cast<int>(gradIndex.size())); ++i){
		if(gradIndex[i].first > threshold){
			indices.push_back(gradIndex[i].second);
		}
	}
	//取点
	for(int idx: indices){
		_res.push_back(centerLine[idx]);
	}


	return _res;
}


void CaliperTool::drawRes(cv::Mat& inputoutput, cv::Scalar color, int thickness) const{
	//绘制_form到_to的方向箭头
	cv::arrowedLine(inputoutput, _from, _to, color, thickness);

	//卡尺绘制范围
	cv::Vec2d dir = _to - _from;
	cv::Vec2d perDir(dir[1], -dir[0]); 
	auto pair0= pointsAlongLine(_from, perDir, _projectLength/2.0);
	auto pair1= pointsAlongLine(_to, perDir, -_projectLength/2.0);
	std::vector<cv::Point> corners = {pair0.first, pair0.second, pair1.first, pair1.second};
	cv::polylines(inputoutput, corners, true, color, thickness);

	//绘制结果点
	//使用反色绘制结果点
	cv::Scalar resColor(255 - color[0], 255 - color[1], 255 - color[2]);
	for(size_t i=0; i<_res.size(); ++i){
		cv::drawMarker(inputoutput, _res[i], resColor, cv::MARKER_CROSS, thickness*4, thickness);
	}
}