#pragma once

#include <math.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>

#include <boost/algorithm/string.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <opencv2/ml/ml.hpp>

using namespace cv;
using namespace std;

#ifndef DRAWINGLOCATORBYKEYDESC_H
#define DRAWINGLOCATORBYKEYDESC_H

class DrawingLocatorByKeyDesc
{
private:
	Ptr<ml::SVM> svm;


public:
	DrawingLocatorByKeyDesc();
	void train(const string& keysCsvPath, const string& classifierSavePath);
	void loadPretrainedModel(const string& classifierLoadPath);
	void saveKeyDescsToCsv(const string& imgsPath, const string& keysCsvPath, const char& classId);

	void outlineSchemes(Mat& img);
	void outlineTables(Mat& img);
	void outlineSchemesAndTables(Mat& img);
};

#endif
