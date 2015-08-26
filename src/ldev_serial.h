#ifndef __LDEV_SERIAL_H__
#define __LDEV_SERIAL_H__


typedef enum  {
  SERIALPORT_PARITY_NONE = 1,
  SERIALPORT_PARITY_MARK = 2,
  SERIALPORT_PARITY_EVEN = 3,
  SERIALPORT_PARITY_ODD = 4,
  SERIALPORT_PARITY_SPACE = 5
}SERIALPORT_PARITY;

typedef enum {
  SERIALPORT_STOPBITS_ONE = 1,
  SERIALPORT_STOPBITS_ONE_FIVE = 2,
  SERIALPORT_STOPBITS_TWO = 3
}SERIALPORT_STOPBITS;


typedef struct {
  int baudRate;
  int dataBits;
  int bufferSize;
  int rtscts;
  int xon;
  int xoff;
  int xany;
  int dsrdtr;
  int hupcl;
  SERIALPORT_PARITY parity;
  SERIALPORT_STOPBITS stopBits;
}SERIALPORT_FD_OPT;

int ldev_serial_open(const char* fpath,SERIALPORT_FD_OPT * data);


#endif
