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

vector<Vec4i> houghLine(/*Mat imgOrg,*/ Mat cannyImg)
{
	vector<Vec4i> lines; //vector<cv::Vec<int, 4>> houghlines
	HoughLinesP(cannyImg, lines, 1, CV_PI / 180, 40, 50, 100);//Treshold:100 - MinlineLength:40 - MaxlineGap:10 -100,10,50 -50,40,10
															  //Noktadaki Kesiþim - Min Çizgi Uzunluðu - 2 Çizgi Arasý Max Boþluk
	/*for (int i = 0; i<lines.size(); i++)
	{
		Point a = Point(lines[i][0], lines[i][1]);
		Point b = Point(lines[i][2], lines[i][3]);
		line(imgOrg, a, b, Scalar(0, 0, 255));
	}*/

	return lines;//imgOrg;
}

vector<Vec2f> avarageSlope(vector<Vec4i> lines)
{
	float slope, intercept, length;
	Point a, b;
	vector<Vec2f> leftLanes, rightLanes;
	//vector<int> leftLengths, rightLengths;

	for (auto line : lines)//Line Split
	{
		a = Point(line[0], line[1]);
		b = Point(line[2], line[3]);

		if (a.x == b.x)//x1 == x2 continuous slope!
			continue;

		slope = (float) (b.y - a.y) / (b.x - a.x);
		intercept = a.y - (slope * a.x);
		//length = sqrt(pow((b.y - a.y), 2) + pow((b.x - a.x), 2));//sqrt((y2-y1)^2 + (x2-x1)^2)

		if (slope < 0)//left lane
		{
			leftLanes.push_back(Vec2f(slope, intercept));
			//leftLengths.push_back(length);
		}
		else//right lane
		{
			rightLanes.push_back(Vec2f(slope, intercept));
			//rightLengths.push_back(length);
		}
	}

	float sumSlope = 0, sumIntercept = 0;
	int i;

	for (i = 0; i < leftLanes.size(); i++)//Avarage Left Lane
	{
		sumSlope += leftLanes[i][0];
		sumIntercept += leftLanes[i][1];
	}
	Vec2f avgLeft = Vec2f(sumSlope / i, sumIntercept / i);

	sumSlope = sumIntercept = 0;
	for (i = 0; i < rightLanes.size(); i++)//Avarage Right Lane
	{
		sumSlope += rightLanes[i][0];
		sumIntercept += rightLanes[i][1];
	}
	Vec2f avgRight = Vec2f(sumSlope / i, sumIntercept / i);

	vector<Vec2f> avg;
	avg.push_back(avgLeft);
	avg.push_back(avgRight);
	
	return avg;
}

Vec4i slopeToPoint(int y1, int y2, Vec2f lineSlope)
{
	float slope = lineSlope[0];
	float intercept = lineSlope[1];

	int x1 = (int) ((y1 - intercept) / slope);//y=mx+b -> x = (y-b)/m
	int x2 = (int) ((y2 - intercept) / slope);//baþlangýçta x kýsayken yeni hesaplanan y deðerinde x in tahmini uzayacaðý koordinatlar hesaplanýyor.

	return { x1, y1, x2, y2 };
}

vector<Vec4i> avgLanes(Mat img, vector<Vec4i> lines)
{
	vector<Vec2f> lanes = avarageSlope(lines);
	
	int y1 = img.rows;
	int y2 = y1 * 0.6;

	Vec4i leftLine = slopeToPoint(y1, y2, lanes[0]);
	Vec4i rightLine = slopeToPoint(y1, y2, lanes[1]);

	return { leftLine, rightLine };
}

Mat drawLines(Mat img, vector<Vec4i> lines)
{
	Mat temp = img.zeros(img.rows, img.cols, img.type());

	for (int i = 0; i<lines.size(); i++)
	{
		Point a = Point(lines[i][0], lines[i][1]);
		Point b = Point(lines[i][2], lines[i][3]);
 		line(temp, a, b, Scalar(0, 0, 255), 10);
	}
	return temp;
}

int main()
{
	Mat img = imread("test2.jpg");

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

	vector<Vec4i> lines = houghLine(cropped);//lines

	//imshow("hough", drawLines(img, lines));
	//waitKey();

	vector<Vec4i> lanes = avgLanes(img, lines);//left lane and right lane

	Mat lanesImg = drawLines(img, lanes);

	Mat newImg;
	addWeighted(img, 1, lanesImg, 0.8, 0, newImg);

	imshow("test", newImg);
	waitKey();

	//Video------------------------------------------------
	VideoCapture video;
	
	Mat videoFrame;

	if (!video.open("video.mp4"))
	{
		cout << "Video acilamadi!";
		system("Pause");
		return 0;
	}
	video.read(videoFrame);

	cout << "FPS: " << video.get(CAP_PROP_POS_FRAMES)<<"*" << video.get(CAP_PROP_FOURCC) <<"*"<<Size(videoFrame.cols, videoFrame.rows) << endl;
	VideoWriter videoWrite("sonuc.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 20, Size(videoFrame.cols, videoFrame.rows));
	//VideoWriter::fourcc('M','J','P','G');
	while (video.read(videoFrame))
	{
		//imshow("video", videoFrame);
		//waitKey();
		
		whiteYellow = whiteAndYellow(videoFrame);

		canny = cannyImg(whiteYellow);

		cropped = regionOfInterest(canny);

		//imshow("test", cropped);
		//waitKey();

		lines = houghLine(cropped);
		lanes = avgLanes(videoFrame, lines);//left lane and right lane
		lanesImg = drawLines(videoFrame, lanes);
		addWeighted(videoFrame, 1, lanesImg, 0.8, 0, newImg);

		//imshow("video", newImg);

		videoWrite.write(newImg);

		if (waitKey(1) != -1)
			break;
	}

	video.release();
	destroyAllWindows();
	return 0;
}