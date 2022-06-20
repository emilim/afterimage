// Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <cmath>

using namespace cv;
using namespace std;

template <typename T>
int sgn(T val)
{
	return (T(0) < val) - (val < T(0));
}
Vec3b rgb2lms(Vec3b frame)
{
	int r = (int)frame[2];
	int g = (int)frame[1];
	int b = (int)frame[0];
	float l = (4.11935 * b) + (43.5161 * g) + (17.8824 * r);
	float m = (3.8671 * b) + (27.1554 * g) + (3.4556 * r);
	float s = (1.4670 * b) + (0.1843 * g) + (0.0299 * r);
	return Vec3b(l, m, s);
}
Vec3b rgb2hsv(Vec3b frame)
{
	int r = (int)(frame[2]);
	int g = (int)(frame[1]);
	int b = (int)(frame[0]);
	int maxR = max(max(r, g), b);
	int minR = min(min(r, g), b);

	float h;
	float s = maxR != minR ? (maxR - minR) / maxR : 0;
	float v = (r + g + b) / 3;
	if (maxR != minR)
	{
		if (maxR == r) // red
		{
			h = 60 * ((g - b) / (maxR - minR));
		}
		else if (maxR == g) // green
		{
			h = 60 * ((b - r) / (maxR - minR)) + 120;
		}
		else // blue
		{
			h = 60 * ((r - g) / (maxR - minR)) + 240;
		}
	}
	else
	{
		h = 0;
	}

	return Vec3b(h, s, v);
}

