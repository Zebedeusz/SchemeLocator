#include "Main.h"

int main(int argc, char* argv[]) 
{
	string imgPath, imgSavePath;

	if (argc < 2 || argc > 3) 
	{
		cerr << endl << "Usage: " << argv[0] << " jpg_file_location [analysed_file_save_location]" << endl;
		return 1;
	}
	else if (argc == 2)
	{
		imgPath = argv[1];
		imgSavePath = imgPath.replace(imgPath.rfind("."), imgPath.length(), "") + "_analysed.jpg";
	}
	else if (argc == 3)
	{
		imgPath = argv[1];
		imgSavePath = argv[2];
	}

	if (imgPath.find(".") > imgPath.length() || imgPath.substr(imgPath.rfind("."), imgPath.length()).compare(".jpg") != 0)
		imgPath += ".jpg";

	cout << endl << "Loaded image located at: " << endl << imgPath << endl;

	Mat imgGrey = imread(imgPath, IMREAD_GRAYSCALE);

	Mat binarized;
	threshold(imgGrey, binarized, 0, 255, THRESH_OTSU);

	Mat binarizedCorrectedSkew;
	correctSkew(binarized, binarizedCorrectedSkew);

	DrawingLocatorByConnComps loc(binarized);
	loc.findSchemesAndTables();

	Mat outlinedSchemes;
	loc.outlineSchemesAndTables(outlinedSchemes);

	imwrite(
		imgSavePath, outlinedSchemes);

	cout << "Image with outlined schemes and tables saved as: " << endl
		<< imgSavePath << endl << endl;

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

	double maxVar = -1;
	int maxIndex = -1;

	for (int i = 0; i < variances.size(); ++i) {
		if (maxVar < variances.at(i)) {
			maxVar = variances.at(i);
			maxIndex = i;
		}
	}

	int angle = maxIndex - 40;
	if (angle != 0) 
	{
		rotateImg(img, imgRotated, angle);
		cout << "Image rotated by " << angle << " degrees" << endl;
	}
	else
		cout << "Image rotation was correct" << endl;
}

double variance(const vector<int>& vec) {
	double sum = 0;
	for (auto& e : vec)
		sum += e;

	double mean = sum / (double) vec.size();

	double sumOfVarianceParts = 0;
	for (auto& e : vec)
		sumOfVarianceParts += pow(e - mean, 2);

	double variance = sumOfVarianceParts / (double) vec.size();

	return variance;
}

void rotateImg(const Mat& src, Mat& dst, double angle)
{
	//convert to white on black image
	Mat tempSrc;
	Mat subMat = Mat::ones(src.size(), src.type()) * 255;
	subtract(subMat, src, tempSrc);

	Point2f pt(tempSrc.cols / 2., tempSrc.rows / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);
	warpAffine(tempSrc, dst, r, Size(tempSrc.cols, tempSrc.rows));

	//reconvert to black on white image
	subtract(subMat, dst, dst);
}