#include "Application.h"
#include <process.h>

Application *app;

void sendToSerialThread(void *dummy)
{
	std::cout << "Serial Port Thread: ON" << endl;
	while (true)
	{
		if (app->getSendFlag())
		{
			// send byte
			app->serial_port->write(app->getSendByte());
			//std::cout << std::hex << app->getSendByte() << " ";
			app->setSendFlag(false);
		}
	}
}

int main()
{
	app = new Application();
	_beginthread(sendToSerialThread, 512, NULL);
	Sleep(100);
	return app->run();
}
