#pragma once

#include <opencv2/opencv.hpp>
#include <list>

using namespace cv;
using namespace std;

#ifndef CONNECTEDCOMPONENT_H
#define CONNECTEDCOMPONENT_H

class ConnectedComponent {

private:
	double euclideanDistance(const Point& a, const Point& b);

public:
	Point left, right, up, down;
	int blackPixels;
	bool isTable;
	int area;
	Point centroid;

	ConnectedComponent(Point l, Point r, Point u, Point d, int blacks);
	ConnectedComponent(list<Point> neighbours, const Mat& I);
	bool compare(const ConnectedComponent& connComp);
};

#endif 