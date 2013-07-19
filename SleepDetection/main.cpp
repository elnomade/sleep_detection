#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <iostream>
#include <string>

class DetectClosestFace {
public:
	DetectClosestFace( void )
		: m_faceCascade(),
		m_eyesCascade()
	{
		if (!m_faceCascade.load("haarcascade_frontalface_alt.xml"))
		{
			throw std::runtime_error("Could not load haarcascade_frontalface_alt.xml file");
		}

		if (!m_eyesCascade.load("haarcascade_eye_tree_eyeglasses.xml"))
		{
			throw std::runtime_error("Could not load haarcascade_eye_tree_eyeglasses.xml file");
		}
	}

	void operator()( cv::Mat & src )
	{
		cv::Mat gray;
		cv::cvtColor(src, gray, CV_BGR2GRAY);
		cv::equalizeHist(gray, gray);

		typedef std::vector<cv::Rect> Faces;

		Faces faces;
		m_faceCascade.detectMultiScale(gray, faces, 1.1, 3, CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(30, 30));

		unsigned int alarmLevel = 0;

		std::for_each(
			faces.cbegin(),
			faces.cend(),
			[&](const Faces::value_type & face)
		{
			++alarmLevel;

			cv::ellipse(
				src,
				cv::Point(face.x + face.width*0.5, face.y + face.height*0.5),
				cv::Size(face.width*0.5, face.height*0.5),
				0,
				0,
				360,
				cv::Scalar(0, 255, 127),
				4,
				8,
				0);

			const cv::Mat && faceROI = gray(face);

			typedef std::vector<cv::Rect> Eyes;

			Eyes eyes;
			m_eyesCascade.detectMultiScale(faceROI, eyes, 1.1, 2, CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));

			alarmLevel += eyes.size() > 1 ? 1 : 0;

			std::for_each(
				eyes.cbegin(),
				eyes.cend(),
				[&](const Eyes::value_type & eye)
			{
				cv::circle(
					src,
					cv::Point((face.x + eye.x + eye.width*0.5), (face.y + eye.y + eye.height*0.5)),
					cvRound((eye.width + eye.height) * 0.25),
					cv::Scalar(127, 0, 127),
					4,
					8,
					0);
			});
		});

		cv::Scalar alarmColors[] = {
			cv::Scalar(  0.0,   0.0, 255.0), // RED		-	Detected nothing
			cv::Scalar(  0.0, 255.0, 255.0), // YELLOW	-	Detected face and one or more eyes
			cv::Scalar(  0.0, 255.0,   0.0), // GREEN	-	Detected face and both eyes
		};

		cv::rectangle(
			src,
			cv::Rect(0, 0, src.size().width, src.size().height),
			alarmColors[alarmLevel],
			10);
	}

private:
	cv::CascadeClassifier m_faceCascade;

	cv::CascadeClassifier m_eyesCascade;
};

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
	CaptureWebCam(DetectClosestFace(), "Detecting closest face");
}
