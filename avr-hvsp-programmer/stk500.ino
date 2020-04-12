/******************************************************************************/
// AVR HVSP stk500v1 lite core  1.01 by Mr.Blinky Mar.2020 CC0 Licence
/*******************************************************************************

based on Atmel application note AN_2525 AVR061: STK500 Communication Protocol,
analyzing USB communications and peeking at AVRdude source.

lite as in limited commands set implemented to support AVRdude thru Arduino IDE
and command line:

* read device ID
* read fuses and lock bits
* chip erase
* read/write flash
* read/write EEPROM (using commandline)

Things learned during developing and debugging this code:

When selecting stk500v1 programmer option with AVRdude using the -cstk500v1 switch
DTR is not set. The Arduino USB core requires DTR to be set otherwise no data is
send to the host. Initally a hack was required but later on I discovered the
Arduino programmer (-carduino switch) basically uses the same protocol and sets
DTR so the hack was no longer required.

TLDR

Use AVRdude with -carduino switch

/******************************************************************************/

// responses
constexpr uint8_t Resp_STK_OK       = 0x10;
constexpr uint8_t Resp_STK_FAILED   = 0x11;
constexpr uint8_t Resp_STK_UNKNOWN  = 0x12;
constexpr uint8_t Resp_STK_NODEVICE = 0x13;
constexpr uint8_t Resp_STK_INSYNC   = 0x14;
constexpr uint8_t Resp_STK_NOSYNC   = 0x15;

// special
constexpr uint8_t Sync_STK_CRC_EOP  = 0x20;

// commands
constexpr uint8_t Cmnd_STK_GET_SYNC         = 0x30;
constexpr uint8_t Cmnd_STK_GET_SIGN_ON      = 0x31;
constexpr uint8_t Cmnd_STK_RESET            = 0x32;
constexpr uint8_t Cmnd_STK_SINGLE_CLOCK     = 0x33;
constexpr uint8_t Cmnd_STK_STORE_PARAMETERS = 0x34;

constexpr uint8_t Cmnd_STK_SET_PARAMETER    = 0x40;
constexpr uint8_t Cmnd_STK_GET_PARAMETER    = 0x41;
constexpr uint8_t Cmnd_STK_SET_DEVICE       = 0x42;
constexpr uint8_t Cmnd_STK_GET_DEVICE       = 0x43;
constexpr uint8_t Cmnd_STK_GET_STATUS       = 0x44;
constexpr uint8_t Cmnd_STK_SET_DEVICE_EXT   = 0x45;

constexpr uint8_t Cmnd_STK_ENTER_PROGMODE   = 0x50;
constexpr uint8_t Cmnd_STK_LEAVE_PROGMODE   = 0x51;
constexpr uint8_t Cmnd_STK_CHIP_ERASE       = 0x52;
constexpr uint8_t Cmnd_STK_CHECK_AUTOINC    = 0x53;
constexpr uint8_t Cmnd_STK_CHECK_DEVICE     = 0x54;
constexpr uint8_t Cmnd_STK_LOAD_ADDRESS     = 0x55;
constexpr uint8_t Cmnd_STK_UNIVERSAL        = 0x56;

constexpr uint8_t Cmnd_STK_PROG_FLASH       = 0x60;
constexpr uint8_t Cmnd_STK_PROG_DATA        = 0x61;
constexpr uint8_t Cmnd_STK_PROG_FUSE        = 0x62;
constexpr uint8_t Cmnd_STK_PROG_LOCK        = 0x63;
constexpr uint8_t Cmnd_STK_PROG_PAGE        = 0x64;
constexpr uint8_t Cmnd_STK_PROG_FUSE_EXT    = 0x65;

constexpr uint8_t Cmnd_STK_READ_FLASH       = 0x70;
constexpr uint8_t Cmnd_STK_READ_DATA        = 0x71;
constexpr uint8_t Cmnd_STK_READ_FUSE        = 0x72;
constexpr uint8_t Cmnd_STK_READ_LOCK        = 0x73;
constexpr uint8_t Cmnd_STK_READ_PAGE        = 0x74;
constexpr uint8_t Cmnd_STK_READ_SIGN        = 0x75;
constexpr uint8_t Cmnd_STK_READ_OSCCAL      = 0x76;
constexpr uint8_t Cmnd_STK_READ_FUSE_EXT    = 0x77;
constexpr uint8_t Cmnd_STK_READ_OSCCAL_EXT  = 0x78;

// parameters
constexpr uint8_t Parm_STK_HW_VER           = 0x80;
constexpr uint8_t Parm_STK_SW_MAJOR         = 0x81;
constexpr uint8_t Parm_STK_SW_MINOR         = 0x82;

// parameter structure
struct Params
{
  uint8_t devicecode;
  uint8_t revision;
  uint8_t progtype;
  uint8_t parmode;
  uint8_t polling;
  uint8_t selftimed;
  uint8_t lockbytes;
  uint8_t fusebytes;
  uint8_t flashpollval1;
  uint8_t flashpollval2;
  uint8_t eeprompollval1;
  uint8_t eeprompollval2;
  uint8_t pagesizehigh;     // 16-bit big endian flashmemory programming pagesize in bytes
  uint8_t pagesizelow;
  uint8_t eepromsizehigh;   // 16-bit big endian eeprom size in bytes
  uint8_t eepromsizelow;
  uint8_t flashsize4;       // 32-bit big endian flash size in bytes
  uint8_t flashsize3;
  uint8_t flashsize2;
  uint8_t flashsize1;
};

Params    params;
uint8_t   eepromPageSize;

// low level serial functions

