#include "Application.h"


Application::Application()
{
	setlocale(LC_ALL, "");
}


Application::~Application()
{
}


int Application::run()
{
	std::cout << app_name << " v." << app_version << std::endl;
	showHelp();
	while (true)
	{
		std::cout << "> ";
		std::cin >> cmd;
		switch (cmd)
		{
		case 'c':
			connectDevice();
			break;
		case 'd':
			disconnectDevice();
			break;
		case 'l':
			loadMIDI();
			break;
		case 'p':
			playMusic();
			break;
		case 's':
			stopMusic();
			break;
		case 'm':
			showHelp();
			break;
		case 'q':
			quitApplication();
			return 0;
			break;
		default:
			std::cout << "Nieprawid³owa komenda!" << std::endl;
			continue;
			break;
		}
	}
	return 1;
}


void Application::showHelp()
{
	std::cout << "Type:" << std::endl;
	std::cout << "\tc - Connect to device." << std::endl;
	std::cout << "\td - Disconnect from device." << std::endl;
	std::cout << "\tl - Load MIDI file." << std::endl;
	std::cout << "\tp - Play to device." << std::endl;
	std::cout << "\ts - Stop playing." << std::endl;
	std::cout << "\tm - Show this menu." << std::endl;
	std::cout << "\tq - Quit." << std::endl;
}

void Application::connectDevice()
{
	if (!m_showDevices())
	{
		return;
	}
	std::cout << "Wybierz urz¹dzenie(0): ";
	std::cin >> device_number;
	m_connectToDevice();
}

void Application::disconnectDevice()
{
	if (serial_port != NULL)
	{
		serial_port->close();
		serial_port = NULL;
		std::cout << "Roz³¹czono z urz¹dzeniem: " << ports[device_number].description.c_str() << std::endl;
	}
	else
	{
		std::cout << "Nie po³¹czono z ¿adnym urz¹dzeniem!" << std::endl;
	}
}

void Application::loadMIDI()
{
	std::cout << "Podaj œcie¿kê do pliku: ";
	//std::getline(std::cin, midi_dir);
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	getline(cin, midi_dir);
	char chars = '"';
	midi_dir.erase(std::remove(midi_dir.begin(), midi_dir.end(), chars), midi_dir.end());

	std::cout << "£adujê: " << midi_dir << endl;
	midi_file.read(midi_dir);
	std::cout << "Plik poprawnie za³adowany!" << std::endl;
	std::cout << "TPQ: " << midi_file.getTicksPerQuarterNote() << std::endl;
	std::cout << "TRACKS: " << midi_file.getTrackCount() << std::endl;
	midi_file.joinTracks();
	m_verboseFile();
}

void Application::playMusic()
{
	if (serial_port != NULL && serial_port->isOpen()) {
		std::cout << "Now I am playing the music." << std::endl;
		auto m_delay_start = std::chrono::high_resolution_clock::now();
		long double error = 0;
		char buffer[8];
		time_t t = time(0);
		MidiEvent* mev;
		int deltatick;
		for (int event = 0; event < midi_file[0].size(); event++) {
			mev = &midi_file[0][event];
			if (event == 0) {
				deltatick = mev->tick;
			}
			else {
				deltatick = mev->tick - midi_file[0][event - 1].tick;
			}
			//std::cout << "SEND: ";
			//std::cout << hex;
			for (int i = 0; i < mev->size(); i++) {
				//serial_port->write(string(1, (*mev)[i]));
				//std::cout << (int)(*mev)[i] << " ";
				while (m_send_flag);
				m_send_byte = string(1, (*mev)[i]);
				m_send_flag = true;
			}
			if (deltatick > 0)
			{
				double delay = (deltatick*(60000000 / (200 * midi_file.getTicksPerQuarterNote())));
				//error += (delay - (int)delay);
				//std::cout << dec << endl;
				//std::cout << "SLEEP: " << (int)delay/1000 << " MS" << std::endl;
				//std::cout << "REAL SLEEP: " << delay/1000 << " MS" << std::endl;
				//std::cout << hex;
				std::this_thread::sleep_for(std::chrono::microseconds((int)delay));
			}			
			//std::cout << std::endl;
		}
		time_t tle = time(0) - t;
		auto m_delay_end = std::chrono::high_resolution_clock::now();
		auto dur = m_delay_end - m_delay_start;
		auto m_delay_f_secs = std::chrono::duration_cast<std::chrono::duration<float>>(dur);
		std::cout << dec;
		std::cout << "Playing has been finished." << std::endl;
		std::cout << "Playing process took: " << tle << "s. (" << m_delay_f_secs.count() << "s by system timers)" << std::endl;
		std::cout << "Delay error: " << (double)error << " us. " << std::endl;
		
	}
	else
		std::cout << "Nie po³¹czono z urz¹dzeniem!" << std::endl;
}

void Application::stopMusic()
{
}

void Application::quitApplication()
{
	if (serial_port != NULL) {
		if (serial_port->isOpen())
			serial_port->close();
		delete serial_port;
	}
}

std::string Application::getSendByte()
{
	return m_send_byte;
}

double Application::getSendDelayUs()
{
	return m_send_delay_us;
}

bool Application::getSendFlag()
{
	return m_send_flag;
}

void Application::setSendFlag(bool flag)
{
	m_send_flag = flag;
}

//			#######################
//
//				PRIVATE METHODS
//
//			#######################

bool Application::m_showDevices()
{
	ports = serial::list_ports();
	
	if (ports.size() < 1) {
		std::cout << "There are not ports to create!" << std::endl;
		return false;
	}
	std::cout << "Avaliable ports: " << ports.size() << std::endl;
	std::cout << "Ports:" << std::endl;
	for (int i = 0; i < static_cast<int>(ports.size()); i++)
	{
		std::cout << "(" << i << ") " << ports[i].description.c_str() << std::endl;
	}
	return true;
}

void Application::m_connectToDevice()
{
	if (serial_port == NULL)
	{
		//std::string port = ports[device_number].port.c_str();
		std::string port = "";
		port.append("\\.");
		port.append("\\");
		port.append(ports[device_number].port.c_str());
		std::cout << port << std::endl;
		serial_port = new serial::Serial(port.c_str(), baud_rate);
		serial_port->setBytesize(serial::eightbits);
		serial_port->setFlowcontrol(serial::flowcontrol_none);
		serial_port->setStopbits(serial::stopbits_one);
	}
	if (!serial_port->isOpen())
		serial_port->open();
}

void Application::m_verboseFile()
{
	std::ofstream file("info.txt");
	file.clear();
	file << "TICK    DELTA   TRACK   MIDI MESSAGE\n";
	file << "____________________________________\n";

	MidiEvent* mev;
	int deltatick;
	for (int event = 0; event < midi_file[0].size(); event++) {
		mev = &midi_file[0][event];
		if (event == 0) {
			deltatick = mev->tick;
		}
		else {
			deltatick = mev->tick - midi_file[0][event - 1].tick;
		}
		file << dec << mev->tick;
		file << '\t' << deltatick;
		file << '\t' << mev->track;
		file << '\t' << hex;
		for (int i = 0; i < mev->size(); i++) {
			file << (int)(*mev)[i] << ' ';
		}
		file << endl;	
	}
	file.close();
}
