/******************************************************************************/
// AVR High Voltage Serial Programmer 1.01 by Mr.Blinky Mar-Apr.2020 CC0 Licence
/******************************************************************************

Atmega32u4 beetle board wiring
------------------------------
 
       SCI __________
       SDO ________  |
       SII ______  | |
       SDI ____  | | |
             __|_|_|_|_______
            |  O O O O       |
BUTTON     _|O               |____
LED_GREEN  _|O        LED         |_BUILTIN_LED
VCC_LED    _|O                    |
           _|O                    |
WEAK_VCC   _|O                ____|
VPP        _|O               |
            |__O_O_O_O_O_O_O_|
        5V ____| |
       GND ______|

connect SCI,SDO,SII and SDI to device with 470 Ohm resistors in series.

connect BUTTON to momentary pushbutton and other end of button to GND

connect cathode(-) of green LED to LED_GREEN pin
connect 330 Ohm resistor to anode(+) of green LED pin
connect other end 330 Ohm resistor to 5V

connect cathode(-) of yellow LED to VCC_LED pin
connect 330 Ohm resistor to anode(+) of yellow LED pin 
connect other end 330 Ohm resistor to 5V

for low power device/circuit:
-----------------------------
Connect WEAK_VCC directly to device VCC pin.

for more demanding circuits:
----------------------------
leave WEAK_VCC unconnected and connect a P-channel MOSFET as following:
Connect MOSFET Source to 5V, Connect MOSFET Drain to device VCC and 
connect MOSFET Gate to anode(+) of yellow LED.

Connect VPP to enable input of 12V DC-DC converter module

connect 1K resistor to 12V
connect other end of 1K resistor to anode(+) of the red LED
connect cathode(-) of the red LED to GND

*******************************************************************************/

// version info
constexpr uint8_t HARDWAREVERSION       = 2;
// 1: for basic prototype made of beetle board, 12V DC-DC converter module and weak vcc
// 2: PCB version with added Vcc LED, Green LED and feature button
//    (P-channel MOSFET for more powerfull VCC is optional)

constexpr uint8_t SOFTWAREVERSION_MAJOR = 1;
constexpr uint8_t SOFTWAREVERSION_MINOR = 1;

// pin configurations (Using direct IO instead of slow DigitalRead/Write and pinMode)

#define HVSP_SCI_BIT    0       // 3/SCL output connected to SCI pin
#define HVSP_SCI_PIN    PIND
#define HVSP_SCI_PORT   PORTD
#define HVSP_SCI_DDR    DDRD

#define HVSP_SDO_BIT    1       // 2/SDA input/output connected to Serial Data Output (SDO) pin
#define HVSP_SDO_PIN    PIND
#define HVSP_SDO_PORT   PORTD
#define HVSP_SDO_DDR    DDRD

#define HVSP_SII_BIT    2       // RX output connect to Serial Instruction Input (SII) pin
#define HVSP_SII_PIN    PIND
#define HVSP_SII_PORT   PORTD
#define HVSP_SII_DDR    DDRD

#define HVSP_SDI_BIT    3       // TX output connected to Serial Data Input (SDI) pin
#define HVSP_SDI_PIN    PIND
#define HVSP_SDI_PORT   PORTD
#define HVSP_SDI_DDR    DDRD
 
#define BUTTON_BIT      7       // D11 Special function button: 
#define BUTTON_PIN      PINB    // short press: toggle VCC state
#define BUTTON_PORT     PORTB   // long press: burn fuse settings and/or embedded hexfile
#define BUTTON_DDR      DDRB

#define GREEN_LED_BIT   6       // D10 active low turns LED on. 
#define GREEN_LED_PIN   PINB    // Status LED to tell both VCC and VPP are disabled
#define GREEN_LED_PORT  PORTB
#define GREEN_LED_DDR   DDRB

#define VCC_LED_BIT     5       // D9 active low turns LED on. 
#define VCC_LED_PIN     PINB    // Status LED to tell VCC_WEAK is enabled.
#define VCC_LED_PORT    PORTB   // when a P-Channel MOSFET is used, it also powers on VCC
#define VCC_LED_DDR     DDRB

#define VCC_BIT         6       // A1 active high turns weak VCC on.
#define VCC_PIN         PINF    // this pin powers VCC directly when no MOSFET is used
#define VCC_PORT        PORTF
#define VCC_DDR         DDRF

#define VPP_BIT         5       // A2 active low turns the 12V DC-DC converter on. 
#define VPP_PIN         PINF    // The RED VPP LED is powered by 12V
#define VPP_PORT        PORTF
#define VPP_DDR         DDRF

