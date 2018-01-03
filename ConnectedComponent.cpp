#include "ConnectedComponent.h"


ConnectedComponent::ConnectedComponent(const Point& l, const Point& r, const Point& u, const Point& d, const bool& table) {

	left = l; right = r; up = u; down = d;
	blackPixels = -1;
	isTable = table;
	area = (down.y - up.y) * (right.x - left.x);
	centroid = Point(0, 0);
}

ConnectedComponent::ConnectedComponent(list<Point> neighbours, const Mat& I) {
		blackPixels = neighbours.size();

		left = Point(I.cols - 1, 0); right = Point(0, 0); up = Point(0, I.rows - 1); down = Point(0, 0);

		for (list<Point>::iterator iter = neighbours.begin(); iter != neighbours.end(); iter++) {
			if (iter->x <= left.x)
				left = Point(iter->x, iter->y);
			if (iter->x >= right.x)
				right = Point(iter->x, iter->y);
			if (iter->y <= up.y)
				up = Point(iter->x, iter->y);
			if (iter->y >= down.y)
				down = Point(iter->x, iter->y);
		}

		area = (down.y - up.y) * (right.x - left.x);

		centroid = Point(0, 0);

		isTable = false;
	}


	double ConnectedComponent::euclideanDistance(const Point& a, const Point& b) {
		return sqrt(pow(a.y - b.y, 2) + pow(a.x - b.x, 2));
	}

	//double ConnectedComponent::longestDiagonal(const list<Point>& points) {

	//	double diag = 0;

	//	Point mainPoint = points.front();

	//	list<Point>::const_iterator iter = points.begin();
	//	iter++;
	//	for (; iter != points.end(); iter++) {
	//		double tempDiag = euclideanDistance(mainPoint, *iter);
	//		if (tempDiag > diag)
	//			diag = tempDiag;
	//	}

	//	if ((int)points.size() > 2) {
	//		list<Point> ps;
	//		ps.insert(ps.end(), ++points.begin(), points.end());
	//		double tempDiag = longestDiagonal(ps);
	//		if (tempDiag > diag)
	//			diag = tempDiag;
	//	}

	//	return diag;
	//}

	bool ConnectedComponent::compare(const ConnectedComponent& connComp) {

		return left == connComp.left && right == connComp.right && up == connComp.up && down == connComp.down
			&& blackPixels == connComp.blackPixels && isTable == connComp.isTable
			&& area == connComp.area && centroid == connComp.centroid;
	}

	bool ConnectedComponent::operator==(const ConnectedComponent& connComp) const {

		return left.x == connComp.left.x && 
			right.x == connComp.right.x &&
			up.y == connComp.up.y && 
			down.y == connComp.down.y
			&& blackPixels == connComp.blackPixels && isTable == connComp.isTable
			&& area == connComp.area && centroid == connComp.centroid;
	}

	bool ConnectedComponent::compareComponents(const ConnectedComponent& connComp1, const ConnectedComponent& connComp2) {

		return connComp1.area > connComp2.area;
	}

	const Point& ConnectedComponent::calculateCentroid(const Mat& I) {

		Mat compRect(I, Rect(Point(left.x, down.y), Point(right.x, up.y)));
		Mat subMat = Mat::ones(compRect.size(), compRect.type()) * 255;
		subtract(subMat, compRect, compRect);

		Moments mms = moments(compRect, true);

		subtract(subMat, compRect, compRect);

		centroid = Point(left.x + cvRound(mms.m10 / mms.m00), up.y + cvRound(mms.m01 / mms.m00));

		return centroid;
	}