uint8_t readByte()
{
  while (Serial.available() <= 0);
  return Serial.read();
}

void writeByte(uint8_t b)
{
  Serial.write(b);
}

// STK functions

void writeNoSync()
{
  writeByte(Resp_STK_NOSYNC);
}

void writeInSync()
{
  writeByte(Resp_STK_INSYNC);
}

void writeOK()
{
  writeByte(Resp_STK_OK);
}

bool endOfPacket()
{
  if (readByte() == Sync_STK_CRC_EOP) return true;

  writeNoSync();
  return false;
}

void endOfCommand()
{
  if (endOfPacket())
  {
    writeInSync();
    writeOK();
  }
}

void getParameter()
{
  uint8_t b = readByte();
  if      (b == Parm_STK_HW_VER)   b = HARDWAREVERSION;
  else if (b == Parm_STK_SW_MAJOR) b = SOFTWAREVERSION_MAJOR;
  else if (b == Parm_STK_SW_MINOR) b = SOFTWAREVERSION_MINOR;
  else b = 0;
  if (endOfPacket())
  {
    writeInSync();
    writeByte(b);
    writeOK();
  }
}

void setDeviceParams()
{
  for(uint8_t i = 0; i < sizeof(params); i++)
    *(uint8_t*)(&params + i) = readByte();
  endOfCommand();
}
void setDeviceExtParams()
{
  uint8_t len = readByte();
  eepromPageSize = readByte(); // only required ext device param
  for(uint8_t i = 2; i < len; i++) readByte(); // ignore all others
  endOfCommand();
}

// AVRdude uses universal (SPI)command rather than the STK commands for:
// read signature, read/write fuses, chip erase

void universalCommand()
{
  uint16_t cmd = (readByte() << 8) | readByte();
  uint8_t idx = readByte();
  uint8_t data =readByte();
  if (endOfPacket())
  {
    writeInSync();
    switch (cmd)
    {
      case 0x3000:                      // prefered by stk500v1 programmer
        data = readSignatureByte(idx);
        break;
      case 0x5000:
      data = readLowFuse();
        break;
      case 0x5008:
      data = readExtFuse();
        break;
      case 0x5800:
        data = readLockBits();
        break;
      case 0x5808:
        data = readHighFuse();
        break;
      case 0xAC80:
        chipErase();
        break;
      case 0xACA0:
        writeLowFuse(data);
        break;
      case 0xACA4:
        writeExtFuse(data);
        break;
      case 0xACA8:
        writeHighFuse(data);
        break;
      case 0xACE0:
        writeLockBits(data);
        break;
    }
    writeByte(data);
    writeOK();
  }
}

void readPage()
{
  int16_t len = (readByte() << 8) | readByte();
  uint8_t memType = readByte();
  if (endOfPacket())
  {
    writeInSync();
    if (memType == 'E')
    {
      readEepromMode();
      currAddr <<= 1;
      while (len--)
      {
        writeByte(readEepromByte());
        ++currAddr;
      }
      currAddr >>= 1;
    }
    else
    {
      readFlashMode();
      while (len--)
      {
        writeByte(readFlashLowByte());
        if (len)
        {
          writeByte(readFlashHighByte());
          --len;
        }
        ++currAddr;
      }
    }
    writeOK();
  }
}

void writePage()
{
  int16_t len = (readByte() << 8) | readByte();
  uint8_t memType = readByte();
    if (memType == 'E')
    {
      writeEepromMode();
      uint16_t pageMask = eepromPageSize - 1;
      currAddr <<= 1;
      while (len--)
      {
        loadEepromPage(readByte());
        currAddr++;
        if (!(currAddr & (pageMask))) writeEepromPage();
      }
      if (currAddr & (pageMask)) writeEepromPage();
      currAddr >>= 1;
    }
    else
    {
      writeFlashMode();
      uint16_t pageMask = ((params.pagesizehigh << 7) | (params.pagesizelow >> 1)) - 1;
      while (len)
      {
        loadFlashPageLowByte(readByte());
        loadFlashPageHighByte(readByte());
        currAddr++;
        if (!(currAddr & pageMask)) writeFlashPage();
        len -= 2;
      }
      if (currAddr & pageMask) writeFlashPage();
    }
  endOfCommand();
}

void stk500Programmer()
{
  switch(readByte())
  {
    case Cmnd_STK_GET_SYNC:
      endOfCommand();
      break;

    case Cmnd_STK_GET_PARAMETER:
      getParameter();
      break;

    case Cmnd_STK_SET_DEVICE:
      setDeviceParams();
      break;

    case Cmnd_STK_SET_DEVICE_EXT:
      setDeviceExtParams();
      break;

    case Cmnd_STK_ENTER_PROGMODE:
      HVSP_enable();
      endOfCommand();
      break;

    case Cmnd_STK_LEAVE_PROGMODE:
      HVSP_disable();
      endOfCommand();
      break;

    case Cmnd_STK_READ_SIGN: // prefered by arduino programmer
      if (endOfPacket())
      {
        writeInSync();
        for(uint8_t i = 0; i <3; i++) writeByte(readSignatureByte(i));
        writeOK();
      }
      break;

    case Cmnd_STK_UNIVERSAL:
      universalCommand();
      break;

    case Cmnd_STK_LOAD_ADDRESS:  // set flash/eeprom word address
      currAddr = readByte() | (readByte() << 8);
      endOfCommand();
      break;

    case Cmnd_STK_READ_PAGE:
      readPage();
      break;

    case Cmnd_STK_PROG_PAGE:
      writePage();
      break;

    default:
      if(endOfPacket()) writeByte(Resp_STK_UNKNOWN);
  }
}