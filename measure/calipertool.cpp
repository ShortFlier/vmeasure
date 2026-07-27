#include "calipertool.h"

//

#include "calipertool.h"
#include "log.h"

#include <algorithm>
#include <limits>

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




CaliperTool::Polar CaliperTool::polarInt(int i)
{
	Polar polar=ANY;
	switch (i) {
	case 1: polar = BRIGHT2DARK; break;
	case 2: polar = DARK2BRIGHT; break;
	}

	return polar;
}

CaliperTool::OutputMode CaliperTool::outputModeInt(int i)
{
	OutputMode mode = BY_CONTRAST;
	switch (i) {
	case 1: mode = BY_SEARCH_DIR; break;
	case 2: mode = BY_CENTER_DIR; break;
	}

	return mode;
}

CaliperTool::CaliperTool(Polar polar, int edgeLength, int threshold, int maxNum, OutputMode outputMode)
:polar(polar), edgeLength(edgeLength), threshold(threshold), maxNum(maxNum), outputMode(outputMode){}

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
	_res.clear();

	if (!validateAndPrepare(mat)) {
		return _res;
	}

	std::vector<std::vector<cv::Point2d>> points(_projectLength, std::vector<cv::Point2d>(_searchLength));
	buildSampleGrid(points);

	cv::Mat samples;
	sampleImage(mat, points, samples);

	cv::Mat projection;
	computeProjection(samples, projection);

	cv::Mat gradient;
	computeGradientAndFilter(projection, gradient);

	std::vector<int> indices;
	selectEdges(gradient, indices);

	std::vector<cv::Point2d>& centerLine = points[_projectLength / 2];
	for (int idx : indices) {
		if (idx >= 0 && idx < static_cast<int>(centerLine.size())) {
			_res.push_back(centerLine[idx]);
		}
	}

	return _res;
}


bool CaliperTool::validateAndPrepare(const cv::Mat& mat)
{
	if (mat.empty()) {
		log_error("CaliperTool::measure Input image is empty.");
		return false;
	}
	if (mat.type() != CV_8UC1) {
		log_error("CaliperTool::measure Input image must be 8-bit single-channel.");
		return false;
	}
	if (_projectLength % 2 == 0) {
		_projectLength++;
	}
	return true;
}

void CaliperTool::buildSampleGrid(std::vector<std::vector<cv::Point2d>>& points)
{
	std::vector<cv::Point2d>& centerLine = points[_projectLength / 2];
	cv::Vec2d dir = _to - _from;
	centerLine[0] = _from;
	for (int i = 1; i < _searchLength; ++i) {
		auto pair = pointsAlongLine(centerLine[i - 1], dir, 1);
		centerLine[i] = pair.second;
	}

	cv::Vec2d perDir(dir[1], -dir[0]);
	for (int i = 0; i < _searchLength; ++i) {
		int n = _projectLength / 2;
		for (int j = 0; j < n; ++j) {
			auto pair = pointsAlongLine(centerLine[i], perDir, j + 1);
			points[n - 1 - j][i] = pair.second;
			points[n + 1 + j][i] = pair.first;
		}
	}
}

void CaliperTool::sampleImage(const cv::Mat& mat, const std::vector<std::vector<cv::Point2d>>& points, cv::Mat& samples)
{
	samples.create(_projectLength, _searchLength, CV_8U);
	for (int i = 0; i < _projectLength; ++i) {
		for (int j = 0; j < _searchLength; ++j) {
			cv::Point2d p = points[i][j];
			cv::Mat subPix;
			try {
				cv::getRectSubPix(mat, cv::Size(1, 1), p, subPix);
				samples.at<uchar>(i, j) = subPix.at<uchar>(0, 0);
			}
			catch (const cv::Exception& e) {
				log_warn(std::string("CaliperTool::sampleImage  ")+e.what());
				samples.at<uchar>(i, j) = 0;
			}
		}
	}
}

void CaliperTool::computeProjection(const cv::Mat& samples, cv::Mat& projection)
{
	cv::reduce(samples, projection, 0, cv::REDUCE_AVG);
}

