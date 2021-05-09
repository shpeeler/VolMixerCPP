#include "serialport.h"

SerialPort::SerialPort(const char* port_name, const int baud_rate)
{
	this->is_connected = false;
	this->serial_handle = CreateFileA(port_name, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (this->serial_handle == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			// file not found
			return;
		}
		// generic error
		return;
	}

	DCB serial_parameters = { 0 };
	if (GetCommState(this->serial_handle, &serial_parameters) == false)
	{
		// log get last error
		return;
	}

	serial_parameters.BaudRate = baud_rate;
	serial_parameters.ByteSize = 8;
	serial_parameters.Parity = NOPARITY;

	if (SetCommState(this->serial_handle, &serial_parameters) == false)
	{
		// log get last error
		return;
	}

	if (SetCommMask(this->serial_handle, EV_RXCHAR) == false)
	{
		// log get last error
		return;
	}
	// log

	overlapped.hEvent = CreateEvent(nullptr, true, FALSE, nullptr);

	this->is_connected = true;
	PurgeComm(this->serial_handle, PURGE_RXCLEAR);
}

SerialPort::~SerialPort()
{
	if (this->is_connected == true)
	{
		this->is_connected = false;
		CloseHandle(this->serial_handle);
	}
}

bool SerialPort::TryReadSerialPort(const char* buffer, const unsigned int buffer_size)
{
	if (WaitCommEvent(this->serial_handle, &event_mask, &overlapped) == false)
	{
		if ((GetLastError() == ERROR_IO_PENDING) == false)
		{
			return false;
		}

		// log getlasterrror if error io pending 
		return false;
	}

	DWORD wait = WaitForSingleObject(this->serial_handle, INFINITE);
	if (wait == WAIT_OBJECT_0)
	{
		DWORD mask;
		DWORD read;
		if (GetCommMask(this->serial_handle, &mask) == false)
		{
			// unable to get comm mask
			return false;
		}
		if ((mask & EV_RXCHAR) == false)
		{
			// no read signal
			return false;
		}

		if (GetOverlappedResult(this->serial_handle, &this->overlapped, &read, FALSE) == false)
		{
			//  error in comm
			return false;
		}

		if (ReadFile(this->serial_handle, (void**)buffer, buffer_size, nullptr, &this->overlapped) == false)
		{
			// unable to read
			return false;
		}

		// log
		return true;
	}

	return false;
}

bool SerialPort::IsConnected()
{
	if (ClearCommError(this->serial_handle, &this->errors, &this->status) == false)
	{
		this->is_connected = false;
	}

	return this->is_connected;
}

void SerialPort::CloseSerial() const
{
	CloseHandle(this->serial_handle);
}

