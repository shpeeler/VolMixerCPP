#include "serialport.h"

/// <summary>
/// creates an instance of SerialPort
/// </summary>
/// <param name="logger">reference to spdlog::logger</param>
/// <param name="port_name">name of the com-port</param>
/// <param name="baud_rate">baudrate for communication</param>
SerialPort::SerialPort(shared_ptr<logger>& logger, const char* port_name, const int baud_rate)
{
	this->logger_ = logger;
	
	this->is_connected = false;
	this->serial_handle = CreateFileA(port_name, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (this->serial_handle == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			logger_->error("ERROR_FILE_NOT_FOUND for serial port: {0}", port_name);
			return;
		}

		logger_->error("error when creating a handle for serial port: {0}", port_name);
		return;
	}

	DCB serial_parameters = { 0 };
	if (GetCommState(this->serial_handle, &serial_parameters) == false)
	{
		logger_->error("unable to get comm-state for: {0}", port_name);
		return;
	}

	serial_parameters.BaudRate = baud_rate;
	serial_parameters.ByteSize = MAX_DATA_LENGTH;
	serial_parameters.Parity = NOPARITY;

	if (SetCommState(this->serial_handle, &serial_parameters) == false)
	{
		logger_->error("unable to set comm-state for: {0}", port_name);
		return;
	}

	if (SetCommMask(this->serial_handle, EV_RXCHAR) == false)
	{
		logger_->error("unable to set comm-mask for: {0}", port_name);
		return;
	}

	overlapped.hEvent = CreateEvent(nullptr, true, FALSE, nullptr);

	this->is_connected = true;
	PurgeComm(this->serial_handle, PURGE_RXCLEAR);
}

/// <summary>
/// destructor of SerialPort
/// </summary>
SerialPort::~SerialPort()
{
	if (this->is_connected == true)
	{
		this->is_connected = false;
		CloseSerial();
	}
}

/// <summary>
/// checks for available data from the configured serial-port and reads its data
/// </summary>
/// <param name="buffer">reference to buffer</param>
/// <param name="buffer_size">buffer-size in bytes</param>
/// <returns>true if successful, false if not</returns>
bool SerialPort::TryReadSerialPort(const char* buffer, const unsigned int buffer_size)
{
	if (WaitCommEvent(this->serial_handle, &event_mask, &overlapped) == false)
	{
		if ((GetLastError() == ERROR_IO_PENDING) == false)
		{
			logger_->info("i/o pending for serial port");
			return false;
		}

		logger_->error("error while reading from serial port");
		return false;
	}

	DWORD wait = WaitForSingleObject(this->serial_handle, INFINITE);
	if (wait == WAIT_OBJECT_0)
	{
		DWORD mask;
		DWORD read;
		if (GetCommMask(this->serial_handle, &mask) == false)
		{
			logger_->error("unable to get comm-mask");
			return false;
		}
		if ((mask & EV_RXCHAR) == false)
		{
			logger_->debug("no read-event triggered");
			return false;
		}

		if (GetOverlappedResult(this->serial_handle, &this->overlapped, &read, FALSE) == false)
		{
			logger_->error("unable to get result for overlapped-read");
			return false;
		}

		if (ReadFile(this->serial_handle, (void**)buffer, buffer_size, nullptr, &this->overlapped) == false)
		{
			logger_->error("unable to read serial-data");
			return false;
		}

		return true;
	}

	return false;
}

/// <summary>
/// checks whether a connection to the serial-port is open
/// </summary>
/// <returns>true if yes, false if not</returns>
bool SerialPort::IsConnected()
{
	if (ClearCommError(this->serial_handle, &this->errors, &this->status) == false)
	{
		this->is_connected = false;
	}

	return this->is_connected;
}

/// <summary>
/// closes the serial-handle
/// </summary>
void SerialPort::CloseSerial() const
{
	CloseHandle(this->serial_handle);
}

