#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include "window.h"

#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <algorithm>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/face.hpp>

#define VIDEO_OFFSET_Y				46

#define ID_TOGGLE_CAPTURE_BUTTON	1
#define ID_LOAD_CASCADE_BUTTON		2
#define ID_CAMERA_INDEX_COMBOXBOX	3

#define TOGGLE_CAPTURE_BUTTON_START	"Start capture"
#define TOGGLE_CAPTURE_BUTTON_STOP	"Stop capture"

#define PERSON_NAME_MAX_LENGTH		20

#define TRAINING_DATA_FILENAME		"trained_faces.xml"

#define VIDEO_WIDTH					800
#define VIDEO_HEIGHT				600


class AppWindow : public Window
{
public:
	LRESULT proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

	void onCreate(HWND hwnd);
	void onDestroy(HWND hwnd);
	void onPaint(HWND hwnd);
	void onCommand(HWND hwnd, WPARAM wp);
	void onMouseMove(HWND hwnd, LPARAM lp);
	void onMouseDown(HWND hwnd, LPARAM lp);
	void onActivate(HWND hwnd, WPARAM wp);

	void startVideo();
	void stopVideo();

	bool drawCVMat(const cv::Mat img, HDC hdc);
	void drawFrame();

	void recognizeFaces();
	void openLearnFaceDialog();
	void learnFace(std::string name);

	bool loadCascade(std::string filename);

	DWORD videoThread();

private:
	cv::Mat m_lastVideoFrame;

	int m_cameraIndex;
	bool m_isWindowActive;
	bool m_videoThreadRunning;
	HANDLE m_videoThreadHandle;

	HWND m_toggleCaptureButton;
	HWND m_cameraIndexComboBox;
	HWND m_loadCascadeButton;
	HWND m_cascadeFilename;

	bool m_learnMode;
	cv::Rect m_hoveredFace;
	std::vector<cv::Rect> m_faces;
	cv::CascadeClassifier m_faceCascade;
	cv::Ptr<cv::face::LBPHFaceRecognizer> m_faceRecognizer;
	bool m_isFaceRecognizerTrained;
	std::atomic_bool m_painting;
	std::atomic_bool m_closing;

	static DWORD staticVideoThreadStart(void* p);
	static INT_PTR CALLBACK learnFaceDialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp);
};


#endif // APP_WINDOW_H
