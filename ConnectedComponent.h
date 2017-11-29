#pragma once

#include <opencv2/opencv.hpp>
#include <list>

using namespace cv;
using namespace std;

#ifndef CONNECTEDCOMPONENT_H
#define CONNECTEDCOMPONENT_H

class ConnectedComponent {

private:
	double longestDiagonal(const list<Point>& points);
	double euclideanDistance(const Point& a, const Point& b);

public:
	Point left, right, up, down;
	int blackPixels;
	double diagonal;
	bool isTable;
	int lineQnt;

	ConnectedComponent(Point l, Point r, Point u, Point d, int blacks, double diag);
	ConnectedComponent(list<Point> neighbours, const Mat& I);
};

#endif 