#define BUILTIN_LED_BIT  7       // D13 Leonardo/Beetle board built in LED  
#define BUILTIN_LED_PIN  PINC    
#define BUILTIN_LED_PORT PORTC   
#define BUILTIN_LED_DDR  DDRC

// pin configure macros
#define HVSP_SCI_AS_INPUT   (HVSP_SCI_DDR  &= ~(1 << HVSP_SCI_BIT))
#define HVSP_SCI_AS_OUTPUT  (HVSP_SCI_DDR  |=  (1 << HVSP_SCI_BIT))

#define HVSP_SDO_AS_INPUT   (HVSP_SDO_DDR  &= ~(1 << HVSP_SDO_BIT))
#define HVSP_SDO_AS_OUTPUT  (HVSP_SDO_DDR  |=  (1 << HVSP_SDO_BIT))
#define HVSP_SDO_PULLUP_ON  (HVSP_SDO_PORT |=  (1 << HVSP_SDO_BIT))
#define HVSP_SDO_PULLUP_OFF (HVSP_SDO_PORT &= ~(1 << HVSP_SDO_BIT))

#define HVSP_SII_AS_INPUT   (HVSP_SII_DDR  &= ~(1 << HVSP_SII_BIT))
#define HVSP_SII_AS_OUTPUT  (HVSP_SII_DDR  |=  (1 << HVSP_SII_BIT))

#define HVSP_SDI_AS_INPUT   (HVSP_SDI_DDR  &= ~(1 << HVSP_SDI_BIT))
#define HVSP_SDI_AS_OUTPUT  (HVSP_SDI_DDR  |=  (1 << HVSP_SDI_BIT))

#define BUTTON_AS_INPUT     (BUTTON_DDR    &= ~(1 << BUTTON_BIT))
#define BUTTON_PULLUP_ON    (BUTTON_PORT   |=  (1 << BUTTON_BIT))
#define GREEN_LED_AS_OUTPUT (GREEN_LED_DDR |=  (1 << GREEN_LED_BIT))
#define VCC_LED_AS_OUTPUT   (VCC_LED_DDR   |=  (1 << VCC_LED_BIT))

#define VCC_AS_INPUT        (VCC_DDR       &= ~(1 << VCC_BIT))
#define VCC_AS_OUTPUT       (VCC_DDR       |=  (1 << VCC_BIT))
#define VPP_AS_OUTPUT       (VPP_DDR       |= (1 << VPP_BIT))

#define BUILTIN_LED_AS_OUTPUT (BUILTIN_LED_DDR |=  (1 << BUILTIN_LED_BIT))

// pin state macros
#define HVSP_SCI_LOW    (HVSP_SCI_PORT &= ~(1 << HVSP_SCI_BIT))
#define HVSP_SCI_HIGH   (HVSP_SCI_PORT |=  (1 << HVSP_SCI_BIT))
#define HVSP_SDO_LOW    (HVSP_SDO_PORT &= ~(1 << HVSP_SDO_BIT))
#define HVSP_SDO_HIGH   (HVSP_SDO_PORT |=  (1 << HVSP_SDO_BIT))
#define HVSP_SII_LOW    (HVSP_SII_PORT &= ~(1 << HVSP_SII_BIT))
#define HVSP_SII_HIGH   (HVSP_SII_PORT |=  (1 << HVSP_SII_BIT))
#define HVSP_SDI_LOW    (HVSP_SDI_PORT &= ~(1 << HVSP_SDI_BIT)) 
#define HVSP_SDI_HIGH   (HVSP_SDI_PORT |=  (1 << HVSP_SDI_BIT)) 

#define GREEN_LED_OFF   (GREEN_LED_PORT  |=  (1 << GREEN_LED_BIT))
#define GREEN_LED_ON    (GREEN_LED_PORT  &= ~(1 << GREEN_LED_BIT))
#define VCC_LED_OFF     (VCC_LED_PORT    |=  (1 << VCC_LED_BIT))
#define VCC_LED_ON      (VCC_LED_PORT    &= ~(1 << VCC_LED_BIT))
#define VCC_OFF         (VCC_PORT        &= ~(1 << VCC_BIT))
#define VCC_ON          (VCC_PORT        |=  (1 << VCC_BIT))
#define VPP_OFF         (VPP_PORT &= ~(1 << VPP_BIT))
#define VPP_ON          (VPP_PORT |= (1 << VPP_BIT))

#define BUILTIN_LED_OFF (BUILTIN_LED_PORT  &= ~(1 << BUILTIN_LED_BIT))
#define BUILTIN_LED_ON  (BUILTIN_LED_PORT  |=  (1 << BUILTIN_LED_BIT))

