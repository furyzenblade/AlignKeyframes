#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string& s) {
    return rtrim(ltrim(s));
}

std::string GetClipboardText() {
    // Try opening the clipboard
    if (!OpenClipboard(nullptr)) exit(1);

    // Get handle of clipboard object for ANSI text
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) exit(1);

    // Lock the handle to get the actual text pointer
    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) exit(1);

    // Save text as string
    std::string text(pszText);
    // Release the lock
    GlobalUnlock(hData);
    // Release the clipboard
    CloseClipboard();
    return text;
}

void SetClipboardText(const std::string& text) {
    if (!OpenClipboard(nullptr)) exit(1);
    if (!EmptyClipboard()) exit(1);

    HGLOBAL clipBuffer;
    char* buffer;

    clipBuffer = GlobalAlloc(GMEM_DDESHARE, text.length() + 1);
    if (clipBuffer == nullptr) exit(1);

    buffer = (char*)GlobalLock(clipBuffer);
    if (buffer == nullptr) exit(1);

    strcpy(buffer, text.c_str());

    GlobalUnlock(clipBuffer);
    SetClipboardData(CF_TEXT, clipBuffer);
    CloseClipboard();
    return;
}

std::vector<std::string> split_string_by_newline(const std::string& str) {
    std::vector<std::string> result = std::vector<std::string>{};
    auto ss = std::stringstream{ str };

    for (std::string line; std::getline(ss, line, '\n');)
        result.push_back(line);

    return result;
}

std::string removeCharacters(std::string S, char c) {
    S.erase(remove(
        S.begin(), S.end(), c),
        S.end());
    return S;
}

std::string strip(std::string& str) {
    //std::cout << str.length() << std::endl;
    std::string temp = str;
    if (!(temp.empty())) {
        //temp = removeCharacters(temp, ' ');
        temp = removeCharacters(temp, '\n');
        temp = removeCharacters(temp, '\t');
        temp = removeCharacters(temp, '\r');
    }
    return temp;
}

void SimulateKeypress_copy() {
    keybd_event(VK_CONTROL, 0x9d, 0, 0);                   // Ctrl Press
    keybd_event((BYTE)VkKeyScanA('C'), 0xAE, 0, 0);               // ‘C’ Press
    keybd_event((BYTE)VkKeyScanA('C'), 0xAE, KEYEVENTF_KEYUP, 0); // ‘C’ Release
    keybd_event(VK_CONTROL, 0x9d, KEYEVENTF_KEYUP, 0);     // Ctrl Release
}

void SimulateKeypress_paste() {
    keybd_event(VK_CONTROL, 0x9d, 0, 0);                   // Ctrl Press
    keybd_event((BYTE)VkKeyScanA('V'), 0xAF, 0, 0);               // ‘V’ Press
    keybd_event((BYTE)VkKeyScanA('V'), 0xAF, KEYEVENTF_KEYUP, 0); // ‘V’ Release
    keybd_event(VK_CONTROL, 0x9d, KEYEVENTF_KEYUP, 0);     // Ctrl Release
}

void SimulateKeypress_time_remap_combo() {
    keybd_event(VK_CONTROL, 0x9d, 0, 0);                   // Ctrl Press
    keybd_event(VK_MENU, 0xb8, 0, 0);                      //Alt Press
    keybd_event((BYTE)VkKeyScanA('T'), 0x94, 0, 0);               // ‘T’ Press
    keybd_event((BYTE)VkKeyScanA('T'), 0x94, KEYEVENTF_KEYUP, 0); // ‘T’ Release
    keybd_event(VK_MENU, 0xb8, KEYEVENTF_KEYUP, 0);        // Alt Release
    keybd_event(VK_CONTROL, 0x9d, KEYEVENTF_KEYUP, 0);     // Ctrl Release
}

INPUT CreateScanCodeEvent(WORD scancode, bool isDown)
{
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;
    input.ki.wScan = scancode;
    input.ki.dwFlags = (isDown ? 0 : KEYEVENTF_KEYUP) | KEYEVENTF_SCANCODE;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    return input;
}

UINT VirtualKeyToScanCode(const char vKeyCode) {
    return MapVirtualKeyExA(vKeyCode, MAPVK_VK_TO_VSC, GetKeyboardLayout(0));
}

#define SLEEP_DELAY 500

