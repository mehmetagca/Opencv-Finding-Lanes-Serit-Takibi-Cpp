#include <iostream>
#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

Mat whiteAndYellow(Mat img)
{
	Mat white, yellow, imgHls, mask, whiteYellow;
	
	cvtColor(img, imgHls, COLOR_BGR2HLS);
	//imshow("hls", imgHls);
	//waitKey();
	
	//white
	inRange(imgHls, Scalar(0, 200, 0) , Scalar(255, 255, 255) , white);

	//yellow
	inRange(imgHls, Scalar(10, 0, 100), Scalar(40, 255, 255), yellow);
	//imshow("yellow", yellow);
	//waitKey();

	bitwise_or(white, yellow, mask);

	bitwise_and(img, img, whiteYellow, mask);
	return whiteYellow;
}

Mat cannyImg(Mat img)
{
	Mat gray;
	cvtColor(img, gray, COLOR_BGR2GRAY);

	Mat blur;
	GaussianBlur(gray, blur, Size(15, 15), 0);

	Mat canny;
	Canny(blur, canny, 50, 150);

	return canny;
}

Mat regionOfInterest(Mat img)
{
	int cols = img.cols;
	int rows = img.rows;

	int x1 = cols * 0.1, y1 = rows * 0.95;
	int x2 = cols * 0.4, y2 = rows * 0.6;
	int x3 = cols * 0.6, y3 = rows * 0.6;
	int x4 = cols * 0.9, y4 = rows * 0.95;

	Mat mask = img.zeros(Size(cols, rows), CV_8UC1);
	//Mat mask(Size(cols, rows), CV_8UC1, Scalar(0));

	Point points[4] = {
	Point(x1, y1),
	Point(x2, y2),
	Point(x3, y3),
	Point(x4, y4) };

	fillConvexPoly(mask, points, 4, Scalar(255));

	bitwise_and(img, mask, mask);

	//imshow("poly", mask);
	//waitKey();

	return mask;
}

Mat houghLine(Mat imgOrg, Mat cannyImg)
{
	vector<Vec4i> lines; //vector<cv::Vec<int, 4>> houghlines
	HoughLinesP(cannyImg, lines, 1, CV_PI / 180, 40, 50, 100);//Treshold:100 - MinlineLength:40 - MaxlineGap:10 -100,10,50 -50,40,10
															  //Noktadaki Kesiþim - Min Çizgi Uzunluðu - 2 Çizgi Arasý Max Boþluk
	for (int i = 0; i<lines.size(); i++)
	{
		Point a = Point(lines[i][0], lines[i][1]);
		Point b = Point(lines[i][2], lines[i][3]);
		line(imgOrg, a, b, Scalar(0, 0, 255));
	}

	return imgOrg;
}

int main()
{
	Mat img = imread("test.jpg");

	if (img.empty())
	{
		cout << "Resim yuklenemedi!";
		system("Pause");
		return 0;
	}

	imshow("orjinal", img);
	waitKey();

	//Mat imgHls;
	//cvtColor(img, imgHls, COLOR_BGR2HLS);
	//cvtColor(imgHls, imgHls, COLOR_BGR2HSV);
	//imshow("test", imgHls);
	//waitKey();



	Mat whiteYellow = whiteAndYellow(img);

	Mat canny = cannyImg(whiteYellow);

	Mat cropped = regionOfInterest(canny);

	Mat linesImg = houghLine(img, cropped);

	//imshow("test", linesImg);
	//waitKey();

	//Video------------------------------------------------
	VideoCapture video;
	Mat videoFrame;

	if (!video.open("video.mp4"))
	{
		cout << "Video acilamadi!";
		system("Pause");
		return 0;
	}

	while (video.read(videoFrame))
	{
		//imshow("video", videoFrame);
		//waitKey();
		
		whiteYellow = whiteAndYellow(videoFrame);

		canny = cannyImg(whiteYellow);

		cropped = regionOfInterest(canny);

		//imshow("test", cropped);
		//waitKey();

		linesImg = houghLine(videoFrame, cropped);

		imshow("video", linesImg);

		if (waitKey(1) != -1)
			break;
	}

	video.release();
	destroyAllWindows();
	return 0;
}