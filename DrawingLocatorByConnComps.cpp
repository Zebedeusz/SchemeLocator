#include "stdafx.h"
#include "DrawingLocatorByConnComps.h"

DrawingLocatorByConnComps::DrawingLocatorByConnComps(const Mat& img) : I(img) {}

void DrawingLocatorByConnComps::findSchemesAndTables() {
	findConnectedComponents();
	filterUniqueTables();
	filterLargerComponents();
	filterNotCollinearComponents();
	distinctSchemesFromTables();
	finalChecks();

	cout << "Schemes and tables look-up finished" << endl;
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
	}

	cout << "Schemes and tables outlined" << endl;
}

void DrawingLocatorByConnComps::filterLargerComponents() {

	if (connectedComponents.size() > 0) {

		cout << "Large components filtering in progress" << endl;

		//double threshold = 6 * meanAreaOfComponents();
		double threshold = 13 * medianAreaOfComponents();

		list<ConnectedComponent> tempConnComps;
		for (list<ConnectedComponent>::const_iterator iter = connectedComponents.begin();
			iter != connectedComponents.end(); iter++) {
			if (iter->area >= threshold)
				tempConnComps.push_back(*iter);
		}

		for (list<ConnectedComponent>::const_iterator iter = connectedComponents.begin();
			iter != connectedComponents.end(); iter++) {
			if (iter->isTable)
				tempConnComps.push_back(*iter);
		}

		connectedComponents = tempConnComps;

		cout << "Found " + to_string(connectedComponents.size()) + " large components" << endl;
	}
}

int DrawingLocatorByConnComps::medianAreaOfComponents() {

	vector<int> areas;
	for (list<ConnectedComponent>::const_iterator iter = connectedComponents.begin();
		iter != connectedComponents.end(); iter++)
	{
		areas.push_back(iter->area);
	}

	sort(areas.begin(), areas.end());
	return areas.at(areas.size() / 2);
}

double DrawingLocatorByConnComps::meanAreaOfComponents() {

	int sumAreas = 0;
	for (list<ConnectedComponent>::const_iterator iter = connectedComponents.begin();
		iter != connectedComponents.end(); iter++) {
		sumAreas += iter->area;
	}

	return (sumAreas / (double)connectedComponents.size());
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

	cout << "Connected components look-up in progress" << endl;

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

	cout << "Found " + to_string(connectedComponents.size()) + " connected components" << endl;
}

