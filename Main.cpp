#include "Main.h"


int main() {
	string imgPathTable = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/TreningoweObrazy/2011_MusicEmotionClassification_Thesis_047.png";
	string imgPathDrawing = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/TreningoweObrazy/2011_MusicEmotionClassification_Thesis_021.png";
	string imgPathDrawing2 = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/TreningoweObrazy/Learning Music Emotion Primitives via Supervised_3.png";
	string imgPathDoc = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/Testowe/2010 LEARNING FEATURES FROM MUSIC AUDIO WITH DEEP BELIEF_4.png";
	string imgSimplePath = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/TreningoweObrazy/1.png";

	string imagesPath = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/PoKonwersji/";
	string imgSavePath = "C:/Users/Micha³/Desktop/Systemy Wizyjne/Dane/obramowane/";

	string imghq = "C:/Users/Micha³/Desktop/Systemy Wizyjne/testa/PDFtoJPG.me-5 (2).jpg";
	//namedWindow("image", WINDOW_AUTOSIZE);
	//resizeWindow("image", 800, 600);

	for (auto & p : experimental::filesystem::directory_iterator(imagesPath)) {

		Mat img = imread(p.path().generic_string(), IMREAD_COLOR);

	//Mat img = imread(imghq, IMREAD_COLOR);

		Mat imgGrey;
		cvtColor(img, imgGrey, COLOR_BGR2GRAY);

		Mat binarized;
		threshold(imgGrey, binarized, 0, 255, THRESH_OTSU);

		DrawingLocatorByConnComps loc(binarized);
		loc.findConnectedComponents();

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


	return 0;
}