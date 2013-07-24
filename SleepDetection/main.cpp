#include "SleepDetector.h"

#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <string>

template <class Process>
void CaptureWebCam( Process && proc, const std::string & name = "" )
{
	cv::VideoCapture video(0);

	if (!video.isOpened())
	{
		std::cerr << "Could not open video device 0";

		return;
	}

	cv::Mat image;
	for (;;)
	{
		video >> image;

		if (image.empty())
		{
			std::cout << "No video input detected, exiting.";

			break;
		}

		proc(image);

		imshow(name, image);

		if (cv::waitKey(33) == 'q') break;
	}

	video.release();

	cv::destroyAllWindows();
}

int main( int, char *[] )
{
	CaptureWebCam(SleepDetector(), "Detecting closest face");
}
