#include "ConnectedComponent.h"


ConnectedComponent::ConnectedComponent(Point l, Point r, Point u, Point d, int blacks) :
		left(l), right(r), up(u), down(d), blackPixels(blacks){}

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

		//Mat compRect(I, Rect(Point(left.x, down.y), Point(right.x, up.y)));
		//vector<Vec2f> lines;
		//HoughLines(compRect, lines, 1, CV_PI / 180, 180, 0, 0);
		//lineQnt = lines.size();
		////TODO isTable should depend on line rotation
		//if (lineQnt > 0)
		//	isTable = false;
		//else
		//	isTable = true;
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