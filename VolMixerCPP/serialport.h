#pragma once

#include <Windows.h>
#include <iostream>

constexpr int MAX_DATA_LENGTH = 8;

class SerialPort
{
private:
    HANDLE serial_handle;
    bool is_connected;
    COMSTAT status;
    DWORD errors;
    DWORD event_mask;
    OVERLAPPED overlapped = { 0 };

public:
    explicit SerialPort(const char* port_name, int baud_rate);
    ~SerialPort();

    bool TryReadSerialPort(const char* buffer, const unsigned int buffer_size);
    bool IsConnected();
    void CloseSerial() const;
};