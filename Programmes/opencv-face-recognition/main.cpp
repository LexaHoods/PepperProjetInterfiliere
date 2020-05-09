#include "app_window.h"
#include "resource.h"

#pragma comment(lib, "opencv_world430")
#pragma comment(lib, "ippicvmt")
#pragma comment(lib, "ippiw")
#pragma comment(lib, "zlib")
#pragma comment(lib, "libjpeg-turbo")
#pragma comment(lib, "libtiff")
#pragma comment(lib, "libpng")
#pragma comment(lib, "libwebp")
#pragma comment(lib, "libjasper")
#pragma comment(lib, "ittnotify")
#pragma comment(lib, "IlmImf")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")



int main()
{
    MSG msg;
    AppWindow window;
    RECT rect = { 0, 0, VIDEO_WIDTH, VIDEO_HEIGHT + VIDEO_OFFSET_Y };
    HACCEL accelerators = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_ACCELERATOR1));

    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    if (!window.create("opencv-face-recognition", "OpenCV Face Recognition - Projet interfilière 2019 - Pepper Squad", rect.right - rect.left, rect.bottom - rect.top, WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME))
        return -1;

    window.show();

    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        if (!TranslateAccelerator(window.getHandle(), accelerators, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}
