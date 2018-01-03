#pragma once

#include <math.h>
#include <string>
#include <iostream>
#include <algorithm>

#include "ConnectedComponent.h"

#ifndef DRAWINGLOCATORBYCONNCOMPS_H
#define DRAWINGLOCATORBYCONNCOMPS_H

class DrawingLocatorByConnComps
{
private:
	list<ConnectedComponent> connectedComponents;
	Mat I;

	void getNeighbouringPixels(const Point& point, Mat& img, list<Point>& neighbours);
	void findConnectedComponents();
	void filterLargerComponents();
	void filterNotCollinearComponents();
	bool isPointOnLine(const Point& linePointA, const Point& linePointB, const Point& point);
	void filterUniqueTables();
	void distinctSchemesFromTables();
	bool isLineNotOblique(const double& theta);
	int countVerticalLines(const vector<Vec2f>& lines);
	int countHorizontalLines(const vector<Vec2f>& lines);
	bool isGrid(const Mat& I);
	bool hasRectangularContour(const Mat& I);
	int countCorners(const Mat& I);

public:
	DrawingLocatorByConnComps(const Mat& img);
	void findSchemesAndTables();

	void outlineSchemes(Mat& img);
	void outlineTables(Mat& img);
	void outlineSchemesAndTables(Mat& img);
};

#endif
