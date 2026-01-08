#pragma once
#include <string>

struct KeyboardInfo
{
    int device = 0;
    std::wstring hwid;
    std::wstring vid;
    std::wstring pid;
};
