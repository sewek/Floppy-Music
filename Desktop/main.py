from mido import MidiFile
import serial
import serial.tools.list_ports

if __name__ == "__main__":
    
    print('Available ports:')
    for port in serial.tools.list_ports.comports():
        print('  - {}'.format(port.device))
    
    port = input('Choose port: ')

    serialPort = serial.Serial(port=port, baudrate=115200)

    file_name = './HesaPirate.mid' #input('Enter path to midi file: ')

    file = MidiFile(file_name)
    for msg in file.play():
        if not msg.is_meta:
            try:
                serialPort.write('D:{};{};{};'.format(msg.channel, msg.note, msg.velocity).encode())
            except AttributeError as ex:
                pass
