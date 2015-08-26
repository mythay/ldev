#include "ldev_serial.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

int ToBaudConstant(int baudRate) {
  switch (baudRate) {
    case 0: return B0;
    case 50: return B50;
    case 75: return B75;
    case 110: return B110;
    case 134: return B134;
    case 150: return B150;
    case 200: return B200;
    case 300: return B300;
    case 600: return B600;
    case 1200: return B1200;
    case 1800: return B1800;
    case 2400: return B2400;
    case 4800: return B4800;
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
#if defined(__linux__)
    case 460800: return B460800;
    case 500000: return B500000;
    case 576000: return B576000;
    case 921600: return B921600;
    case 1000000: return B1000000;
    case 1152000: return B1152000;
    case 1500000: return B1500000;
    case 2000000: return B2000000;
    case 2500000: return B2500000;
    case 3000000: return B3000000;
    case 3500000: return B3500000;
    case 4000000: return B4000000;
#endif
  }
  return -1;
}
static int ToDataBitsConstant(int dataBits) {
  switch (dataBits) {
    case 8: default: return CS8;
    case 7: return CS7;
    case 6: return CS6;
    case 5: return CS5;
  }
  return -1;
}

int ldev_serial_open(const char* fpath, SERIALPORT_FD_OPT * data)
{
    int fd;
    int flags;
    struct termios options;
    int baudRate = ToBaudConstant(data->baudRate);
    int dataBits = ToDataBitsConstant(data->dataBits);
    if(dataBits == -1) {
        return -1;
    }
    flags = (O_RDWR | O_NOCTTY | O_NONBLOCK | O_CLOEXEC | O_SYNC);
    if(data->hupcl == 0) {
      flags &= ~HUPCL;
    }
    fd = open(fpath, flags);
    if (fd == -1) {
        return -1;
    }

    // struct sigaction saio;
    // saio.sa_handler = sigio_handler;
    // sigemptyset(&saio.sa_mask);
    // saio.sa_flags = 0;
    // sigaction(SIGIO, &saio, NULL);

    // //all process to receive SIGIO
    // fcntl(fd, F_SETOWN, getpid());
    // int flflags = fcntl(fd, F_GETFL);
    // fcntl(fd, F_SETFL, flflags | FNONBLOCK);

    // Set baud and other configuration.
    tcgetattr(fd, &options);

    // Removing check for valid BaudRates due to ticket: #140
    // #if not ( defined(MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4) )
    // Specify the baud rate


    // On linux you can alter the meaning of B38400 to mean a custom baudrate...
    if (baudRate != -1) {
        cfsetispeed(&options, baudRate);
        cfsetospeed(&options, baudRate);
    }

    // Removing check for valid BaudRates due to ticket: #140
    // #endif

    /*
    IGNPAR  : ignore bytes with parity errors
    */
    options.c_iflag = IGNPAR;

    /*
    ICRNL   : map CR to NL (otherwise a CR input on the other computer
              will not terminate input)
    */
    // Pulling this for now. It should be an option, however. -Giseburt
    //options.c_iflag = ICRNL;

    //  otherwise make device raw (no other input processing)


    // Specify data bits
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= dataBits;

    options.c_cflag &= ~(CRTSCTS);

    if (data->rtscts) {
    options.c_cflag |= CRTSCTS;
    // evaluate specific flow control options
    }

    options.c_iflag &= ~(IXON | IXOFF | IXANY);

    if (data->xon) {
        options.c_iflag |= IXON;
    }

    if (data->xoff) {
        options.c_iflag |= IXOFF;
    }

    if (data->xany) {
        options.c_iflag |= IXANY;
    }


    switch (data->parity)
    {
        case SERIALPORT_PARITY_NONE:
            options.c_cflag &= ~PARENB;
            // options.c_cflag &= ~CSTOPB;
            // options.c_cflag &= ~CSIZE;
            // options.c_cflag |= CS8;
            break;
        case SERIALPORT_PARITY_ODD:
            options.c_cflag |= PARENB;
            options.c_cflag |= PARODD;
            // options.c_cflag &= ~CSTOPB;
            // options.c_cflag &= ~CSIZE;
            // options.c_cflag |= CS7;
            break;
        case SERIALPORT_PARITY_EVEN:
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            // options.c_cflag &= ~CSTOPB;
            // options.c_cflag &= ~CSIZE;
            // options.c_cflag |= CS7;
            break;
        default:
            close(fd);
            return -1;
    }

    switch(data->stopBits) {
        case SERIALPORT_STOPBITS_ONE:
            options.c_cflag &= ~CSTOPB;
            break;
        case SERIALPORT_STOPBITS_TWO:
            options.c_cflag |= CSTOPB;
            break;
        default:
            close(fd);
            return -1;
    }

    options.c_cflag |= CLOCAL; //ignore status lines
    options.c_cflag |= CREAD;  //enable receiver
    options.c_cflag |= HUPCL;  //drop DTR (i.e. hangup) on close

    // Raw output
    options.c_oflag = 0;

    // ICANON makes partial lines not readable. It should be otional.
    // It works with ICRNL. -Giseburt
    options.c_lflag = 0; //ICANON;

#if 1    
    options.c_cc[VMIN]= 1;
    options.c_cc[VTIME]= 0;
#endif    
    // removed this unneeded sleep.
    // sleep(1);
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &options);

    // On OS X, starting in Tiger, we can set a custom baud rate, as follows:

    return fd;
}

