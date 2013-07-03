#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <string>

class DetectFaces {
public:
	DetectFaces( void )
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

		typedef std::vector<cv::Rect> Faces, Eyes;

		Faces faces;
		m_faceCascade.detectMultiScale(gray, faces, 1.1, 3, CV_HAAR_SCALE_IMAGE/*|CV_HAAR_FIND_BIGGEST_OBJECT*/, cv::Size(30, 30));

		std::for_each(
			faces.cbegin(),
			faces.cend(),
			[&](const Faces::value_type & face)
		{
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

			const auto && faceROI = gray(face);

			Eyes eyes;
			m_eyesCascade.detectMultiScale(faceROI, eyes, 1.1, 2, CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));

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
	}
private:
	cv::CascadeClassifier m_faceCascade;
	cv::CascadeClassifier m_eyesCascade;
};

template <class Process>
void CaptureWebCam( Process && proc, const std::string & name )
{
	cv::namedWindow(name, CV_WINDOW_AUTOSIZE);

	cv::VideoCapture video(0);

	cv::Mat image;
	for (;;)
	{
		video >> image;

		proc(image);

		imshow(name, image);

		if (cv::waitKey(33) == 'q') break;
	}

	video.release();

	cv::destroyAllWindows();
}

int main( int, char *[] )
{
	CaptureWebCam(DetectFaces(), "Detecting faces");
}
