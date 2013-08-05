#pragma once

#ifndef FaceDetector_h__
#define FaceDetector_h__

#include <opencv2/objdetect/objdetect.hpp>

#include <memory>

class SleepDetector
{
public:
	explicit SleepDetector( void );

	void operator()( cv::Mat & src );

public:
	struct IAwareness {
		virtual ~IAwareness( void ) {}

		virtual void onBlink( cv::Mat & src ) = 0;

		virtual void onStaring( cv::Mat & src ) = 0;

		virtual void onNotLooking( cv::Mat & src ) = 0;
	};

private:
	cv::CascadeClassifier m_faceCascade;

	cv::CascadeClassifier m_eyesCascade;

	std::unique_ptr<IAwareness> m_awareness;
};

#endif // FaceDetector_h__
