#pragma once

#include <iostream>
#include <string>
#include <cstdio>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <thread>
#include <time.h>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "serial.h"
#include "MidiFile.h"

class Application
{
public:
	// public methods
	Application();
	~Application();
	int run();
	void showHelp();
	void connectDevice();
	void disconnectDevice();
	void loadMIDI();
	void playMusic();
	void stopMusic();
	void quitApplication();

	std::string getSendByte();
	double getSendDelayUs();
	bool getSendFlag();
	void setSendFlag(bool flag);
	// public variables
	serial::Serial *serial_port = NULL;

private:
	// private methods
	bool m_showDevices();
	void m_connectToDevice();
	void m_verboseFile();
	// private variables
	std::string app_name = "FloppyDrivePlayer";
	std::string app_version = "0.1 indev";
	char cmd;
	std::vector<serial::PortInfo> ports;
	int device_number = 0;
	int baud_rate = 115200;
	long timeout = 3000;
	MidiFile midi_file;
	std::string midi_dir;
	std::string m_send_byte;
	double m_send_delay_us;
	bool m_send_flag;
	
};