void DrawingLocatorByConnComps::filterNotCollinearComponents() {

	if (connectedComponents.size() > 0) {

		cout << "Removing collinear components in progress" << endl;

		double sumHeights = 0, sumWidths = 0;
		for (list<ConnectedComponent>::const_iterator iter = connectedComponents.begin();
			iter != connectedComponents.end(); iter++) {
			sumHeights += iter->down.y - iter->up.y;
			sumWidths += iter->right.x - iter->left.x;
		}

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

		HoughLines(centroids, lines, 1, CV_PI / 180, 3, 0, 0, 0, 2 * (CV_PI / 180));
		HoughLines(centroids, tempLines, 1, CV_PI / 180, 3, 0, 0, 88 * (CV_PI / 180), 92 * (CV_PI / 180));
		lines.insert(lines.end(), tempLines.begin(), tempLines.end());
		HoughLines(centroids, tempLines, 1, CV_PI / 180, 3, 0, 0, 178 * (CV_PI / 180), 180 * (CV_PI / 180));
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

	if (connectedComponents.size() > 0) {

		cout << "Unique tables look-up in progress" << endl;

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

		cout << "Found " + to_string(tables.size()) + " unique tables" << endl;
	}
}

void DrawingLocatorByConnComps::distinctSchemesFromTables() {

	if (connectedComponents.size() > 0) {

		cout << "Tables from schemes distinction in progress" << endl;

		Mat tempI = I.clone();

		for (list<ConnectedComponent>::iterator iter = connectedComponents.begin();
			iter != connectedComponents.end(); iter++) {

			if (iter->isTable)
				continue;

			Mat compRect(tempI, Rect(Point(iter->left.x, iter->down.y), Point(iter->right.x, iter->up.y)));

			//convert to white on black image
			Mat subMat = Mat::ones(compRect.size(), compRect.type()) * 255;
			subtract(subMat, compRect, compRect);

			Mat vertical, horizontal, vertAndHorz;
			Mat structHorizontal = getStructuringElement(MORPH_RECT, Size(compRect.cols / 5, 1));
			Mat structVertical = getStructuringElement(MORPH_RECT, Size(1, compRect.rows / 5));

			//finding vertical lines
			erode(compRect, vertical, structVertical, Point(-1, -1));
			dilate(vertical, vertical, structVertical, Point(-1, -1));

			//finding horizontal lines
			erode(compRect, horizontal, structHorizontal, Point(-1, -1));
			dilate(horizontal, horizontal, structHorizontal, Point(-1, -1));

			//combining vertical and horizontal lines
			vertAndHorz = vertical + horizontal;

			//imwrite(
			//	"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/vertAndHorz.jpg",
			//	vertAndHorz);

			int blackPixelsInVertndHorz = countNonZero(vertAndHorz);

			Mat structVertHorz = getStructuringElement(MORPH_RECT, Size(6, 6));
			dilate(vertAndHorz, vertAndHorz, structVertHorz, Point(-1, -1));

			//imwrite(
			//	"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/vertAndHorz_dil.jpg",
			//	vertAndHorz);

			//erode(vertAndHorz, vertAndHorz, structVertHorz, Point(-1, -1));

			//imwrite(
			//	"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/vertAndHorz_er.jpg",
			//	vertAndHorz);

			//finding vertical lines
			//dilate(vertical, vertical, structVertical, Point(-1, -1)) : 
			erode(vertAndHorz, vertical, structVertical, Point(-1, -1));

			//finding horizontal lines
			//dilate(horizontal, horizontal, structHorizontal, Point(-1, -1)) :
			erode(vertAndHorz, horizontal, structHorizontal, Point(-1, -1));

			Mat joints;
			bitwise_and(horizontal, vertical, joints);

			//imwrite(
			//	"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/joints1.jpg",
			//	joints);

			//if component has no joints - it cannot be a table
			if (countNonZero(joints) == 0)
				continue;

			reduceJointsToPoints(joints);

			//imwrite(
			//	"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/joints2.jpg",
			//	joints);

			Point min, max;
			findImageBoundaries(joints, min, max);
			joints = Mat(joints, Rect(min, max));

			//imwrite(
			//	"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/joints3.jpg",
			//	joints);

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

			//change of method
			//for (int i = 0; i < lines.size(); ++i) {

			//METHOD CHANGE
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

			if (hasRectangularContour(joints) && countCorners(joints) > 4 && isGrid(joints)) {
				iter->isTable = true;

				//removing signs outside of the table
				int tempLeftX = iter->left.x;
				int tempUpY = iter->up.y;
				iter->left = Point(tempLeftX + min.x, tempUpY + min.y);
				iter->right = Point(tempLeftX + max.x, tempUpY + min.y);
				iter->up = Point(tempLeftX + min.x, tempUpY + min.y);
				iter->down = Point(tempLeftX + min.x, tempUpY + max.y);
				//method change
				//const double THRESHOLD = 0.02;

				//CHANGE OF CONCEPTION
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

	if (I.dims != 2)
		return false;

	if (I.at<uchar>(0, 0) != 255)
		return false;
	if (I.cols > 0)
		if(I.at<uchar>(0, I.cols - 1) != 255)
			return false;
	if (I.rows > 0)
		if (I.at<uchar>(I.rows - 1, 0) != 255)
			return false;
	if (I.rows > 0 && I.cols > 0)
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

void DrawingLocatorByConnComps::findImageBoundaries(Mat& Img, Point& min, Point& max){

	int minX, minY, maxX, maxY;
	minY = Img.rows + 1;
	minX = Img.cols + 1;
	maxY = 0;
	maxX = 0;

	for (int i = 0; i < Img.rows; ++i) {
		for (int j = 0; j < Img.cols; ++j) {
			if (Img.at<uchar>(i, j) == 255) {
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

	min = Point(minX, minY);
	max = Point(maxX == 0 ? 0 : maxX + 1, maxY == 0 ? 0 : maxY + 1);
}

void DrawingLocatorByConnComps::reduceJointsToPoints(Mat& joints) {

	//convert to black on white image
	Mat subMat = Mat::ones(joints.size(), joints.type()) * 255;
	subtract(subMat, joints, joints);

	for (int i = 0; i < joints.rows; ++i) {
		for (int j = 0; j < joints.cols; ++j) {
			if (joints.at<uchar>(i, j) == 0) {

				list<Point> neighbours;
				getNeighbouringPixels(Point(j, i), joints, neighbours);

				Point min = findHighestPoint(neighbours);

				joints.at<uchar>(min.y, min.x) = 0;
			}
		}
	}
		
	//reconvert to white on black image
	subtract(subMat, joints, joints);
}

Point DrawingLocatorByConnComps::findHighestPoint(const list<Point>& points) {

	Point min(-1, -1);
	for (list<Point>::const_iterator iter = points.begin(); iter != points.end(); iter++) {
		if (min.x == -1 ||
			iter->y < min.y ||
			(iter->y == min.y && iter->x < min.x))
			
				min = *iter;
	}

	return min;
}

void DrawingLocatorByConnComps::finalChecks() {

	if (connectedComponents.size() > 0) {

		//list<ConnectedComponent> compsFinalList;

		//check if any component does not lie inside another one
		for (list<ConnectedComponent>::iterator iter = connectedComponents.begin();
			iter != connectedComponents.end(); iter++) {

			for (list<ConnectedComponent>::iterator iterIn = connectedComponents.begin();
				iterIn != connectedComponents.end(); iterIn++) {

				if (iterIn->compare(*iter))
					continue;

				//if left up corner is in another component
				if (((iterIn->left.x >= iter->left.x &&
					iterIn->left.x <= iter->right.x &&
					iterIn->up.y >= iter->up.y &&
					iterIn->up.y <= iter->down.y)
					||
					//if right up corner is in another component
					(iterIn->right.x >= iter->left.x &&
						iterIn->right.x <= iter->right.x &&
						iterIn->up.y >= iter->up.y &&
						iterIn->up.y <= iter->down.y)
					||
					//if left down corner is in another component
					(iterIn->left.x >= iter->left.x &&
						iterIn->left.x <= iter->right.x &&
						iterIn->down.y >= iter->up.y &&
						iterIn->down.y <= iter->down.y)
					||
					//if right down corner is in another component
					(iterIn->right.x >= iter->left.x &&
						iterIn->right.x <= iter->right.x &&
						iterIn->down.y >= iter->up.y &&
						iterIn->down.y <= iter->down.y)
					))
				{
					//change dimensions of larger component if necessary
					if (iterIn->right.x > iter->right.x)
						iter->right.x = iterIn->right.x;
					if (iterIn->down.y > iter->down.y)
						iter->down.y = iterIn->down.y;
					if (iterIn->left.x < iter->left.x)
						iter->left.x = iterIn->left.x;
					if (iterIn->up.y < iter->up.y)
						iter->up.y = iterIn->up.y;

					//remove smaller component
					connectedComponents.erase(iterIn);
					iterIn = connectedComponents.begin();
				}
			}
		}
	}
}








