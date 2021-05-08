#pragma once

#include <Windows.h>
#include <iostream>

constexpr int MAX_DATA_LENGTH = 255;

class SerialPort
{
private:
    HANDLE serial_handle;
    bool is_connected;
    COMSTAT status;
    DWORD errors;
public:
    explicit SerialPort(const char* port_name, int baud_rate);
    ~SerialPort();

    int ReadSerialPort(const char* buffer, unsigned int buf_size);
    bool IsConnected();
    void CloseSerial() const;
};