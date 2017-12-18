#pragma once

#include <math.h>
#include <string>
#include <iostream>
#include <filesystem>

#include "DrawingLocatorByKeyDesc.h"
#include "DrawingLocatorByConnComps.h"

void correctSkew(const Mat& img, Mat& imgRotated);
double variance(const vector<int>& vec);
void rotateImg(const Mat& src, Mat& dst, double angle);