int main(int argc, char *argv[])
{
	/*
	int filter[width][height][3] = { 0 };
	int filteredImg[width][height][3] = { 0 };
	int integralImg[width][height][3] = { 0 };
	std::unique_ptr<int[][height][3]> filter{std::make_unique<int[][height][3]>(width)};
	std::unique_ptr<int[][height][3]> filteredImg{ std::make_unique<int[][height][3]>(width) };
	std::unique_ptr<int[][height][3]> integralImg{ std::make_unique<int[][height][3]>(width) };
	int sz[] = { width, height, 3 };
	Mat filter(3, sz, CV_64FC3, Scalar::all(255));
	Mat filteredImg(3, sz, CV_64FC3, Scalar::all(0));
	Mat integralImg(3, sz, CV_64FC3, Scalar::all(0));
	*/
	VideoCapture cap(0); //, CAP_DSHOW);

	double dWidth = cap.get(CAP_PROP_FRAME_WIDTH);
	double dHeight = cap.get(CAP_PROP_FRAME_HEIGHT);
	const int r = 1;
	const int width = dWidth / r;
	const int height = dHeight / r;
	double a = 2;	// stepness activation function
	double l = 0.1; // time constant
	int mode = 0;

	Mat filter(height, width, CV_32FC3, Scalar::all(255));
	Mat integralImg(height, width, CV_8UC3, Scalar::all(0));
	Mat filteredImg(height, width, CV_8UC3, Scalar::all(0));
	Mat hsvImage(height, width, CV_8UC3, Scalar::all(0));
	Mat lmsImage(height, width, CV_8UC3, Scalar::all(0));

	int lut[255];
	for (int i = 0; i < 255; i++)
	{
		float x = i / 255.0;
		x = (sgn(2 * x - 1) * pow(abs(2 * x - 1), a) + 1) / 2;
		lut[i] = 255.0 - (x * 255.0);
	}

	// if not success, exit program
	if (cap.isOpened() == false)
	{
		cout << "Cannot open the video camera" << endl;
		cin.get();
		return -1;
	}

	cap.set(CAP_PROP_CONVERT_RGB, true);
	cap.set(CAP_PROP_FRAME_WIDTH, width);
	cap.set(CAP_PROP_FRAME_HEIGHT, height);

	string window_name = "My Camera Feed";
	namedWindow(window_name);
	VideoWriter original("original.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(width, height));
	VideoWriter illusion("illusion.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(width, height));

	while (true)
	{
		Mat frame;
		bool bSuccess = cap.read(frame);
		cv::resize(frame, frame, Size(width, height));

		/*
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				for (int k = 0; k < 3; k++) {
					//double pxImg = frame.at<Vec3b>(j, i)[k];
					//double pxintegralImg = integralImg.at<Vec3f>(j, i)[k];
					//double pxFilter = filter.at<Vec3f>(j, i)[k];
					//double pxfilteredImg = filteredImg.at<Vec3f>(j, i)[k];
					//Vec3f rgbV = integralImg.at<Vec3f>(j, i);
					//float px = frame.at<Vec3b>(j, i)[k] / 255.0;

					//integralImg.at<Vec3f>(j, i)[k] = ((integralImg.at<Vec3f>(j, i)[k] + (l * px)) / (1 + l));

					//int maxR = max(max(rgbV[0], rgbV[1]), rgbV[2]);
					//int minR = min(min(rgbV[0], rgbV[1]), rgbV[2]);
					//float s = maxR != minR ? (maxR - minR) / maxR : 0;

					//filter.at<Vec3f>(j, i)[k] = (filter.at<Vec3f>(j, i)[k] + s * lut[int(integralImg.at<Vec3f>(j, i)[k] * s)]) / (1 + s);

					//filteredImg.at<Vec3f>(j, i)[k] = px * filter.at<Vec3f>(j, i)[k] / 255.0;

				}
			}
		}
		*/
		for (int r = 0; r < frame.rows; r++)
		{
			Vec3b *ptrFrame = frame.ptr<Vec3b>(r);
			Vec3b *ptrIntegralImg = integralImg.ptr<Vec3b>(r);
			Vec3f *ptrFilter = filter.ptr<Vec3f>(r);
			Vec3b *ptrFilteredImg = filteredImg.ptr<Vec3b>(r);
			Vec3b *ptrHSVImage = hsvImage.ptr<Vec3b>(r);
			Vec3b *ptrLMSImage = lmsImage.ptr<Vec3b>(r);

			for (int c = 0; c < frame.cols; c++)
			{
				ptrLMSImage[c] = rgb2lms(ptrFrame[c]);
				ptrHSVImage[c] = rgb2hsv(ptrFrame[c]);
				float factor, maxF;

				switch (mode)
				{
				case 0:
					for (int k = 0; k < 3; k++)
					{
						ptrIntegralImg[c][k] = ((ptrIntegralImg[c][k] + (l * ptrFrame[c][k])) / (1 + l));
						ptrFilter[c][k] = ((ptrFilter[c][k] + ptrHSVImage[c][1] * lut[int(ptrIntegralImg[c][k] * ptrHSVImage[c][1])]) / (1 + ptrHSVImage[c][1]));
						ptrFilteredImg[c][k] = ptrFrame[c][k] * ptrFilter[c][k] / 255.0;
					}

					maxF = max(max((float)ptrFilteredImg[c][0], (float)ptrFilteredImg[c][1]), (float)ptrFilteredImg[c][2]);
					factor = ptrHSVImage[c][2] / maxF;
					for (int k = 0; k < 3; k++)
					{
						ptrFilteredImg[c][k] = ptrFilteredImg[c][k] * factor;
					}
					break;
				case 1:
					for (int k = 0; k < 3; k++)
					{
						ptrIntegralImg[c][k] = ((ptrIntegralImg[c][k] + (l * ptrLMSImage[c][k])) / (1 + l));
						ptrFilter[c][k] = ((ptrFilter[c][k] + ptrHSVImage[c][1] * lut[int(ptrIntegralImg[c][k] * ptrHSVImage[c][1])]) / (1 + ptrHSVImage[c][1]));
						ptrFilteredImg[c][k] = ptrLMSImage[c][k] * ptrFilter[c][k] / 255.0;
					}

					maxF = max(max((float)ptrFilteredImg[c][0], (float)ptrFilteredImg[c][1]), (float)ptrFilteredImg[c][2]);
					factor = ptrHSVImage[c][2] / maxF;
					for (int k = 0; k < 3; k++)
					{
						ptrFilteredImg[c][k] = ptrFilteredImg[c][k] * factor;
					}
					break;
				}
			}
		}

		if (bSuccess == false)
		{
			cout << "Video camera is disconnected" << endl;
			cin.get();
			break;
		}

		Mat displayh, displayh2, display;
		hconcat(frame, filteredImg, displayh);
		// hconcat(filter, integralImg, displayh2);
		// vconcat(displayh, displayh2, display);
		imshow(window_name, filteredImg);
		original.write(frame);
		illusion.write(filteredImg);

		if (waitKey(10) == 27)
		{
			cout << "Esc key is pressed by user. Stoppig the video" << endl;
			break;
		}
	}

	cap.release();
	original.release();
	illusion.release();
	destroyAllWindows();
	return 0;
}