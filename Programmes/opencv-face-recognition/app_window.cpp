#include "app_window.h"
#include "resource.h"
#include <windowsx.h>
#include <commctrl.h>


LRESULT AppWindow::proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CREATE:
		onCreate(hwnd);
		break;
	case WM_DESTROY:
		onDestroy(hwnd);
		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		onPaint(hwnd);
		break;
	case WM_COMMAND:
		onCommand(hwnd, wp);
		break;
	case WM_MOUSEMOVE:
		onMouseMove(hwnd, lp);
		break;
	case WM_LBUTTONDOWN:
		onMouseDown(hwnd, lp);
		break;
	case WM_ACTIVATE:
		onActivate(hwnd, wp);
		break;
	default:;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

void AppWindow::onCreate(HWND hwnd)
{
	m_cameraIndex = 0;
	m_closing = false;
	m_learnMode = false;
	m_videoThreadHandle = NULL;

	m_toggleCaptureButton = CreateWindow(WC_BUTTON, TOGGLE_CAPTURE_BUTTON_START, WS_CHILD | WS_VISIBLE, 10, 10, 160, 26, hwnd, (HMENU)ID_TOGGLE_CAPTURE_BUTTON, NULL, 0);
	m_cameraIndexComboBox = CreateWindow(WC_COMBOBOX, "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 180, 11, 90, 26, hwnd, (HMENU)ID_CAMERA_INDEX_COMBOXBOX, NULL, 0);
	m_loadCascadeButton = CreateWindow(WC_BUTTON, "Load cascade", WS_CHILD | WS_VISIBLE, 280, 10, 110, 26, hwnd, (HMENU)ID_LOAD_CASCADE_BUTTON, NULL, 0);
	m_cascadeFilename = CreateWindow(WC_STATIC, "No cascade loaded", WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 400, 10, 390, 26, hwnd, NULL, NULL, 0);

	SendMessage(m_cameraIndexComboBox, CB_ADDSTRING, 0, (LPARAM)"Camera 0");
	SendMessage(m_cameraIndexComboBox, CB_ADDSTRING, 0, (LPARAM)"Camera 1");
	SendMessage(m_cameraIndexComboBox, CB_ADDSTRING, 0, (LPARAM)"Camera 2");
	SendMessage(m_cameraIndexComboBox, CB_ADDSTRING, 0, (LPARAM)"Camera 3");
	SendMessage(m_cameraIndexComboBox, CB_ADDSTRING, 0, (LPARAM)"Camera 4");
	SendMessage(m_cameraIndexComboBox, CB_ADDSTRING, 0, (LPARAM)"Camera 5");
	SendMessage(m_cameraIndexComboBox, CB_ADDSTRING, 0, (LPARAM)"Camera 6");
	SendMessage(m_cameraIndexComboBox, CB_ADDSTRING, 0, (LPARAM)"Camera 7");
	SendMessage(m_cameraIndexComboBox, CB_ADDSTRING, 0, (LPARAM)"Camera 8");
	SendMessage(m_cameraIndexComboBox, CB_ADDSTRING, 0, (LPARAM)"Camera 9");
	SendMessage(m_cameraIndexComboBox, CB_SETCURSEL, 0, 0);

	if (!loadCascade("lbpcascades\\lbpcascade_frontalface.xml")) {
		DestroyWindow(hwnd);
		return;
	}

	m_faceRecognizer = cv::face::LBPHFaceRecognizer::create();

	try {
		m_faceRecognizer->read(TRAINING_DATA_FILENAME);
		m_isFaceRecognizerTrained = true;
	}
	catch (cv::Exception& e) {
		CV_UNUSED(e);
		m_isFaceRecognizerTrained = false;
		std::cout << "The file " << TRAINING_DATA_FILENAME << " could not be loaded" << std::endl;
	}
}

void AppWindow::onDestroy(HWND hwnd)
{
	m_closing = true;

	stopVideo();
	
	CloseHandle(m_videoThreadHandle);
	PostQuitMessage(0);
}

void AppWindow::onPaint(HWND hwnd)
{
	m_painting = true;

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	FillRect(hdc, &clientRect, (HBRUSH)(COLOR_BTNFACE + 1));

	drawFrame();
	EndPaint(hwnd, &ps);

	m_painting = false;
}

void AppWindow::onCommand(HWND hwnd, WPARAM wp)
{
	int id = LOWORD(wp);

	if (id == ID_TOGGLE_CAPTURE_BUTTON) {
		DWORD exitCode = NULL;

		if (m_videoThreadHandle != NULL && GetExitCodeThread(m_videoThreadHandle, &exitCode) == TRUE && exitCode == STILL_ACTIVE) {
			EnableWindow(m_toggleCaptureButton, !m_videoThreadRunning);
			stopVideo();
		} else {
			EnableWindow(m_toggleCaptureButton, FALSE);
			startVideo();
		}
	} else if (id == ID_CAMERA_INDEX_COMBOXBOX) {
		if (HIWORD(wp) == CBN_SELCHANGE)
			m_cameraIndex = static_cast<int>(SendMessage(m_cameraIndexComboBox, CB_GETCURSEL, 0, 0));
	} else if (id == ID_LOAD_CASCADE_BUTTON) {
		char path[MAX_PATH] = { 0 };
		OPENFILENAME ofn;

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = path;
		ofn.nMaxFile = sizeof(path);
		ofn.lpstrFilter = "XML Files (*.xml)\0*.xml\0";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

		if (GetOpenFileName(&ofn) != 0)
			loadCascade(path);
	} else if (id == ID_ACCELERATOR_ESCAPE) {
		DestroyWindow(hwnd);
	}
}

void AppWindow::onMouseMove(HWND hwnd, LPARAM lp)
{
	if (!m_learnMode)
		return;

	cv::Point point(GET_X_LPARAM(lp), GET_Y_LPARAM(lp) - VIDEO_OFFSET_Y);
	bool hoveringFace = false;

	for (size_t i = 0; i < m_faces.size(); i++)
	{
		if (m_faces[i].contains(point) && !hoveringFace) {
			cv::rectangle(m_lastVideoFrame, m_faces[i], cv::Scalar(0, 0, 255), 2);
			m_hoveredFace = m_faces[i];
			hoveringFace = true;
		} else {
			cv::rectangle(m_lastVideoFrame, m_faces[i], cv::Scalar(255, 0, 0), 2);
		}
	}

	drawFrame();
}

void AppWindow::onMouseDown(HWND hwnd, LPARAM lp)
{
	if (!m_learnMode || m_hoveredFace.empty())
		return;

	cv::Point point(GET_X_LPARAM(lp), GET_Y_LPARAM(lp) - VIDEO_OFFSET_Y);

	if (!m_hoveredFace.contains(point))
		return;

	openLearnFaceDialog();
}

void AppWindow::onActivate(HWND hwnd, WPARAM wp)
{
	m_isWindowActive = (LOWORD(wp) != WA_INACTIVE);
}

void AppWindow::startVideo()
{
	if(m_videoThreadHandle)
		CloseHandle(m_videoThreadHandle);

	m_videoThreadHandle = CreateThread(NULL, 0, staticVideoThreadStart, (void*)this, 0, NULL);

	if (m_videoThreadHandle == NULL)
		MessageBox(m_hwnd, "The video thread could not be created", "Error", MB_OK);
}

void AppWindow::stopVideo()
{
	m_videoThreadRunning = false;

	if(m_closing)
		WaitForSingleObject(m_videoThreadHandle, 2500);
}

bool AppWindow::drawCVMat(const cv::Mat img, HDC hdc)
{
	if (!img.data)
		return false;

	cv::Size imgSize = img.size();
	unsigned int depth;

	switch (img.depth())
	{
	case CV_8U:
	case CV_8S:
		depth = 8u;
		break;
	default:
		depth = 0u;
	}

	BITMAPINFOHEADER bih;
	ZeroMemory(&bih, sizeof(bih));
	bih.biSize = sizeof(bih);
	bih.biWidth = imgSize.width;
	bih.biHeight = -(imgSize.height);
	bih.biPlanes = 1;
	bih.biBitCount = img.channels() * depth;

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader = bih;
	bi.bmiColors->rgbBlue = 0;
	bi.bmiColors->rgbGreen = 0;
	bi.bmiColors->rgbRed = 0;
	bi.bmiColors->rgbReserved = 0;

	StretchDIBits(hdc, 0, 0, imgSize.width, imgSize.height, 0, 0, imgSize.width, imgSize.height, img.data, &bi, DIB_RGB_COLORS, SRCCOPY);

	return true;
}

void AppWindow::drawFrame()
{
	HDC hdc = GetDC(m_hwnd);
	
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP bmp = CreateCompatibleBitmap(hdc, m_lastVideoFrame.cols, m_lastVideoFrame.rows);
	HGDIOBJ oldBmp = SelectObject(hdcMem, bmp);

	if (m_lastVideoFrame.data)
		drawCVMat(m_lastVideoFrame, hdcMem);

	BitBlt(hdc, 0, VIDEO_OFFSET_Y, m_lastVideoFrame.cols, m_lastVideoFrame.rows, hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, oldBmp);
	DeleteObject(bmp);
	DeleteDC(hdcMem);

	ReleaseDC(m_hwnd, hdc);
}

void AppWindow::recognizeFaces()
{
	cv::Mat grayImg;
	m_faces.clear();
	
	cv::cvtColor(m_lastVideoFrame, grayImg, cv::COLOR_BGR2GRAY);
	cv::equalizeHist(grayImg, grayImg);

	m_faceCascade.detectMultiScale(grayImg, m_faces);

	for (size_t i = 0; i < m_faces.size(); i++)
	{
		if (m_isFaceRecognizerTrained) {
			int label;
			double confidence;
			cv::Mat face(grayImg, m_faces[i]);
			m_faceRecognizer->predict(face, label, confidence);

			int captionHeight = 16;
			cv::Rect captionRect(m_faces[i].x - 1, m_faces[i].y - captionHeight, m_faces[i].width + 2, captionHeight);
			cv::rectangle(m_lastVideoFrame, captionRect, cv::Scalar(255, 0, 0), -1);

			int baseline = 0;
			cv::String caption = std::to_string(label) + ": " + m_faceRecognizer->getLabelInfo(label) + " (" + std::to_string((int)confidence) + ")";
			cv::Size textSize = cv::getTextSize(caption, cv::FONT_HERSHEY_PLAIN, 1.0, 1, &baseline);
			cv::Point textPosition(captionRect.x + (captionRect.width - textSize.width) / 2, captionRect.y + captionRect.height - 1);
			cv::putText(m_lastVideoFrame, caption, textPosition, cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(255, 255, 255), 1);
		}

		cv::rectangle(m_lastVideoFrame, m_faces[i], cv::Scalar(255, 0, 0), 2);
	}
}

void AppWindow::openLearnFaceDialog()
{
	DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LEARN_FACE_DIALOG), m_hwnd, learnFaceDialogProc, (LPARAM)this);
}

