#include "Header.h"

wstring stringToWideString(const string& str) {
    wstring wideStr(str.begin(), str.end());
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