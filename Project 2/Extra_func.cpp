#include "Header.h"

std::wstring stringToWideString(const std::string& str) {
    std::wstring wideStr(str.begin(), str.end());
    return wideStr;
}

int littleEndianByteArrayToInt(const BYTE* byteArray, size_t length) {
    int result = 0;
    for (size_t i = 0; i < length; ++i) {
        result |= (static_cast<int>(byteArray[i]) << (i * 8));
    }
    return result;
}

int byteToTwosComplement(int byteValue) {
    if (byteValue & 0x80) {
        return byteValue - 256;
    }
    else {
        return byteValue;
    }
}
std::wstring FileSystemEntity::stringToWstring(const std::string& str) {
    std::locale loc(std::locale(), new std::codecvt_utf8<wchar_t>);
    const std::ctype<wchar_t>& ct = std::use_facet<std::ctype<wchar_t>>(loc);
    std::wstring wstr(str.length(), L' ');
    ct.widen(str.data(), str.data() + str.length(), &wstr[0]);

    return wstr;
}