// pin input macro
#define HVSP_SDO_STATE  (HVSP_SDO_PIN & (1 << HVSP_SDO_BIT))

// vars
bool progMode;          // programmer state
bool keepVccOn;         // Vcc state after ending programmer mode
uint16_t currAddr;      // current flash/EEPROM word address
uint8_t flashPage;      // flash page of current word address

bool buttonState;
bool buttonStateOld;    
uint16_t oldMillis;
/*******************************************************************************/
// low level hardware functions
/*******************************************************************************/

void vccDown()
{
  VCC_AS_INPUT;
  VCC_OFF;
  VCC_LED_AS_OUTPUT;
  VCC_LED_OFF;
  GREEN_LED_AS_OUTPUT;
  GREEN_LED_ON;
}

void vccUp()
{
  VCC_AS_OUTPUT;
  VCC_ON;
  VCC_LED_AS_OUTPUT;
  VCC_LED_ON;
  GREEN_LED_AS_OUTPUT;
  GREEN_LED_OFF;
}

void HVSP_init()
{
  BUTTON_AS_INPUT;
  BUTTON_PULLUP_ON;
  VPP_AS_OUTPUT;
  VPP_OFF;
  HVSP_disable();
  vccDown();
}

void HVSP_enable()
{
  if (progMode) return; //already in programming mode
  
  if (keepVccOn) // Vcc must be off to successfully enter programming mode
  {
    VPP_ON;      // trigger a device reset to wake possible sleeping devices
    delay(25);
    VPP_OFF;
    delay(25);
    vccDown();
    delay(250);  // wait for Vcc to go down.
  }
  HVSP_SCI_AS_OUTPUT;
  HVSP_SCI_LOW;
  // set Prog_enable pins to '000'
  HVSP_SDO_AS_OUTPUT;
  HVSP_SDO_LOW;
  HVSP_SII_AS_OUTPUT;
  HVSP_SII_LOW;
  HVSP_SDI_AS_OUTPUT;
  HVSP_SDI_LOW;
  // Enter High-voltage Serial programming mode
  vccUp();
  delayMicroseconds(20);
  VPP_ON;
  delayMicroseconds(10);
  // release Prog_enable[2] pin
  HVSP_SDO_AS_INPUT; 
  HVSP_SDO_PULLUP_ON; 
  delayMicroseconds(300);
  progMode = true;
  buttonState = false;
}

void HVSP_disable()
{
  if (progMode) while (!HVSP_SDO_STATE); // wait for pending command to finish
  VPP_OFF;
  HVSP_SDI_AS_INPUT;
  HVSP_SDI_LOW;
  HVSP_SII_AS_INPUT;
  HVSP_SII_LOW;
  HVSP_SCI_AS_INPUT;
  HVSP_SCI_LOW;
  HVSP_SDO_PULLUP_OFF; 
  if (!keepVccOn) vccDown(); // Vcc can be optionally kept on for in circuit development
  progMode = false;
}

void clockPulse()
{
  delayMicroseconds(8); // longer idle time here allowing previously set data to stabelize
  HVSP_SCI_HIGH;        // rising edge clocks data in and out      
  delayMicroseconds(2);
  HVSP_SCI_LOW;
}

uint8_t transfer(uint8_t data, uint8_t cmd)
{
  uint8_t result = 0;
  while (!HVSP_SDO_STATE); // wait for chip ready
  // start bits
  HVSP_SDI_LOW;
  HVSP_SII_LOW;
  clockPulse();
  // 8-bit shifter
  for (uint8_t mask = 128; mask != 0; mask >>= 1)
  {
    HVSP_SDI_LOW;
    if (data & mask) HVSP_SDI_HIGH;
    HVSP_SII_LOW;      
    if (cmd & mask)  HVSP_SII_HIGH;
    if (HVSP_SDO_STATE) result |= mask;
    clockPulse();
  }
  // double stop bits
  HVSP_SDI_LOW;
  HVSP_SII_LOW;
  clockPulse();
  clockPulse();
  return result;
}

/*******************************************************************************/
// HVSP device commands
/*******************************************************************************/

uint8_t readSignatureByte(uint8_t idx)
{
  transfer(0x08, 0x4C);
  transfer(idx, 0x0C);
  transfer(0x00, 0x68);
  return transfer(0x00, 0x6c);
}

void chipErase()
{
  transfer(0x80, 0x4C);
  transfer(0x00, 0x64);
  transfer(0x00, 0x6C);
}

void readFlashMode()
{
  transfer(0x02, 0x4C);
} 

uint8_t readFlashLowByte()
{
  transfer(currAddr, 0x0C);
  transfer(currAddr >> 8, 0x1C);
  transfer(0x00, 0x68);
  return transfer(0x00, 0x6C);
}

