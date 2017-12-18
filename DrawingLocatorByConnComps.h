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
	vector<Vec2f> lines;

	void getNeighbouringPixels(const Point& point, Mat& img, list<Point>& neighbours);
	void findConnectedComponents();
	void filterLargerComponents();
	void filterNotCollinearComponents();
	bool isPointOnLine(const Point& linePointA, const Point& linePointB, const Point& point);

public:
	DrawingLocatorByConnComps(const Mat& img);
	void findSchemesAndTables();

	void outlineSchemes(Mat& img);
	void outlineTables(Mat& img);
	void outlineSchemesAndTables(Mat& img);
};

#endif
