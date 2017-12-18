#include "stdafx.h"
#include "DrawingLocatorByKeyDesc.h"

void DrawingLocatorByKeyDesc::outlineSchemesAndTables(Mat& img) {

	Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
	//cv::Ptr<Feature2D> f2d = xfeatures2d::SURF::create();

	vector<KeyPoint> keypoints;
	f2d->detect(img, keypoints);

	Mat descriptors;
	f2d->compute(img, keypoints, descriptors);
	cout << "Descriptors computed" << endl;

	Mat testData(descriptors.rows, 6 + descriptors.cols, CV_32FC1);
	cout << "Test data initialized" << endl;

	int i = 0;
	for (auto & keyPoint : keypoints) {
		testData.row(i).col(0) = keyPoint.pt.x;
		testData.row(i).col(1) = keyPoint.pt.y;
		testData.row(i).col(2) = keyPoint.size;
		testData.row(i).col(3) = keyPoint.angle;
		testData.row(i).col(4) = keyPoint.response;
		testData.row(i).col(5) = keyPoint.octave;

		int k = 6;
		for (int j = 0; j < descriptors.cols; ++j) {
			testData.row(i).col(k) = descriptors.row(i).col(j);
			k++;
		}
		i++;
	}

	cout << "testData rows: " << testData.rows << endl;
	cout << "testData cols: " << testData.cols << endl;

	Mat testResults;

	svm->predict(testData, testResults);

	cout << "SVM prediction done" << endl;

	string siftCsvs = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/DoKlas/CSV/SIFT/";

	ofstream myfile;
	myfile.open(siftCsvs + "predicted.csv");
	myfile << cv::format(testResults, cv::Formatter::FMT_CSV) << std::endl;
	myfile.close();
}

DrawingLocatorByKeyDesc::DrawingLocatorByKeyDesc() {
	svm = ml::SVM::create();

	svm->setType(ml::SVM::C_SVC);
	svm->setKernel(ml::SVM::RBF);
	svm->setC(12.5);
	svm->setGamma(0.50625);
}

void DrawingLocatorByKeyDesc::saveKeyDescsToCsv(const string& imgsPath, const string& keysCsvPath, const char& classId) {
	ofstream csvFile;
	csvFile.open(keysCsvPath, ios_base::app);

	for (auto & p : experimental::filesystem::directory_iterator(imgsPath)) {

		Mat img = imread(p.path().generic_string(), IMREAD_COLOR);
		Mat imgGrey;
		cvtColor(img, imgGrey, COLOR_BGR2GRAY);
	
		Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
		//cv::Ptr<Feature2D> f2d = xfeatures2d::SURF::create();

		vector<KeyPoint> keypoints;
		f2d->detect(imgGrey, keypoints);

		Mat descriptors;
		f2d->compute(imgGrey, keypoints, descriptors);

		int i = 0;
		for (auto & keyPoint : keypoints) {
			csvFile << keyPoint.pt.x << ",";
			csvFile << keyPoint.pt.y << ",";
			csvFile << keyPoint.size << ",";
			csvFile << keyPoint.angle << ",";
			csvFile << keyPoint.response << ",";
			csvFile << keyPoint.octave << ",";
			csvFile << format(descriptors.row(i), Formatter::FMT_CSV) << ",";
			csvFile << classId << "\n";
			i++;
		}
	}
	csvFile.close();
}



void DrawingLocatorByKeyDesc::train(const string& keysCsvPath, const string& classifierSavePath) {

	//vector<vector<string> > data;

	//for (auto & p : experimental::filesystem::directory_iterator(keysCsvPath)) {
	//	ifstream csvFile;
	//	csvFile.open(p.path().generic_string());

	//	std::string line = "";
	//	while (getline(csvFile, line)){
	//		vector<string> vec;
	//		boost::algorithm::split(vec, line, boost::is_any_of(";"));
	//		data.push_back(vec);
	//	}
	//	cout << data.size();
	//}

	Ptr<ml::TrainData> data = ml::TrainData::loadFromCSV(keysCsvPath, 0, -1, -1, String(), ',');

	Mat trainData = data->getTrainSamples();
	Mat trainLabels = data->getTrainResponses();
	trainLabels.convertTo(trainLabels, CV_32S); // needed for SVM

	cout << "trainData rows: " << trainData.rows << endl;
	cout << "trainData cols: " << trainData.cols << endl;

	svm->train(trainData, ml::ROW_SAMPLE, trainLabels);

	cout << "SVM trained" << endl;

	svm->save(classifierSavePath + "model.yml");

	cout << "Model saved" << endl;
}

void DrawingLocatorByKeyDesc::loadPretrainedModel(const string& classifierLoadPath) {
	svm->load(classifierLoadPath);
}