int main() {
    //Get After Effects window handle:
    HWND windowHandle = NULL;
    for (windowHandle = GetTopWindow(NULL); windowHandle != NULL; windowHandle = GetNextWindow(windowHandle, GW_HWNDNEXT)) //Loop though all top windows
    {
        if (!IsWindowVisible(windowHandle))
            continue;

        int length = GetWindowTextLength(windowHandle);
        if (length == 0)
            continue;

        char* title = new char[(size_t)length + 1];
        GetWindowTextA(windowHandle, title, length + 1);

        if (strstr(title, "After Effects") == NULL) {
            delete[] title;
            continue;
        }

        //std::cout << "HWND: " << windowHandle << " Title: " << title << std::endl;
        delete[] title;
        break;
    }

    //Focus AE:
    if (windowHandle == NULL) {
        MessageBoxA(NULL, "Failed to get window!", NULL, MB_OK);
        //std::cout << "Failed to get window!" << std::endl;
        return -1;
    }
    if (!SetForegroundWindow(windowHandle)) {
        MessageBoxA(NULL, "Failed to bring window to foreground!", NULL, MB_OK);
        //std::cout << "Failed to bring window to foreground!" << std::endl;
        return -1;
    }
    Sleep(SLEEP_DELAY);

    //Ctrl + C:
    std::vector<INPUT> keystrokes;
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode(VK_CONTROL), true));  //Press CTRL
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode('C'), true));         //Press C
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode('C'), false));        //Release C
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode(VK_CONTROL), false)); //Release CTRL

    //Ctrl + Alt + T:
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode(VK_CONTROL), true));  //Press CTRL
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode(VK_MENU), true));     //Press ALT
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode('T'), true));         //Press T
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode('T'), false));        //Release T
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode(VK_MENU), false));    //Release ALT
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode(VK_CONTROL), false)); //Release CTRL

    SendInput((UINT)keystrokes.size(), keystrokes.data(), sizeof(keystrokes[0]));       //Send keystrokes to AE
    Sleep(SLEEP_DELAY);

    SetForegroundWindow(GetConsoleWindow()); //Unfocus AE

    //Process keyframe data in clipboard:
    std::string str;
    str = GetClipboardText();
    std::vector<std::string> lines = split_string_by_newline(str);
    int count = 0, convert = 0, getfirstnum = 1;
    //std::cout << lines.size() << std::endl;
    std::string finalString = "";
    for (unsigned int i = 0; i < lines.size(); i++) {
        count++;
        if (convert == 0) {
            if (lines[i].find("Frame	seconds") != std::string::npos) {
                finalString += rtrim(lines[i]) + "\t" + "\r\n";
            }
            else {
                finalString += rtrim(lines[i]) + "\r\n";
            }
        }
        if (lines[i].find("End of Keyframe Data") != std::string::npos) {
            convert = 0;
            finalString += "\r\n";
            finalString += "\r\n";
            finalString += "End of Keyframe Data";
            finalString += "\r\n";
        }
        if (convert == 1 && rtrim(lines[i]).compare("") != 0) {
            std::vector<std::string> tokens;
            std::istringstream iss(lines[i]);
            std::string token;
            while (std::getline(iss, token, '\t')) {
                tokens.push_back(token);
            }
            if (getfirstnum == 1) {
                getfirstnum = (int)std::stof(tokens[1]);
            }
            finalString += "\t";
            finalString += std::to_string(getfirstnum);
            finalString += "\t";
            finalString += tokens[2];
            finalString += "\t";
            finalString += "\r\n";
            getfirstnum++;
        }
        if (lines[i].find("seconds") != std::string::npos) {
            convert = 1;
        }
    }
    EmptyClipboard();
    SetClipboardText(finalString);
    //Sleep(2000);

	/* too unstable, results vary depending on the version of ae
    SetForegroundWindow(windowHandle); //Focus AE
    //Sleep(100);

    //Page Down:
    keystrokes.clear();
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode(VK_NEXT), true));  //Press PAGE DOWN
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode(VK_NEXT), false)); //Release PAGE DOWN

    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode(VK_PRIOR), true));  //Press PAGE UP
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode(VK_PRIOR), false)); //Release PAGE UP
    SendInput((UINT)keystrokes.size(), keystrokes.data(), sizeof(keystrokes[0]));
    Sleep(SLEEP_DELAY);

    //Ctrl + V:
    keystrokes.clear();
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode(VK_CONTROL), true));  //Press CTRL
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode('V'), true));         //Press V
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode('V'), false));        //Release V
    keystrokes.push_back(CreateScanCodeEvent(VirtualKeyToScanCode(VK_CONTROL), false)); //Release CTRL
    SendInput((UINT)keystrokes.size(), keystrokes.data(), sizeof(keystrokes[0]));
    //Sleep(100);
	*/
    return 0;
}
