﻿#include <Windows.h>
#include <mmsystem.h>
#include <vector>
#include <string>
#include <tchar.h>

#pragma comment(lib, "winmm.lib")

HWND hwnd;
HINSTANCE hInst;
static TCHAR szWindowClass[] = _T("Player");
static TCHAR szTitle[] = _T("Player");


struct AudioTrack {
    std::wstring filePath;
    std::wstring name;
};

std::vector<AudioTrack> playlist;
int currentTrackIndex = 0;

HWND hPlayButton;
HWND hPauseButton;
HWND hStopButton;
HWND hNextButton;
HWND hPrevButton;
HWND hPlaylist;
HWND hAddMusicButton;



void PlayTrack(int trackIndex) {

    WCHAR result[128]; // Буфер для хранения результата
    mciSendString(L"status Song mode", result, sizeof(result), NULL);

    if (wcscmp(result, L"paused") == 0 && currentTrackIndex == trackIndex) {
        mciSendString(L"play Song", NULL, 0, NULL);
    }
    else {
        currentTrackIndex = trackIndex;
        mciSendString(L"close all", NULL, 0, NULL);
        std::wstring command = L"open \"" + playlist[currentTrackIndex].filePath + L"\" type mpegvideo alias Song";
        mciSendString(command.c_str(), NULL, 0, NULL);
        mciSendString(L"play Song", NULL, 0, NULL);
    }
    SendMessage(hPlaylist, LB_SETCURSEL, (WPARAM)currentTrackIndex, 0); //устанавливает текущий элемент в элементе управления списка 

}


void NextTrack() {
    if (currentTrackIndex == playlist.size() - 1) {
        PlayTrack(0);
    }
    else {
        PlayTrack(currentTrackIndex + 1);
    }
}

void PrevTrack() {
    if (currentTrackIndex == 0) {
        PlayTrack(playlist.size() - 1);
    }
    else {
        PlayTrack(currentTrackIndex - 1);
    }
}


void AddTrackToPlaylist(const std::wstring& filePath) {
    AudioTrack track;
    track.filePath = filePath;
    size_t lastSlash = filePath.find_last_of(L"\\");
    if (lastSlash != std::wstring::npos) {
        track.name = filePath.substr(lastSlash + 1);
    }
    else {
        track.name = filePath;
    }
    playlist.push_back(track);

    SendMessage(hPlaylist, LB_ADDSTRING, 0, (LPARAM)track.name.c_str());
}

void AddMusicFileFromDialog(HWND hWnd) {
    OPENFILENAME ofn;
    wchar_t szFile[MAX_PATH] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Audio Files\0*.mp3;*.wav;*.ogg\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;  //Устанавливает индекс фильтра по умолчанию (1 - Audio Files)
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        AddTrackToPlaylist(szFile);
    }
}



//Главная функция обработки сообщения
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        hAddMusicButton = CreateWindow(_T("BUTTON"), _T("Add Music File"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 487, 30, hWnd, (HMENU)7, hInst, NULL);
        hPrevButton = CreateWindow(_T("BUTTON"), _T("Prev"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 280, 121, 30, hWnd, (HMENU)5, hInst, NULL);
        hPlayButton = CreateWindow(_T("BUTTON"), _T("Play"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 122, 280, 121, 30, hWnd, (HMENU)1, hInst, NULL);
        hPauseButton = CreateWindow(_T("BUTTON"), _T("Pause"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 244, 280, 121, 30, hWnd, (HMENU)2, hInst, NULL);
        hNextButton = CreateWindow(_T("BUTTON"), _T("Next"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 366, 280, 121, 30, hWnd, (HMENU)4, hInst, NULL);
        hPlaylist = CreateWindow(_T("LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | LBS_HASSTRINGS, 0, 33, 487, 257, hWnd, (HMENU)6, hInst, NULL);
        hStopButton = CreateWindow(_T("BUTTON"), _T("Stop"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 244, 280, 121, 30, hWnd, (HMENU)3, hInst, NULL);

        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1:
            PlayTrack(currentTrackIndex);
            break;

        case 2:
            mciSendString(L"pause Song", NULL, 0, NULL);

            break;

        case 3:
            mciSendString(L"stop Song", NULL, 0, NULL);
            break;

        case 4:
            NextTrack();
            break;

        case 5:
            PrevTrack();
            break;

        case 6:
            if (HIWORD(wParam) == LBN_DBLCLK) {
                int selectedTrackIndex = SendMessage(hPlaylist, LB_GETCURSEL, 0, 0);
                if (selectedTrackIndex != LB_ERR) {
                    PlayTrack(selectedTrackIndex);
                }
            }
            break;

        case 7:
            AddMusicFileFromDialog(hWnd);
            break;

        default:
            break;
        }
        break;


    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
                 break;

    case WM_DESTROY:
        mciSendString(L"close all", NULL, 0, NULL);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}










int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;// Cтиль окна. CS_HREDRAW перерисовка,при изменении ширины, CS_VREDRAW высоты
    wcex.lpfnWndProc = WndProc; // Указатель на функцию, которая будет обрабатывать сообщения
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance; // Дескриптор текущего экземпляра приложения, для указания, какому приложению принадлежит окно.
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION); //значок окна 
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(51, 51, 51));
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Player"), 0);
        return 1;
    }

    hInst = hInstance;
    hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        0, 0, 500, 350, NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        MessageBox(NULL, _T("Call to CreateWindow failed!"), _T("Player"), 0);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg); // Отправляет сообщение msg оконной процедуре WndProc,  которая была зарегистрирована в wcex
    }

    return (int)msg.wParam;
}