void AppWindow::learnFace(std::string name)
{
	name[0] = std::toupper(name[0]);

	cv::Mat face(m_lastVideoFrame, m_hoveredFace);
	cv::cvtColor(face, face, cv::COLOR_BGR2GRAY);
	cv::resize(face, face, cv::Size(100, 100));

	int label;

	std::vector<int> m_foundLabels = m_faceRecognizer->getLabelsByString(name);

	if (!m_foundLabels.empty()) {
		label = m_foundLabels[0];
	} else {
		label = 0;

		std::vector<int> allLabels = m_faceRecognizer->getLabels();
		std::vector<int>::iterator it;

		it = std::find(allLabels.begin(), allLabels.end(), label);

		while (it != allLabels.end())
			it = std::find(allLabels.begin(), allLabels.end(), ++label);
	}

	std::vector<int> labels;
	labels.push_back(label);

	std::vector<cv::Mat> images;
	images.push_back(face);
	m_faceRecognizer->update(images, labels);
	m_faceRecognizer->setLabelInfo(label, name);

	m_isFaceRecognizerTrained = true;
	m_faceRecognizer->save(TRAINING_DATA_FILENAME);
}

bool AppWindow::loadCascade(std::string filename)
{
	bool success(false);

	try {
		success = m_faceCascade.load(filename);
	}
	catch (cv::Exception& e) {
		CV_UNUSED(e);
		success = false;
	}

	EnableWindow(m_toggleCaptureButton, success);

	if (!success) {
		SetWindowText(m_cascadeFilename, "No cascade loaded");
		MessageBox(m_hwnd, "Could not load the cascade", "Error", MB_OK);
	} else {
		size_t pos = filename.find_last_of("\\");

		if (pos != std::string::npos)
			filename.erase(0, pos + 1);

		SetWindowText(m_cascadeFilename, std::string("Cascade: " + filename).c_str());
	}
	
	return success;
}

