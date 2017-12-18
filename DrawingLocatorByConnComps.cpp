#include "stdafx.h"
#include "DrawingLocatorByConnComps.h"


DrawingLocatorByConnComps::DrawingLocatorByConnComps(const Mat& img) : I(img) {}

void DrawingLocatorByConnComps::findSchemesAndTables() {
	findConnectedComponents();
	filterLargerComponents();
	filterNotCollinearComponents();
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
			iter->isTable ? Scalar(0, 0, 255) : Scalar(0, 0, 255), 3);
		//cout << iter->area << endl;
	}
	//cout << endl;

	//for (int i = 0; i < lines.size(); ++i) {
	//	cout << lines.at(i).row(0) << lines.at(i).row(1) << endl;

	//	double rho = lines[i][0], theta = lines[i][1];
	//	Point pt1, pt2;
	//	double a = cos(theta), b = sin(theta);
	//	double x0 = a*rho, y0 = b*rho;
	//	pt1.x = cvRound(x0 + 1000 * (-b));
	//	pt1.y = cvRound(y0 + 1000 * (a));
	//	pt2.x = cvRound(x0 - 1000 * (-b));
	//	pt2.y = cvRound(y0 - 1000 * (a));
	//	line(img, pt1, pt2, Scalar(0, 0, 255), 1, CV_AA);
	//}
	//cout << endl;
}

void DrawingLocatorByConnComps::filterLargerComponents() {

	int sumAreas = 0;
	//double sumDiag = 0;
	for (list<ConnectedComponent>::const_iterator iter = connectedComponents.begin(); 
		iter != connectedComponents.end(); iter++) {
		//sumDiag += iter->diagonal;
		sumAreas += iter->area;
	}

	//double threshold = 5 * (sumDiag / (double)comps.size());
	double threshold = 6 * (sumAreas / (double)connectedComponents.size());

	list<ConnectedComponent> tempConnComps;
	for (list<ConnectedComponent>::const_iterator iter = connectedComponents.begin(); 
		iter != connectedComponents.end(); iter++) {
		//if (iter->diagonal >= threshold)
		if (iter->area >= threshold)
			//connectedComponents.push_back(*iter);
			tempConnComps.push_back(*iter);
	}
	connectedComponents = tempConnComps;
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

void DrawingLocatorByConnComps::findConnectedComponents() {

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
				connectedComponents.push_back(ConnectedComponent(neighbours, tempI));
			}
		}
	}
}