uint8_t readFlashHighByte()
{
  transfer(0x00, 0x6C);
  transfer(0x00, 0x78);
  uint8_t result = transfer(0x00, 0x7C);
  transfer(0x00, 0x7C);
  return result;
}

void writeFlashMode()
{
  transfer(0x10, 0x4C);  
}

void loadFlashPageLowByte(uint8_t b) 
{
  flashPage = currAddr >> 8;
  transfer(currAddr & 0xFF, 0x0C);
  transfer(b, 0x2C);
}
  
void loadFlashPageHighByte(uint8_t b) 
{
  transfer(b, 0x3C);
  transfer(0x00, 0x7D);
  transfer(0x00, 0x7C);
}

void writeFlashPage()
{
  transfer(flashPage, 0x1C);
  transfer(0x00, 0x64);
  transfer(0x00, 0x6C);
}

void readEepromMode()
{
  transfer(0x03, 0x4C);
} 

uint8_t readEepromByte()
{
  return readFlashLowByte();
}

void writeEepromMode()
{
  transfer(0x11, 0x4C);
}

// EEPROM can be programmed faster by loading a page and then programming it
 
void loadEepromPage(uint8_t b)
{
  transfer(currAddr, 0x0C);
  transfer(currAddr >> 8, 0x1C);
  transfer(b, 0x2C);
  transfer(0x00, 0x6D);
  transfer(0x00, 0x6C);
}

void writeEepromPage()
{
  transfer(0x00, 0x64);
  transfer(0x00, 0x6C);
}

// program a single byte directly to EEPROM
 
void writeEepromByte(uint8_t b)
{
  transfer(currAddr, 0x0C);
  transfer(currAddr >> 8, 0x1C);
  transfer(b, 0x2C);
  transfer(0x00, 0x6D);
  transfer(0x00, 0x64);
  transfer(0x00, 0x6C);
}

uint8_t readLowFuse()
{
  transfer(0x04, 0x4C);
  transfer(0x00, 0x68);
  return transfer(0x00, 0x6C);
}
void writeLowFuse(uint8_t fuse)
{
   transfer(0x40, 0x4C);
   transfer(fuse, 0x2C);
   transfer(0x00, 0x64);
   transfer(0x00, 0x6C);
}

uint8_t readHighFuse()
{
  transfer(0x04, 0x4C);
  transfer(0x00, 0x7A);
  return transfer(0x00, 0x7E);
}

void writeHighFuse(uint8_t fuse)
{
  transfer(0x40, 0x4C);
  transfer(fuse, 0x2C);
  transfer(0x00, 0x74);
  transfer(0x00, 0x7C);
}

uint8_t readExtFuse()
{
    transfer(0x04, 0x4C);
    transfer(0x00, 0x6A);
    return transfer(0x00, 0x6E);
}

void writeExtFuse(uint8_t fuse)
{
  transfer(0x40, 0x4C);
  transfer(fuse, 0x2C);
  transfer(0x00, 0x66);
  transfer(0x00, 0x6e);
}

uint8_t readLockBits()
{
  transfer(0x04, 0x4C);
  transfer(0x00, 0x78);
  return transfer(0x00, 0x7C);
}

void writeLockBits(uint8_t fuse)
{
  transfer(0x20, 0x4C);
  transfer(fuse, 0x2C);
  transfer(0x00, 0x64);
  transfer(0x00, 0x6c);
}

/******************************************************************************/
// Arduino main functions
/******************************************************************************/

void setup()
{ 
  HVSP_init();
  Serial.begin(115200);
}

void loop()
{
  // serial handled by programmer 
  if (Serial.available() > 0)
  { 
    BUILTIN_LED_ON;
    stk500Programmer();
    BUILTIN_LED_OFF;
  }
  // button feature only available when not in programming mode
  if (!progMode) 
  {
    buttonStateOld = buttonState;
    buttonState = !(BUTTON_PIN & (1 << BUTTON_BIT));
    if ((!buttonStateOld) && buttonState) //just pressed
      oldMillis = (uint16_t) millis();
    if (buttonStateOld && (!buttonState)) //just released
    {
      uint16_t duration;
      uint16_t now = (uint16_t) millis();
      if (now > oldMillis) duration = now - oldMillis;
      else duration = oldMillis - now;
      if (duration > 20) // debounce time
      {
        if (duration < 500) //short press: toggle Vcc
        {
          keepVccOn = !keepVccOn;
          if (keepVccOn) vccUp();
          else vccDown();
        }
        else //long press : Set factory defaults
        {
          //TO DO
        }
      }
    }
  }
}
