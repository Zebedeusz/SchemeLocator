#include "Main.h"


int main() {
	string imgPathTable = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2002 Music Simi_2.jpg";
	string imgPathDividedTable = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2010_ISMIR_MUSIC_EMOTION RECOGNITION.jpg";

	string imagesPath = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/";
	string imgSavePath = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/";

	string imghq = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/DoKlas/Reszta/2010 Architecture for Automated Tagging and Clustering of Song Files2010 Architecture for Automated Tagging and Clustering of Song Files-5.jpg";



	//namedWindow("image", WINDOW_AUTOSIZE);
	//resizeWindow("image", 800, 600);

	string imgsForTest[] = {
		//"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2002 Content-ba_5.jpg",
		//wywala siê podczas szukania komp
		//"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2003 Algorithmi_5.jpg",
		//mo¿e nieadekwatny
		//"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2009 Music Clus_6.jpg",
		//error z at
		//"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2003 Algorithmi_3.jpg",
		//"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2002 Content-ba.jpg",
		//"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2002 Content-ba_1.jpg",
		//"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2002 Content-ba_2.jpg",
		//"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2002 Content-ba_3.jpg",
		"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2002 Music Simi_2.jpg",
		"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2002 Self-Organ.jpg",
		"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2002 Self-Organ_2.jpg",
		"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2006 Blind Clus_1.jpg",
		"C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/2010_ISMIR_MUSI_6.jpg",
	};

	//for (auto & p : experimental::filesystem::directory_iterator(imagesPath)) {
	for (string & s : imgsForTest) {

		cout << "Loaded image located at: " << endl << s << endl;

		Mat imgGrey = imread(s, IMREAD_GRAYSCALE);

		//Mat img = imread(imgPathTable, IMREAD_COLOR);

		//Mat imgGrey;
		//cvtColor(img, imgGrey, COLOR_BGR2GRAY);

		Mat binarized;
		threshold(imgGrey, binarized, 0, 255, THRESH_OTSU);

		//Mat binarizedCOrrectedSkew;
		//correctSkew(binarized, binarizedCOrrectedSkew);

		//imwrite(
		//	imgSavePath + "binarized.jpg",
		//	binarized);

		DrawingLocatorByConnComps loc(binarized);
		loc.findSchemesAndTables();

		Mat outlinedSchemes;
		loc.outlineSchemesAndTables(outlinedSchemes);

		imwrite(
			imgSavePath + s.replace(0, s.rfind("/"), ""),
			outlinedSchemes);

		cout << "Image with outlined schemes and tables saved as: " << endl 
			<< imgSavePath + s.replace(0, s.rfind("/"), "") << endl << endl;

		//imwrite(
		//	imgSavePath + imgPathTable.replace(0, imghq.rfind("/"), ""),
		//	outlinedSchemes);
	}

	return 0;
}

void correctSkew(const Mat& img, Mat& imgRotated) {

	cout << "Skew correction in progress" << endl;

	vector<double> variances;

	for (int angle = -40; angle < 41; angle++) {

		rotateImg(img, imgRotated, angle);

		if (angle == 0)
			continue;

		vector<int> blackPixelsPerRow;

		for (int i = 0; i < img.rows; ++i)
			blackPixelsPerRow.push_back(0);

		for (int i = 0; i < imgRotated.rows; ++i) {
			for (int j = 0; j < imgRotated.cols; ++j) {

				if (imgRotated.at<uchar>(i, j) == 0)
					blackPixelsPerRow.at(i) += 1;
			}
		}

		double var = variance(blackPixelsPerRow);

		variances.push_back(var);
	}

	//double maxVar = -1;
	//int maxIndex = -1;

	//for (int i = 0; i < variances.size(); ++i) {
	//	if (maxVar < variances.at(i)) {
	//		maxVar = variances.at(i);
	//		maxIndex = i;
	//	}
	//}

	//int angle = maxIndex - 40;

	double minVar = variances.at(0);
	int minIndex = 0;

	for (int i = 1; i < variances.size(); ++i) {
		if (minVar > variances.at(i)) {
			minVar = variances.at(i);
			minIndex = i;
		}
	}

	int angle = minIndex - 40;
	rotateImg(img, imgRotated, angle);

	cout << "Image rotated by " << angle << " degress" << endl;
}

double variance(const vector<int>& vec) {
	double sum = 0;
	for (auto& e : vec)
		sum += e;

	double mean = sum / vec.size();

	double sumOfVarianceParts = 0;
	for (auto& e : vec)
		sumOfVarianceParts += pow(e - mean, 2);

	double variance = sumOfVarianceParts / vec.size();

	return variance;
}

void rotateImg(const Mat& src, Mat& dst, double angle)
{
	Point2f pt(src.cols / 2., src.rows / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);
	warpAffine(src, dst, r, Size(src.cols, src.rows));
}