void DrawingLocatorByConnComps::filterNotCollinearComponents() {

	double sumHeights = 0, sumWidths = 0;
	for (list<ConnectedComponent>::const_iterator iter = connectedComponents.begin();
		iter != connectedComponents.end(); iter++) {
		sumHeights += iter->down.y - iter->up.y;
		sumWidths += iter->right.x - iter->left.x;
	}

	double avgHeight = sumHeights / (double) connectedComponents.size();
	const double DISTANCE_RESOUTION = 0.2 * avgHeight;

	double avgWidth = sumWidths / (double)connectedComponents.size();
	const int WIDTH_THRESHOLD = avgWidth / 3;

	Mat centroids = Mat::zeros(I.rows, I.cols, I.type());

	for (list<ConnectedComponent>::iterator iter = connectedComponents.begin();
		iter != connectedComponents.end(); iter++) {

		Mat compRect(I, Rect(Point(iter->left.x, iter->down.y), Point(iter->right.x, iter->up.y)));

		Moments mms = moments(compRect, true);

		iter->centroid = Point(iter->left.x + (mms.m10 / mms.m00), iter->up.y + (mms.m01 / mms.m00));

		centroids.at<uchar>(iter->centroid) = 255;
	}

	//vector<Vec2f> lines;
	vector<Vec2f> tempLines;

	HoughLines(centroids, lines, DISTANCE_RESOUTION, CV_PI / 180, 4, 0, 0, 0, 2 * (CV_PI / 180));
	HoughLines(centroids, tempLines, DISTANCE_RESOUTION, CV_PI / 180, 4, 0, 0, 88 * (CV_PI / 180), 92 * (CV_PI / 180));
	lines.insert(lines.end(), tempLines.begin(), tempLines.end());
	HoughLines(centroids, tempLines, DISTANCE_RESOUTION, CV_PI / 180, 4, 0, 0, 178 * (CV_PI / 180), 180 * (CV_PI / 180));
	lines.insert(lines.end(), tempLines.begin(), tempLines.end());

	//iterate over all components
	//iterate over all lines
	//if a component does not lie on a line it's a scheme
	list<ConnectedComponent> tempConnectedComponents;
	list<ConnectedComponent> connCompsOnLine;
	for (list<ConnectedComponent>::iterator iter = connectedComponents.begin();
		iter != connectedComponents.end(); iter++) {

		bool isOnAnyLine = false;

		for (int i = 0; i < lines.size(); ++i) {
			double rho = lines[i][0], theta = lines[i][1];
			Point pt1, pt2;
			double a0 = cos(theta), b0 = sin(theta);
			double x0 = a0*rho, y0 = b0*rho;
			pt1.x = cvRound(x0 + 1000 * (-b0));
			pt1.y = cvRound(y0 + 1000 * (a0));
			pt2.x = cvRound(x0 - 1000 * (-b0));
			pt2.y = cvRound(y0 - 1000 * (a0));

			//double a = tan((CV_PI / 2) + theta);
			//double b = rho / sin(theta);

			//int cenX = iter->centroid.x, cenY = iter->centroid.y;
			//int lineX0 = cvRound(x0 + (iter->centroid.y)*(-b0));
			//int lineY0 = cvRound(y0 + (iter->centroid.x)*(a0));
			//double lineX = (((double)iter->centroid.y - b) / a);
			//double lineY = (a*(double)iter->centroid.x + b);

			//if (iter->centroid.x == cvRound(x0 + (iter->centroid.y)*(-b)) &&
			//	iter->centroid.y == cvRound(y0 + (iter->centroid.x)*(a)))
			//	tempConnComps.push_back(*iter);

			//if (iter->centroid.x == lineX && iter->centroid.y == lineY)
			//	tempConnComps.push_back(*iter);

			if (isPointOnLine(pt1, pt2, iter->centroid)) {
				isOnAnyLine = true;
				break;
			}
		}

		if(!isOnAnyLine)
			tempConnectedComponents.push_back(*iter);
		else
			connCompsOnLine.push_back(*iter);
	}
	
	//iterate over all components on line
	//if distance between a component and any component from the rest on lines is greater than threshhold it's a scheme
	for (list<ConnectedComponent>::iterator iter = connCompsOnLine.begin();
		iter != connCompsOnLine.end(); iter++) {

		bool isCloseToAnyConnComp = false;

		for (list<ConnectedComponent>::iterator iterRest = connCompsOnLine.begin();
			iterRest != connCompsOnLine.end(); iterRest++) {

			if (iterRest->compare(*iter))
				continue;

			int distanceBetweenConnComps = min(abs(iter->left.x - iterRest->right.x),
				abs(iterRest->left.x - iter->right.x));
			
			if (distanceBetweenConnComps < WIDTH_THRESHOLD) {
				isCloseToAnyConnComp = true;
				break;
			}
		}

		if (!isCloseToAnyConnComp)
			tempConnectedComponents.push_back(*iter);
	}

	connectedComponents = tempConnectedComponents;
}

bool DrawingLocatorByConnComps::isPointOnLine(const Point& linePointA, const Point& linePointB, const Point& point){
	const double EPSILON = 14.0;

	double a = (linePointB.y - linePointA.y) / (linePointB.x - linePointA.x);
	double b = linePointA.y - a * linePointA.x;
	double f = fabs(point.y - (a*point.x + b));
	if (f < EPSILON)
		return true;

	return false;
}


