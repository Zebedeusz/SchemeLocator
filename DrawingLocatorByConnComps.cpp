#include "stdafx.h"
#include "DrawingLocatorByConnComps.h"


DrawingLocatorByConnComps::DrawingLocatorByConnComps(const Mat& img) : I(img) {}

void DrawingLocatorByConnComps::findConnectedComponents() {
	filterLargerComponents(getConnectedComponents());
}

void DrawingLocatorByConnComps::outlineSchemes(Mat& img) {
	img = I.clone();
	cvtColor(img, img, COLOR_GRAY2BGR);

	for (list<ConnectedComponent>::const_iterator iter = connectedComponents.begin(); 
		iter != connectedComponents.end();
		iter++) {
		rectangle(img,
			Point(iter->left.x, iter->down.y),
			Point(iter->right.x, iter->up.y), Scalar(0, 128, 255));
	}
}

void DrawingLocatorByConnComps::outlineTables(Mat& img) {
	img = I.clone();
	cvtColor(img, img, COLOR_GRAY2BGR);

	for (list<ConnectedComponent>::const_iterator iter = connectedComponents.begin(); 
		iter != connectedComponents.end(); 
		iter++) {
		rectangle(img,
			Point(iter->left.x, iter->down.y),
			Point(iter->right.x, iter->up.y), Scalar(0, 128, 255), 1, 5, 0); 
	}
}

void DrawingLocatorByConnComps::outlineSchemesAndTables(Mat& img) {
	img = I.clone();
	cvtColor(img, img, COLOR_GRAY2BGR);

	for (list<ConnectedComponent>::const_iterator iter = connectedComponents.begin(); 
		iter != connectedComponents.end(); 
		iter++) {
		rectangle(img,
			Point(iter->left.x, iter->down.y),
			Point(iter->right.x, iter->up.y), 
			iter->isTable ? Scalar(0, 128, 255) : Scalar(0, 255, 128));
		double hor = double(iter->right.x - iter->left.x);
		double ver = double(iter->down.y - iter->up.y);
		cout << hor << " : " << ver << endl;
		//cout << iter->lineQnt << endl;
	}
}

void DrawingLocatorByConnComps::filterLargerComponents(const list<ConnectedComponent>& comps) {

	double sumDiag = 0;
	for (list<ConnectedComponent>::const_iterator iter = comps.begin(); iter != comps.end(); iter++) {
		sumDiag += iter->diagonal;
	}

	double threshold = 5 * (sumDiag / (double)comps.size());

	for (list<ConnectedComponent>::const_iterator iter = comps.begin(); iter != comps.end(); iter++) {
		//double dimsRatio = double((iter->right.x - iter->left.x)) / double((iter->down.y - iter->up.y));
		//double hor = double(iter->right.x - iter->left.x);
		//double ver = double(iter->down.y - iter->up.y);
		//cout << dimsRatio << endl;
		if (iter->diagonal >= threshold)
			connectedComponents.push_back(*iter);
	}
}

void DrawingLocatorByConnComps::getNeighbouringPixels(
	const Point& point, Mat& img, list<Point>& neighbours) {

	int initX = point.x - 1 >= 0 ? point.x - 1 : 0;
	int initY = point.y - 1 >= 0 ? point.y - 1 : 0;
	int finX = point.x + 1 < img.cols ? point.x + 1 : img.cols - 1;
	int finY = point.y + 1 < img.rows ? point.y + 1 : img.rows - 1;

	if (!(find(neighbours.begin(), neighbours.end(), point) != neighbours.end())){
		neighbours.push_back(point);
		img.at<uchar>(Point(point.x, point.y)) = 255;
	}

	for (int y = initY; y <= finY; y++) {
		for (int x = initX; x <= finX; x++) {
			if (x == point.x && y == point.y)
				continue;

			if (int(img.at<uchar>(Point(x, y))) == 0) {
				if (find(neighbours.begin(), neighbours.end(), Point(x, y)) != neighbours.end())
					continue;
				getNeighbouringPixels(Point(x, y), img, neighbours);
			}
		}
	}
}

list<ConnectedComponent> DrawingLocatorByConnComps::getConnectedComponents() {
	list<ConnectedComponent> comps;

	Mat tempI = I.clone();

	int channels = tempI.channels();
	int nRows = tempI.rows;
	int nCols = tempI.cols * channels;
	int i, j;

	for (i = 0; i < nRows; ++i) {
		for (j = 0; j < nCols; ++j) {
			if (int(tempI.at<uchar>(i, j)) == 0) {
				list<Point> neighbours;
				getNeighbouringPixels(Point(j, i), tempI, neighbours);
				comps.push_back(ConnectedComponent(neighbours, tempI));
			}
		}
	}

	return comps;
}


