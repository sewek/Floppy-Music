
#include <TimerOne.h>
#include <SPI.h>

#include "bytes.h"

#define RESOLUTION 40
#define HDD_MOSI 51
#define HDD_MISO 50
#define HDD_SCL 52
#define HDD1_SS 40
#define HDD2_SS 41

const uint8_t devices_num = 2;
const uint8_t hdds_num = 2;

const uint8_t hdds_ss[hdds_num] = {HDD1_SS, HDD2_SS};

struct device
{
  uint8_t max_position;
  uint8_t current_position;
  unsigned int current_tick;
  unsigned int note_tick;
  uint8_t direction_pin;
  uint8_t motor_pin;
  uint8_t direction_state;
  uint8_t motor_state;
};

device *dev;

void setup()
{
  dev = new device[2];
  init_devices();
  for(int i = 0; i < hdds_num; i++)
  {
    pinMode(hdds_ss[i], OUTPUT);
    digitalWrite(hdds_ss[i], HIGH);
  }
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  //pinMode(13, OUTPUT);
  delay(1000);
  Timer1.initialize(RESOLUTION);
  Timer1.attachInterrupt(tick);
  delay(500);
  Serial.begin(115200);
  Serial.println("Floppy Drivers Device has been started.");
  Serial.print("Initializing... ");
  Serial.println("Done!");
  Serial.println("Wait for host and listen requests...");
  while(Serial.available());
  Serial.print("Wake up!");
}


void loop()
{
  if (Serial.available() > 2)
  {
    //digitalWrite(13, LOW);
    uint8_t data = Serial.read();
    //Serial.print("DEBUG: Receved: 0x");
    //Serial.println(data, HEX);
    if (isNote(data))
    {
      uint8_t note = Serial.read();
      uint8_t velocity = Serial.read();
      if (isNoteON(data, velocity))
      {
        //Serial.print("DEBUG: note: 0x");
        //Serial.println(note, HEX);
        setNote((data & 0x0F), note, velocity);
      }
      else
      {
        dev[(data & 0x0F)-1].note_tick = 0;
        dev[(data & 0x0F)-1].current_tick = 0;
      }
    }
    else if (isReset(data))
    {
      for (int i = 0; i < devices_num; i++)
      {
        dev[i].note_tick = 0;
        dev[i].current_tick = 0;
      }
    }
    //digitalWrite(13, HIGH);
  }
}

void toogle(uint8_t device)
{
  if (dev[device].current_position > dev[device].max_position)
  {
    digitalWrite(dev[device].direction_pin, HIGH);
    dev[device].direction_state = HIGH;
  }
  else if (dev[device].current_position <= 0)
  {
    digitalWrite(dev[device].direction_pin, LOW);
    dev[device].direction_state = LOW;
  }
  if(dev[device].direction_state == HIGH)
    dev[device].current_position--;
    else
    dev[device].current_position++;
  digitalWrite(dev[device].motor_pin, dev[device].motor_state);
  if (dev[device].motor_state == HIGH)
    dev[device].motor_state = LOW;
    else
    dev[device].motor_state = HIGH;
  //dev[device].motor_state = ~dev[device].motor_state;
}

void tick()
{
  //Serial.println("Tick interrupt.");
  for (int i = 0; i < devices_num; i++)
  {
    if (dev[i].note_tick > 0)
    {
      //Serial.println("Note tick jest ustawiony na jakas nute.");
      if (dev[i].current_tick >= dev[i].note_tick)
      {
        //Serial.println("Make toogle.");
        toogle(i);
        dev[i].current_tick = 0;
      }
      else
      {
        dev[i].current_tick++;
      }
    }
  }
}

bool isNote(uint8_t cmd)
{
  uint8_t pnote = (cmd >> 4);
  //Serial.print("Note turn: ");
  //Serial.println(pnote);
  if (pnote == 8 || pnote == 9)
    return true;
  return false;
}

bool isReset(uint8_t cmd)
{
  if (cmd == 1)
    return true;
  return false;
}

bool isNoteON(uint8_t cmd, uint8_t velocity)
{
  uint8_t pcmd = (cmd >> 4);
  if (pcmd == 9 && velocity > 0)
  {
    return true;
  }
  return false;
}

void init_devices()
{  
  uint8_t max_position[] = { 158, 158 };
  uint8_t current_position[] = { 0, 0 };
  unsigned int current_tick[] = { 0, 0 };
  unsigned int note_tick[] = { 0, 0 };

  uint8_t direction_pin[] = { 3, 5 };
  uint8_t motor_pin[] = { 2, 4 };

  for (int i = 0; i < devices_num; i++)
  {
    dev[i].current_position = current_position[i];
    dev[i].current_tick = current_tick[i];
    dev[i].direction_pin = direction_pin[i];
    dev[i].direction_state = LOW;
    dev[i].max_position = max_position[i];
    dev[i].motor_pin = motor_pin[i];
    dev[i].motor_state = LOW;
    dev[i].note_tick = note_tick[i];
    pinMode(dev[i].motor_pin, OUTPUT);
    pinMode(dev[i].direction_pin, OUTPUT);
    digitalWrite(dev[i].motor_pin, LOW);
    digitalWrite(dev[i].direction_pin, LOW);
  }
}

void setNote(uint8_t track, uint8_t note, uint8_t velocity)
{
  if (track > devices_num)
    sendToHDD(track, note, velocity);
  dev[track-1].note_tick = interrupt_notes[note];
  dev[track-1].current_tick = 0;
  Serial.print("Play note: 0x");
  Serial.print(note, HEX);
  Serial.print(", on track: ");
  Serial.print(track, DEC);
  Serial.print(", with velocity: ");
  Serial.println(velocity, DEC);
}

void sendToHDD(uint8_t track, uint8_t note, uint8_t velocity)
{
  //TODO: Send to SPI.
  if (track > devices_num && track <= hdds_num + devices_num)
  {
    uint8_t current_track = track - devices_num - 1;
    digitalWrite(hdds_ss[current_track], LOW);
    // send bytes
    SPI.transfer(current_track);
    SPI.transfer(note);
    SPI.transfer(velocity);
    digitalWrite(hdds_ss[current_track], HIGH);
  }
}

