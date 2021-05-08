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
		else
		{
			// generic error
			return;
		}
	}


	DCB serial_parameters = { 0 };
	if (GetCommState(this->serial_handle, &serial_parameters) == false)
	{
		// log
		return;
	}

	serial_parameters.BaudRate = baud_rate;
	serial_parameters.ByteSize = 8;
	serial_parameters.Parity = NOPARITY;

	if(SetCommState(this->serial_handle, &serial_parameters) == false)
	{
		// log
		return;
	}

	if (SetCommMask(this->serial_handle, EV_RXCHAR) == false)
	{
		//
	}
	
	// log

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

int SerialPort::ReadSerialPort(const char* buffer, unsigned buf_size)
{

	
	return 0;
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

