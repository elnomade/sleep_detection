#include "SleepDetector.h"

#include <opencv2/imgproc/imgproc.hpp>

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>

#include <chrono>

namespace
{
	using namespace boost::msm::front;
	using namespace boost::msm::back;

	using namespace std::chrono;

	class Awareness : public SleepDetector::IAwareness {
	public:
		/* States */
		struct Asleep : public state<> {};

		struct Sleepy : public state<> {};

		struct Awake : public state<> {};

		/* Events */
		struct Blinking {
			Blinking( cv::Mat & src )
				: m_src(src),
				m_blink(steady_clock::now()) {}

			cv::Mat & m_src;

			steady_clock::time_point m_blink;
		};

		struct Staring {
			Staring( cv::Mat & src )
				: m_src(src) {}

			cv::Mat & m_src;
		};

		struct NotLooking {
			NotLooking( cv::Mat & src )
				: m_src(src) {}

			cv::Mat & m_src;
		};

		/* Actions */
		struct DrawRed {
			template <class EVT, class FSM, class SourceState, class TargetState>
			void operator()( EVT const & evt, FSM &, SourceState &, TargetState & )
			{
				cv::rectangle(
					evt.m_src,
					cv::Rect(0, 0, evt.m_src.size().width, evt.m_src.size().height),
					cv::Scalar( 0.0, 0.0, 255.0),
					10);
			}
		};

		struct DrawYellow {
			template <class EVT, class FSM, class SourceState, class TargetState>
			void operator()( EVT const & evt, FSM &, SourceState &, TargetState & )
			{
				cv::rectangle(
					evt.m_src,
					cv::Rect(0, 0, evt.m_src.size().width, evt.m_src.size().height),
					cv::Scalar( 0.0, 255.0, 255.0),
					10);
			}
		};

		struct DrawGreen {
			template <class EVT, class FSM, class SourceState, class TargetState>
			void operator()( EVT const & evt, FSM &, SourceState &, TargetState & )
			{
				cv::rectangle(
					evt.m_src,
					cv::Rect(0, 0, evt.m_src.size().width, evt.m_src.size().height),
					cv::Scalar( 0.0, 255.0, 0.0),
					10);
			}
		};

		/* Guards */
		template <seconds::rep meanBlinkTime>
		struct AtLeast {
			template <class EVT, class FSM, class SourceState, class TargetState>
			bool operator()( EVT const & evt, FSM & fsm, SourceState &, TargetState & )
			{
				const auto timeBetweenBlinks = duration_cast<seconds>(evt.m_blink - fsm.m_lastBlink).count();

				fsm.m_lastBlink = evt.m_blink;

				if (timeBetweenBlinks < meanBlinkTime)
				{
					return true;
				}
				else
				{
					fsm.process_event(Staring(evt.m_src));

					return false;
				}
			}
		};

		struct SleepTransitions : public state_machine_def<SleepTransitions>
		{
			typedef boost::mpl::vector<
				//  < Start  | Event      | Next   | Action     | Guard          >
				Row < Awake  , Staring    , Awake  , DrawGreen  , none           >,
				Row < Awake  , Blinking   , Sleepy , DrawYellow , AtLeast<60/40> >,
				Row < Awake  , NotLooking , Asleep , DrawRed    , none           >,
				//  <                                                            >
				Row < Sleepy , Blinking   , Sleepy , DrawYellow , none           >,
				Row < Sleepy , Staring    , Awake  , DrawGreen  , none           >,
				Row < Sleepy , NotLooking , Asleep , DrawRed    , none           >,
				//  <                                                            >
				Row < Asleep , NotLooking , Asleep , DrawRed    , none           >,
				Row < Asleep , Staring    , Awake  , DrawGreen  , none           >,
				Row < Asleep , Blinking   , Sleepy , DrawYellow , none           >
			> transition_table;

			typedef Awake initial_state;

			steady_clock::time_point m_lastBlink;

			SleepTransitions( void ) : m_lastBlink(steady_clock::now()) {}
		};

		typedef state_machine<SleepTransitions> SM;

		Awareness( void )
			: m_machine()
		{
			m_machine.start();
		}

		virtual void onBlink( cv::Mat & src ) override
		{
			m_machine.process_event(Blinking(src));
		}

		virtual void onStaring( cv::Mat & src ) override
		{
			m_machine.process_event(Staring(src));
		}

		virtual void onNotLooking( cv::Mat & src ) override
		{
			m_machine.process_event(NotLooking(src));
		}

	private:
		SM m_machine;
	};
}

SleepDetector::SleepDetector(void)
	: m_faceCascade(),
	m_eyesCascade(),
	m_awareness(new Awareness())
{
	const std::string frontalface("haarcascade_frontalface_alt.xml");
	if (!m_faceCascade.load(frontalface))
	{
		throw std::runtime_error("Could not load " + frontalface + " file");
	}

	const std::string eyeglasses("haarcascade_eye_tree_eyeglasses.xml");
	if (!m_eyesCascade.load(eyeglasses))
	{
		throw std::runtime_error("Could not load " + eyeglasses + "file");
	}
}

SleepDetector::~SleepDetector(void)
{
}

void SleepDetector::operator()( cv::Mat & src )
{
	cv::Mat gray;
	cv::cvtColor(src, gray, CV_BGR2GRAY);
	cv::equalizeHist(gray, gray);

	typedef std::vector<cv::Rect> Faces;

	Faces faces;
	m_faceCascade.detectMultiScale(gray, faces, 1.1, 3, CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(30, 30));

	unsigned int subjectiveAlertness = 0; // start out with the lowest alertness.

	if (!faces.empty())
	{
		subjectiveAlertness += 1;

		const cv::Mat && faceROI = gray(faces[0]);

		typedef std::vector<cv::Rect> Eyes;
		Eyes eyes;
		m_eyesCascade.detectMultiScale(faceROI, eyes, 1.1, 2, CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));

		subjectiveAlertness += eyes.size() > 1 ? 1 : 0;
	}

// 	std::for_each(
// 		faces.cbegin(),
// 		faces.cend(),
// 		[&](const Faces::value_type & face)
// 	{
// 		++subjectiveAlertness;
// 
// 		cv::ellipse(
// 			src,
// 			cv::Point(face.x + face.width*0.5, face.y + face.height*0.5),
// 			cv::Size(face.width*0.5, face.height*0.5),
// 			0,
// 			0,
// 			360,
// 			cv::Scalar(0, 255, 127),
// 			4,
// 			8,
// 			0);
// 
// 		const cv::Mat && faceROI = gray(face);
// 
// 		typedef std::vector<cv::Rect> Eyes;
// 
// 		Eyes eyes;
// 		m_eyesCascade.detectMultiScale(faceROI, eyes, 1.1, 2, CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));
// 
// 		subjectiveAlertness += eyes.size() > 1 ? 1 : 0;
// 
// 		std::for_each(
// 			eyes.cbegin(),
// 			eyes.cend(),
// 			[&](const Eyes::value_type & eye)
// 		{
// 			cv::circle(
// 				src,
// 				cv::Point((face.x + eye.x + eye.width*0.5), (face.y + eye.y + eye.height*0.5)),
// 				cvRound((eye.width + eye.height) * 0.25),
// 				cv::Scalar(127, 0, 127),
// 				4,
// 				8,
// 				0);
// 		});
// 	});

	switch (subjectiveAlertness)
	{
	case 2:
		m_awareness->onStaring(src);
		break;

	case 1:
		m_awareness->onBlink(src);
		break;

	default:
		m_awareness->onNotLooking(src);
		break;
	}
}
