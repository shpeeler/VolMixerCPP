#pragma once

#include <Windows.h>

#include "spdlog/sinks/basic_file_sink.h"
using std::shared_ptr;
using spdlog::logger;

constexpr int MAX_DATA_LENGTH = 8;

class SerialPort
{
private:
    shared_ptr<logger> logger_;
    HANDLE serial_handle;
    bool is_connected;
    COMSTAT status;
    DWORD errors;
    DWORD event_mask;
    OVERLAPPED overlapped = { 0 };

public:
    explicit SerialPort(shared_ptr<logger>& logger, const char* port_name, const int baud_rate);
    ~SerialPort();

    bool TryReadSerialPort(const char* buffer, const unsigned int buffer_size);
    bool IsConnected();
    void CloseSerial() const;
};