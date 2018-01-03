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

	ConnectedComponent(const Point& l, const Point& r, const Point& u, const Point& d, const bool& table);
	ConnectedComponent(list<Point> neighbours, const Mat& I);
	bool compare(const ConnectedComponent& connComp);
	bool operator==(const ConnectedComponent& connComp) const;
	static bool compareComponents(const ConnectedComponent& connComp1, const ConnectedComponent& connComp2);
	const Point& calculateCentroid(const Mat& I);
};

#endif 