void CaliperTool::computeGradientAndFilter(cv::Mat& projection, cv::Mat& gradient)
{
	int klen = edgeLength * 2 + 1;
	cv::Mat kernel(1, klen, CV_32F, cv::Scalar(0));
	for (int i = 0; i < edgeLength; ++i) {
		kernel.at<float>(0, i) = -1;
		kernel.at<float>(0, edgeLength + 1 + i) = 1;
	}
	// center remains 0
	cv::filter2D(projection, gradient, CV_32F, kernel);

	for (int i = 0; i < gradient.cols; ++i) {
		float val = gradient.at<float>(0, i);
		if (polar == BRIGHT2DARK) {	//保留负值
			if (val > 0) gradient.at<float>(0, i) = 0;
			else gradient.at<float>(0, i) = std::abs(val);
		}
		else if (polar == DARK2BRIGHT) {	//保留正值
			if (val < 0) gradient.at<float>(0, i) = 0;
			// positive stays as-is
		}
		else {
			gradient.at<float>(0, i) = std::abs(val);
		}
	}
}

void CaliperTool::selectEdges(const cv::Mat& gradient, std::vector<int>& indices)
{
	indices.clear();
	if (gradient.empty() || gradient.cols <= 0 || maxNum <= 0) {
		return;
	}

	struct EdgeScore {
		int index = 0;
		float contrast = 0.0f;
		float score = 0.0f;
	};

	const int n = gradient.cols;
	const float eps = 1e-6f;
	float maxContrast = 0.0f;
	for (int i = 0; i < n; ++i) {
		float val = gradient.at<float>(0, i);
		if (val > maxContrast) {
			maxContrast = val;
		}
	}

	const float denominatorC = std::max(maxContrast - static_cast<float>(threshold), eps);
	const float center = (static_cast<float>(n) - 1.0f) * 0.5f;
	const float centerNorm = std::max(center, eps);
	const float searchNorm = std::max(static_cast<float>(n - 1), 1.0f);

	// 归一化定义：
	// C = max(contrast - threshold, 0) / max(maxContrast - threshold, eps)
	// S = i / max(n - 1, 1)                     (沿 _from -> _to 的搜索方向位置)
	// D = |i - center| / max(center, eps)       (相对中心的归一化距离)

	std::vector<EdgeScore> candidates;
	candidates.reserve(n);
	for (int i = 0; i < n; ++i) {
		float contrast = gradient.at<float>(0, i);
		if (contrast <= static_cast<float>(threshold)) {
			continue;
		}

		float cNorm = std::max(contrast - static_cast<float>(threshold), 0.0f) / denominatorC;
		float sNorm = static_cast<float>(i) / searchNorm;
		float dNorm = std::abs(static_cast<float>(i) - center) / centerNorm;

		float score = 0.0f;
		switch (outputMode) {
		case BY_SEARCH_DIR:
			// 搜索方向模式：
			// Score_search = 0.60 * (1 - S) + 0.30 * C + 0.10 * (1 - D)
			// 分值越高越优先，强调靠近搜索起点，其次对比度，再次中心距离。
			score = 0.60f * (1.0f - sNorm) + 0.30f * cNorm + 0.10f * (1.0f - dNorm);
			break;
		case BY_CENTER_DIR:
			// 中心方向模式：
			// Score_center = 0.60 * (1 - D) + 0.35 * C + 0.05 * (1 - S)
			// 分值越高越优先，强调靠近中心，其次对比度，最后搜索方向位置。
			score = 0.60f * (1.0f - dNorm) + 0.35f * cNorm + 0.05f * (1.0f - sNorm);
			break;
		case BY_CONTRAST:
		default:
			// 对比度模式：
			// Score_contrast = 0.85 * C + 0.15 * (1 - D)
			// 分值越高越优先，主要按对比度排序，中心距离用于次级稳定。
			score = 0.85f * cNorm + 0.15f * (1.0f - dNorm);
			break;
		}

		candidates.push_back(EdgeScore{i, contrast, score});
	}

	std::sort(candidates.begin(), candidates.end(), [](const EdgeScore& a, const EdgeScore& b) {
		if (a.score == b.score) {
			if (a.contrast == b.contrast) {
				return a.index < b.index;
			}
			return a.contrast > b.contrast;
		}
		return a.score > b.score;
	});

	int count = std::min(maxNum, static_cast<int>(candidates.size()));
	for (int i = 0; i < count; ++i) {
		indices.push_back(candidates[i].index);
	}
}


void CaliperTool::drawRes(cv::Mat& inputoutput, cv::Scalar color, int thickness) const{
	//绘制_form到_to的方向箭头
	cv::arrowedLine(inputoutput, _from, _to, color, thickness);

	//卡尺绘制范围矩形框
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