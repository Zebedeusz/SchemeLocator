#include "stdafx.h"
#include "DrawingLocatorByConnComps.h"


DrawingLocatorByConnComps::DrawingLocatorByConnComps(const Mat& img) : I(img) {}

void DrawingLocatorByConnComps::findSchemesAndTables() {
	findConnectedComponents();
	filterUniqueTables();
	filterLargerComponents();
	filterNotCollinearComponents();
	distinctSchemesFromTables();
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
			iter->isTable ? Scalar(0, 255, 0) : Scalar(0, 0, 255), 3);
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

	for (list<ConnectedComponent>::const_iterator iter = connectedComponents.begin();
		iter != connectedComponents.end(); iter++) {
		if(iter->isTable)
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

		if (!iter->isTable) {
			iter->calculateCentroid(I);
			centroids.at<uchar>(iter->centroid) = 255;
		}
	}

	vector<Vec2f> lines;
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

		if (!iter->isTable) {

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

			if (!isOnAnyLine)
				tempConnectedComponents.push_back(*iter);
			else
				connCompsOnLine.push_back(*iter);
		}
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

	for (list<ConnectedComponent>::const_iterator iter = connectedComponents.begin();
		iter != connectedComponents.end(); iter++) {
		if (iter->isTable)
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

void DrawingLocatorByConnComps::filterUniqueTables() {

	const int LINE_THICKNESS_THRESHOLD = 4;
	const int LINE_LENGTH_THRESHOLD = 50;

	list<ConnectedComponent> tables;
	list<ConnectedComponent> lines;

	for (list<ConnectedComponent>::iterator iter = connectedComponents.begin();
		iter != connectedComponents.end(); iter++) {

		if (iter->down.y - iter->up.y <= LINE_THICKNESS_THRESHOLD &&
			iter->right.x - iter->left.x >= LINE_LENGTH_THRESHOLD) {

			iter->calculateCentroid(I);

			lines.push_back(*iter);
		}
	}

	for (list<ConnectedComponent>::iterator iter = lines.begin(); iter != lines.end(); iter++) {

		list<ConnectedComponent> linesInTable;
		linesInTable.push_back(*iter);

		for (list<ConnectedComponent>::iterator iterIn = lines.begin(); iterIn != lines.end(); iterIn++) {

			if (iterIn->compare(*iter))
				continue;

			if (iter->centroid.x == iterIn->centroid.x)
				linesInTable.push_back(*iterIn);
		}

		if (linesInTable.size() > 1) {

			Point left(I.cols - 1, 0), right(0, 0), up(0, I.rows - 1), down(0, 0);

			for (list<ConnectedComponent>::iterator iter = linesInTable.begin(); iter != linesInTable.end(); iter++) {

				if (iter->left.x <= left.x)
					left = iter->left;
				if (iter->right.x >= right.x)
					right = iter->right;
				if (iter->up.y <= up.y)
					up = iter->up;
				if (iter->down.y >= down.y)
					down = iter->down;
			}

			tables.push_back(ConnectedComponent(left, right, up, down, true));
		}
	}

	tables.sort(ConnectedComponent::compareComponents);
	tables.unique();

	for (list<ConnectedComponent>::iterator iter = tables.begin(); iter != tables.end(); iter++)
		connectedComponents.push_back(*iter);
}

void DrawingLocatorByConnComps::distinctSchemesFromTables() {

	Mat tempI = I.clone();

	for (list<ConnectedComponent>::iterator iter = connectedComponents.begin();
		iter != connectedComponents.end(); iter++) {

		Mat compRect(tempI, Rect(Point(iter->left.x, iter->down.y), Point(iter->right.x, iter->up.y)));

		//convert to white on black image
		Mat subMat = Mat::ones(compRect.size(), compRect.type()) * 255;
		subtract(subMat, compRect, compRect);

		Mat vertical, horizontal, vertAndHorz;
		Mat structHorizontal = getStructuringElement(MORPH_RECT, Size(compRect.cols/15, 1));
		Mat structVertical = getStructuringElement(MORPH_RECT, Size(1, compRect.rows/12));

		//finding vertical lines
		erode(compRect, vertical, structVertical, Point(-1, -1));
		dilate(vertical, vertical, structVertical, Point(-1, -1));

		//finding horizontal lines
		erode(compRect, horizontal, structHorizontal, Point(-1, -1));
		dilate(horizontal, horizontal, structHorizontal, Point(-1, -1));

		//combining vertical and horizontal lines
		vertAndHorz = vertical + horizontal;

		Mat joints;
		bitwise_and(horizontal, vertical, joints);

		imwrite(
			"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/joints.jpg",
			joints);

		//method change
		//Mat tempCompRect = compRect.clone();

		//Mat structOnComp = getStructuringElement(MORPH_RECT, Size(2, 2));

		//erode(tempCompRect, tempCompRect, structOnComp, Point(-1, -1));
		//imwrite(
		//	"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/eroded.jpg",
		//	tempCompRect);
		//dilate(tempCompRect, tempCompRect, structOnComp, Point(-1, -1));
		//imwrite(
		//	"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/dilated.jpg",
		//	tempCompRect);

		////to black on white
		//subtract(subMat, tempCompRect, tempCompRect);

		//list<Point> pointsInComp;
		//for (auto& p : {Point(0, 0),
		//				Point(0, tempCompRect.rows - 1),
		//				Point(tempCompRect.cols - 1, 0),
		//				Point(tempCompRect.cols - 1, tempCompRect.rows - 1)})
		//{
		//	if (tempCompRect.at<uchar>(p) == 0) {
		//		getNeighbouringPixels(p, tempCompRect, pointsInComp);
		//		break;
		//	}
		//}

		//Mat fullComp = Mat::ones(compRect.size(), compRect.type()) * 255;

		//for (auto& p : pointsInComp)
		//	fullComp.at<uchar>(p.y, p.x) = 0;

		//imwrite(
		//	"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/fullComp.jpg",
		//	fullComp);

		//subtract(subMat, fullComp, fullComp);


		//bool areAllLinesNotOblique = equal(fullComp.begin<uchar>(), fullComp.end<uchar>(), vertAndHorz.begin<uchar>());

		//change of method
		//for (int i = 0; i < lines.size(); ++i) {

		//	double rho = lines[i][0], theta = lines[i][1];
		//	Point pt1, pt2;
		//	double a = cos(theta), b = sin(theta);
		//	double x0 = a*rho, y0 = b*rho;
		//	pt1.x = iter->left.x + cvRound(x0 + 1000 * (-b));
		//	pt1.y = iter->up.y + cvRound(y0 + 1000 * (a));
		//	pt2.x = iter->left.x + cvRound(x0 - 1000 * (-b));
		//	pt2.y = iter->up.y + cvRound(y0 - 1000 * (a));
		//	line(tempI, pt1, pt2, Scalar(0, 0, 255), 1, CV_AA);

		//	//double theta = lines[i][1];

		//	if (!isLineNotOblique(theta)){
		//		cout << theta << endl;
		//		areAllLinesNotOblique = false;
		//		break;
		//	}
		//}
		//imwrite(
		//	"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/lines.jpg",
		//	tempI);

		//finding lines parameters
		//vector<Vec2f> lines;
		//HoughLines(vertAndHorz, lines, 1, CV_PI / 180, 200);

		//int verticalLinesQnt = countVerticalLines(lines);
		//int horizontalLinesQnt = countHorizontalLines(lines);

		//cout << "lines: " + lines.size() << endl;
		//cout << "Vert: " << verticalLinesQnt << endl;
		//cout << "Horz: " << horizontalLinesQnt << endl;

		if (hasRectangularContour(joints) && countCorners(joints) > 4 && isGrid(joints)) {
			iter->isTable = true;

			subtract(subMat, vertAndHorz, vertAndHorz);

			int minX, minY, maxX, maxY;
			minY = vertAndHorz.rows + 1;
			minX = vertAndHorz.cols + 1;
			maxY = -1;
			maxX = -1;

			for (int i = 0; i < vertAndHorz.rows; ++i) {
				for (int j = 0; j < vertAndHorz.cols; ++j) {
					if (vertAndHorz.at<uchar>(i, j) == 0) {
						if (i < minY)
							minY = i;
						if (j < minX)
							minX = j;
						if (i > maxY)
							maxY = i;
						if (j > maxX)
							maxX = j;
					}
				}
			}

			//removing signs outside if the table
			iter->left = Point(iter->left.x + minX, iter->up.y + minY);
			iter->right = Point(iter->left.x + maxX, iter->up.y + minY);
			iter->up = Point(iter->left.x + minX, iter->up.y + minY);
			iter->down = Point(iter->left.x + minX, iter->up.y + maxY);

			//method change
			//const double THRESHOLD = 0.02;

			//double rho = lines[0][0], theta = lines[0][1];
			//double a = cos(theta), b = sin(theta);
			//double x0 = a*rho, y0 = b*rho;

			//int minX, minY, maxX, maxY;
			//minY = tempI.rows + 1;
			//minX = tempI.cols + 1;
			//maxY = -1;
			//maxX = -1;

			////removing signs outside of the table
			//for (int i = 0; i < lines.size(); ++i) {
			//	rho = lines[i][0], theta = lines[i][1];
			//	a = cos(theta), b = sin(theta);
			//	x0 = a*rho, y0 = b*rho;

			//	//for all vertical lines find max and min y
			//	if ((theta < THRESHOLD) ||
			//		(theta < CV_PI + THRESHOLD && theta > CV_PI - THRESHOLD) ||
			//		(theta < 2 * CV_PI + THRESHOLD && theta > 2 * CV_PI - THRESHOLD))
			//	{
			//		int tempX = iter->left.x + cvRound(x0 + 1000 * (-b));
			//		if (tempX < minX)
			//			minX = tempX;
			//		if (tempX > maxX)
			//			maxX = tempX;
			//	}

			//	//for all horizontal lines find max and min x
			//	else if ((theta < CV_PI / 2 + THRESHOLD && theta > CV_PI / 2 - THRESHOLD) ||
			//		(theta < 3 * (CV_PI / 2) + THRESHOLD && theta > 3 * (CV_PI / 2) - THRESHOLD))
			//	{
			//		int tempY = iter->up.y + cvRound(y0 + 1000 * (a));
			//		if (tempY < minY)
			//			minY = tempY;
			//		if (tempY > maxY)
			//			maxY = tempY;
			//	}
			//}

			////change component boundaries: left = min x, right = max x, up = min y, down = max y
			//iter->left = Point(minX, minY);
			//iter->right = Point(maxX, minY);
			//iter->up = Point(minX, minY);
			//iter->down = Point(minX, maxY);
		}

		//CHANGE OF CONCEPTION
		//in a table all horizontal lines have equal centroids - the same with vertical lines
		//bool areAllHorizontalLinesEqualInCentroids = false, areAllVerticalLinesEqualInCentroids = false;
		//if (areAllLinesNotOblique) {

		//	//creating vextors for horizontal and vertical lines
		//	vector<Vec2f> linesHorizontal, linesVertical;

		//	for (int i = 0; i < lines.size(); ++i) {
		//		double theta = lines[i][1];

		//		if (theta == 0 || theta == CV_PI || theta == 2 * CV_PI)
		//			linesHorizontal.push_back(lines[i]);
		//		else if (theta == CV_PI/2 || theta == CV_PI + CV_PI/2)
		//			linesVertical.push_back(lines[i]);
		//	}

		//	//checking if all horizontal lines have equal centroids
		//	list<Point> centroids;
		//	for (int i = 0; i < linesHorizontal.size(); ++i) {

		//		double rho = lines[i][0];
		//		double theta = linesHorizontal[i][1];
		//		double a0 = cos(theta), b0 = sin(theta);
		//		double x0 = a0*rho, y0 = b0*rho;
		//		int y = cvRound(y0 + iter->left.y * (a0));

		//		centroids.push_back(ConnectedComponent(
		//			Point(iter->left.x, y),
		//			Point(iter->right.x, y),
		//			Point(iter->left.x, y),
		//			Point(iter->left.x, y), false).calculateCentroid(I));
		//	}

		//	centroids.unique();
		//	if (centroids.size() == 1)
		//		areAllHorizontalLinesEqualInCentroids = true;

		//	centroids.clear();

		//	//checking if all vertical lines have equal centroids
		//	for (int i = 0; i < linesVertical.size(); ++i) {

		//		double rho = lines[i][0];
		//		double theta = linesVertical[i][1];
		//		double a0 = cos(theta), b0 = sin(theta);
		//		double x0 = a0*rho, y0 = b0*rho;
		//		int x = cvRound(x0 + iter->left.x * (-b0));

		//		centroids.push_back(ConnectedComponent(
		//			Point(x, iter->left.y),
		//			Point(x, iter->right.y),
		//			Point(x, iter->up.y),
		//			Point(x, iter->down.y), false).calculateCentroid(I));
		//	}

		//	centroids.unique();
		//	if (centroids.size() == 1)
		//		areAllVerticalLinesEqualInCentroids = true;
		//}

		//if (areAllHorizontalLinesEqualInCentroids && areAllVerticalLinesEqualInCentroids)
		//	iter->isTable = true;
	}
}

bool DrawingLocatorByConnComps::isLineNotOblique(const double& theta) {

	const double THRESHOLD = 0.02;
	bool isLineNotOblique = false;

	//horizontal check - 0 deg
	isLineNotOblique |= theta < THRESHOLD;
	//horizontal check - 180 deg
	isLineNotOblique |= theta < CV_PI + THRESHOLD && theta > CV_PI - THRESHOLD;
	//horizontal check - 360 deg
	isLineNotOblique |= theta < 2 * CV_PI + THRESHOLD && theta > 2 * CV_PI - THRESHOLD;
	//vertical check - 90 deg
	isLineNotOblique |= theta < CV_PI / 2 + THRESHOLD && theta > CV_PI / 2 - THRESHOLD;
	//vertical check - 270 deg
	isLineNotOblique |= theta < 3 * (CV_PI / 2) + THRESHOLD && theta > 3 * (CV_PI / 2) - THRESHOLD;

	return isLineNotOblique;
}

int DrawingLocatorByConnComps::countHorizontalLines(const vector<Vec2f>& lines) {
	const double THRESHOLD = 0.02;
	int cnt = 0;
	
	for (int i = 0; i < lines.size(); ++i) {
		double theta = lines[i][1];

		//vertical check - 90 deg
		if (theta < CV_PI / 2 + THRESHOLD && theta > CV_PI / 2 - THRESHOLD)
			cnt++;

		//vertical check - 270 deg
		else if (theta < 3 * (CV_PI / 2) + THRESHOLD && theta > 3 * (CV_PI / 2) - THRESHOLD)
			cnt++;
	}

	return cnt;
}

int DrawingLocatorByConnComps::countVerticalLines(const vector<Vec2f>& lines) {
	const double THRESHOLD = 0.02;
	int cnt = 0;

	for (int i = 0; i < lines.size(); ++i) {
		double theta = lines[i][1];

		//horizontal check - 0 deg
		if (theta < THRESHOLD)
			cnt++;

		//horizontal check - 180 deg
		else if (theta < CV_PI + THRESHOLD && theta > CV_PI - THRESHOLD)
			cnt++;

		//horizontal check - 360 deg
		else if (theta < 2 * CV_PI + THRESHOLD && theta > 2 * CV_PI - THRESHOLD)
			cnt++;
	}

	return cnt;
}

bool DrawingLocatorByConnComps::isGrid(const Mat& I) {
	for (int i = 0; i < I.rows; ++i) {
		for (int j = 0; j < I.cols; ++j) {
			if (I.at<uchar>(i, j) == 255) {

				//check if there's corresponding point to the left
				//bool hasCorrespondingPointToTheLeft = false;
				//if (j != 0) {
				//	for (int c = j; c >= 0; --c) {
				//		if (I.at<uchar>(i, c) == 255) {
				//			hasCorrespondingPointToTheLeft = true;
				//			break;
				//		}
				//	}
				//}
				//if (!hasCorrespondingPointToTheLeft)
				//	return false;

				//check if there's corresponding point to the right
				if (j != I.cols - 1) {
					bool hasCorrespondingPointToTheRight = false;

					for (int c = j; c < I.cols; ++c) {
						if (I.at<uchar>(i, c) == 255) {
							hasCorrespondingPointToTheRight = true;
							break;
						}
					}

					if (!hasCorrespondingPointToTheRight)
						return false;
				}

				////check if there's corresponding point higher
				//if (i != 0) {

				//}

				//check if there's corresponding point lower
				if (i != I.rows - 1) {
					bool hasCorrespondingPointLower = false;

					for (int r = i; r < I.rows; ++r) {
						if (I.at<uchar>(r, j) == 255) {
							hasCorrespondingPointLower = true;
							break;
						}
					}

					if (!hasCorrespondingPointLower)
						return false;
				}
			}
		}
	}
	return true;
}

bool DrawingLocatorByConnComps::hasRectangularContour(const Mat& I) {

	if (I.at<uchar>(0, 0) != 255)
		return false;
	if (I.at<uchar>(0, I.rows - 1) != 255)
		return false;
	if (I.at<uchar>(I.cols - 1, 0) != 255)
		return false;
	if (I.at<uchar>(I.rows - 1, I.cols - 1) != 255)
		return false;
	return true;
}

int DrawingLocatorByConnComps::countCorners(const Mat& I) {

	int cnt = 0;

	for (int i = 0; i < I.rows; ++i) {
		for (int j = 0; j < I.cols; ++j) {
			if (I.at<uchar>(i, j) == 255)
				cnt++;
		}
	}

	return cnt;
}





