#include "Main.h"


int main() {
	string imgPathTable = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/TreningoweObrazy/2011_MusicEmotionClassification_Thesis_047.png";
	string imgPathDrawing = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/TreningoweObrazy/2011_MusicEmotionClassification_Thesis_021.png";
	string imgPathDrawing2 = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/TreningoweObrazy/Learning Music Emotion Primitives via Supervised_3.png";
	string imgPathDoc = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/Testowe/2010 LEARNING FEATURES FROM MUSIC AUDIO WITH DEEP BELIEF_4.png";
	string imgSimplePath = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/TreningoweObrazy/1.png";

	string imagesPath = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/";
	string imgSavePath = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/";

	string imghq = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/DoKlas/Reszta/2010 Architecture for Automated Tagging and Clustering of Song Files2010 Architecture for Automated Tagging and Clustering of Song Files-5.jpg";

	string imgsText = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/DoKlas/Teksty/";
	string csvTextDescs = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/DoKlas/textDescs.csv";
	string imgsScheme = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/DoKlas/Schematy/";
	string csvSchemeDescs = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/DoKlas/schemeDescs.csv";
	string imgsTable = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/DoKlas/Tabele/";
	string csvTableDescs = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/DoKlas/tableDescs.csv";

	string siftCsvs = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/DoKlas/CSV/SIFT/";
	string allSiftCsvs = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/DoKlas/CSV/SIFT/allDescs.csv";

	string modelPath = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/DoKlas/CSV/SIFT/model.yml";


	//namedWindow("image", WINDOW_AUTOSIZE);
	//resizeWindow("image", 800, 600);

	for (auto & p : experimental::filesystem::directory_iterator(imagesPath)) {

		Mat img = imread(p.path().generic_string(), IMREAD_COLOR);

	//Mat img = imread(imghq, IMREAD_COLOR);

		Mat imgGrey;
		cvtColor(img, imgGrey, COLOR_BGR2GRAY);

		Mat binarized;
		threshold(imgGrey, binarized, 0, 255, THRESH_OTSU);

		//Mat binarizedCOrrectedSkew;
		//correctSkew(binarized, binarizedCOrrectedSkew);

		//imwrite(
		//	imgSavePath + "rotated.jpg", 
		//	binarized);

		DrawingLocatorByConnComps loc(binarized);
		loc.findSchemesAndTables();

		Mat outlinedSchemes;
		loc.outlineSchemesAndTables(outlinedSchemes);

		imwrite(
			imgSavePath + p.path().generic_string().replace(0, p.path().generic_string().rfind("/"), ""),
			outlinedSchemes);

		//imwrite(
		//	imgSavePath + imghq.replace(0, imghq.rfind("/"), ""),
		//	outlinedSchemes);

		//namedWindow("image", WINDOW_AUTOSIZE);
		////resizeWindow("image", 800, 600);
		//imshow("image", outlinedSchemes);
		////resizeWindow("image", 800, 600);
		//waitKey();
	}

	//DrawingLocatorByKeyDesc dr;
	////dr.saveKeyDescsToCsv(imgsText, allSiftCsvs, '0');
	////dr.saveKeyDescsToCsv(imgsScheme, allSiftCsvs, '1');
	////dr.saveKeyDescsToCsv(imgsTable, siftCsvs + "allDescs.csv", '2');
	//dr.train(allSiftCsvs, siftCsvs);
	////dr.loadPretrainedModel(modelPath);

	//Mat img = imread(imghq, IMREAD_COLOR);
	//cout << "Loaded img" << endl;

	//Mat imgGrey;
	//cvtColor(img, imgGrey, COLOR_BGR2GRAY);
	//cout << "Loaded img grey" << endl;

	//dr.outlineSchemesAndTables(imgGrey);

	//Mat img = imread(imghq, IMREAD_COLOR);

	//dr.outlineSchemesAndTables(img);

	return 0;
}

void correctSkew(const Mat& img, Mat& imgRotated) {

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