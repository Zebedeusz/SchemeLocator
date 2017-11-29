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
	list<ConnectedComponent> getConnectedComponents();
	void filterLargerComponents(const list<ConnectedComponent>& comps);

public:
	DrawingLocatorByConnComps(const Mat& img);
	void findConnectedComponents();

	void outlineSchemes(Mat& img);
	void outlineTables(Mat& img);
	void outlineSchemesAndTables(Mat& img);
};

#endif