DWORD AppWindow::videoThread()
{
	cv::VideoCapture capture(m_cameraIndex, cv::CAP_DSHOW);
	cv::Mat frame;

	capture.set(cv::CAP_PROP_FRAME_WIDTH, 640.0);
	capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480.0);

	if (capture.isOpened()) {
		m_videoThreadRunning = true;
		m_learnMode = false;

		EnableWindow(m_toggleCaptureButton, TRUE);
		EnableWindow(m_cameraIndexComboBox, FALSE);
		EnableWindow(m_loadCascadeButton, FALSE);
		SetWindowText(m_toggleCaptureButton, TOGGLE_CAPTURE_BUTTON_STOP);
	} else {
		MessageBox(m_hwnd, "Video capture could not be started", "Error", MB_OK);
	}

	while (m_videoThreadRunning)
	{
		if (!m_painting) {
			capture >> frame;

			cv::flip(frame, frame, 1);
			cv::resize(frame, m_lastVideoFrame, cv::Size(800, 600));

			recognizeFaces();
			drawFrame();

			if (m_isWindowActive && GetAsyncKeyState('S') & 0x8000)
				capture.set(cv::CAP_PROP_SETTINGS, 0.0);
		}

		// Sleep(5);
	}

	if (!m_closing) {
		EnableWindow(m_toggleCaptureButton, TRUE);
		EnableWindow(m_cameraIndexComboBox, TRUE);
		EnableWindow(m_loadCascadeButton, TRUE);
		SetWindowText(m_toggleCaptureButton, TOGGLE_CAPTURE_BUTTON_START);

		m_hoveredFace = cv::Rect();
		m_learnMode = true;
	}

	return 0;
}

DWORD AppWindow::staticVideoThreadStart(void* p)
{
	AppWindow *_this = (AppWindow*)p;
	return _this->videoThread();
}

INT_PTR AppWindow::learnFaceDialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
	HWND edit(NULL);
	int enteredChars;
	char text[PERSON_NAME_MAX_LENGTH + 1];
	static AppWindow *_this;

	switch (msg)
	{
	case WM_INITDIALOG:
		_this = (AppWindow*)lp;
		edit = GetDlgItem(hdlg, IDC_EDIT1);
		SendDlgItemMessage(hdlg, IDC_EDIT1, EM_SETLIMITTEXT, PERSON_NAME_MAX_LENGTH, 0);
		if ((HWND)wp != edit) {
			SetFocus(edit);
			return FALSE;
		}
		return TRUE;
	case WM_COMMAND:
		switch (wp)
		{
		case IDOK:
			enteredChars = (int)SendDlgItemMessage(hdlg, IDC_EDIT1, EM_LINELENGTH, 0, 0);
			if (enteredChars == 0) {
				MessageBox(hdlg, "You need to enter at least 1 character", "Error", MB_OK);
				return TRUE;
			}
			GetDlgItemText(hdlg, IDC_EDIT1, text, sizeof(text));
			if (text[0] == ' ') {
				MessageBox(hdlg, "The first character cannot be a space", "Error", MB_OK);
				return TRUE;
			}
			EndDialog(hdlg, TRUE);
			_this->learnFace(text);
			return TRUE;
		case IDCANCEL:
			EndDialog(hdlg, TRUE);
			return TRUE;
		}
		return FALSE;
	default:;
	}

	return FALSE;
}
