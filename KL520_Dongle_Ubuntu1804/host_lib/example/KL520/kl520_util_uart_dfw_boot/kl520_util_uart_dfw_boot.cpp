#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <stdint.h>
//#include "kdp2_core.h"
//#include "kdp2_postprocess.h"

//#include "helper_functions.h"

#include <time.h>
#include "xmodem.h"
#include "crc16.h"
#ifdef __linux__
#include <errno.h>
#include <fcntl.h> 
#include <termios.h>
#define com_port "/dev/ttyUSB1"
#elif _WIN32
#include <windows.h>
#define com_port "\\\\.\\COM7"
#endif

#define DFW_MINION_FILE ("../../app_binaries/KL520/dfw/minions.bin")
#define DFW_SCPU_FILE ("../../app_binaries/KL520/dfw/fw_scpu.bin")
#define DFW_NCPU_FILE ("../../app_binaries/KL520/dfw/fw_ncpu.bin")
#define DFW_MINION_SIZE (16 * 1024)
#define DFW_FW_SIZE (128 * 1024)
#define UART_FIFO_SIZE 4096
#define READ_TIMEOUT 10      // milliseconds
#define MSG_HDR_CMD     0xA583
#define	MSG_HDR_RSP     0x8A35
#define MSG_HDR_SIZE    16  // includes both MsgHdr and CmdPram addr & len
#define MAX_UART_READ_RETRY 1000
//#define DEBUG_MSG

#define SCPU_START_ADDRESS          0x10102000
#define NCPU_START_ADDRESS          0x28000000

typedef struct {
     uint16_t preamble;
     uint16_t crc16;
     uint32_t cmd;
     uint32_t addr;
     uint32_t len;
} __attribute__((packed)) MsgHdr;

typedef enum {
    CMD_NONE = 0,
    CMD_MEM_READ,
    CMD_MEM_WRITE,
    CMD_DATA,
    CMD_ACK,
    CMD_STS_CLR,
    CMD_MEM_CLR,
    CMD_CRC_ERR,
    CMD_TEST_ECHO,

    CMD_DEMO_SET_MODEL = 0x10,
    CMD_DEMO_SET_IMAGES,
    CMD_DEMO_RUN_ONCE,
    CMD_DEMO_RUN,
    CMD_DEMO_STOP,
    CMD_DEMO_RESULT_LEN,
    CMD_DEMO_RESULT,    // Response/report to host
    CMD_DEMO_FD,
    CMD_DEMO_LM,        // for development (remove after merge LM, LIVESS, FR to CMD_DEMO_RUN)
    CMD_DEMO_LD,        //
    CMD_DEMO_FR,        //

    CMD_DEMO_INFERENCE = 100, //=0x64
    CMD_DEMO_REG1,
    CMD_DEMO_REG2,
    CMD_DEMO_REG3,
    CMD_DEMO_REG4,
    CMD_DEMO_REG5,
    CMD_DEMO_ADDUSER,
    CMD_DEMO_DELUSER,
    CMD_DEMO_ABORT_REG,

    CMD_DEMO_TINY_YOLO = 110, //=0x6e

    // Flash command
    CMD_FLASH_INFO = 0x1000,
    CMD_FLASH_CHIP_ERASE,
    CMD_FLASH_SECTOR_ERASE,
    CMD_FLASH_READ,
    CMD_FLASH_WRITE,
    CMD_SCPU_RUN,
} Cmd;

using std::cout;
using std::cin;
using std::endl;
using std::string;

bool fWaitingOnRead;
#ifdef __linux__
struct termios hComm;
int fd;
#elif _WIN32
HANDLE hComm;
DCB dcbOri;
#endif

static void delay(int milli_seconds)
{
    if (milli_seconds == 0) return;
    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

static char *read_file_to_buffer(const char *file_path, long *buffer_size)
{
    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        printf("%s(): fopen failed, file:%s, %s\n", __FUNCTION__, file_path, strerror(errno));
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file); //get the size

    *buffer_size = file_size;

    char *buffer = (char *)malloc(file_size);

    fseek(file, 0, SEEK_SET); //move to begining

    size_t read_size = fread(buffer, 1, file_size, file);
    if (read_size != (size_t)file_size)
    {
        printf("%s(): fread failed, file size: %u, read size %u\n", __FUNCTION__,
               (unsigned int)file_size, (unsigned int)read_size);
        free(buffer);
        buffer = NULL;
        *buffer_size = 0;
    }

    fclose(file);

    return buffer;
}

// Writes bytes to the serial port, returning 0 on success and -1 on failure.
#ifdef __linux__
int write_port(termios port, uint8_t * buffer, size_t size)
#else
int write_port(HANDLE port, uint8_t * buffer, size_t size)
#endif
{
#ifdef __linux__
    if(fd < 0) {
        printf("com dev is not open.\n");
        return -1;
    }

    if(size < 1 || buffer == NULL) {
        printf("invalid msg buffer to send.\n");
        return -1;
    }

    //kn_dump_msg((int8_t* )buffer, size, true);

    int bytes_left = size;
    int8_t* ptr = (int8_t* )buffer;
    while (bytes_left > 0) {
        int n = write(fd, ptr, bytes_left);
        if(n <= 0) {
            if (errno == EINTR) {
                printf("sending msg got an interrupt, retrying.\n");
                continue;
            } else if(errno == EAGAIN) {
                printf("sending msg got an eagain, retrying.\n");
                usleep(1000);
                continue;
            } else {
                printf("sending msg got unexpected exception, aborted.\n");
                return -1;
            }
        }
        bytes_left -= n;
        ptr += n;
    }

#elif _WIN32
    DWORD written;
    BOOL success = WriteFile(port, buffer, size, &written, NULL);
    if (!success)
    {
        printf("Failed to write to port");
        return -1;
    }
    if (written != size)
    {
        printf("Failed to write all bytes to port");
        return -1;
    }
#endif
    return 0;
}

// Reads bytes from the serial port.
// Returns after all the desired bytes have been read, or if there is a
// timeout or other error.
// Returns the number of bytes successfully read into the buffer, or -1 if
// there was an error reading.
#ifdef __linux__
uint32_t read_port(termios port, uint8_t * buffer, size_t size)
#else
SSIZE_T read_port(HANDLE port, uint8_t * buffer, size_t size)
#endif
{
#ifdef __linux__
    if(fd < 0) {
        printf("com dev is not open.\n");
        return 0;
    }

    if(size < 1 || buffer == NULL) {
        printf("invalid msg buffer to receive.\n");
        return 0;
    }

    uint32_t res = read(fd, buffer, sizeof(buffer));
    if (res <= 0) {
        printf("Failed to read from uart with res [%d]", res);
        return 0;
    }
    return res;
#elif _WIN32
    DWORD received;
    BOOL success = ReadFile(port, buffer, size, &received, NULL);
    if (!success)
    {
        printf("Failed to read from port");
        return 0;
    }
    return received;
#endif
}
/**
 * \brief _inbyte consume 1 byte from serial port
 * \param timeout timeout in miliseconds
 * \return consumed byte or -1 in case of error
 */
int _inbyte(unsigned short timeout)
{
    char c = 0;

    if (read_port(hComm, (uint8_t *)&c, 1)) {
#ifdef DEBUG_MSG
        printf("\n%d\n", c);
#endif
        return c;
    }
    return -1;
}

/**
 * \brief _outbyte write 1 byte to the serial port
 * \param c byte to write
 */
void _outbyte(int c)
{
#ifdef DEBUG_MSG
    printf("%x-", c);
#endif
    write_port(hComm,(uint8_t *)&c,1);
}

int com_port_set_baudrate(int br)
{
#ifdef __linux__
    int SUPPORTED_SPEED[] = {921600, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300};
    speed_t SPEED_ATTR[] = {B921600, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300};

    int size = sizeof(SUPPORTED_SPEED) / sizeof(int);

    for (int i = 0; i < size; i++) {
        // get user baud rate & set option
        if (br == SUPPORTED_SPEED[i]) {
            cfsetispeed (&hComm, SPEED_ATTR[i]);  // set UART input baud rate
            cfsetospeed (&hComm, SPEED_ATTR[i]);  // set UART output baud rate
            int status = tcsetattr (fd, TCSANOW, &hComm); // set option and work now
            if (status != 0) {
                printf("calling tcsetattr to set baud rate falied :%s.\n", strerror (errno));
                tcflush (fd, TCIOFLUSH);                // clear I/O buffer
                return 0;
            }
            tcflush (fd, TCIOFLUSH);                // clear I/O buffer
            printf("set uart baudrate to:%d..\n", br);
            return 1;
        }
    }
#elif _WIN32
    dcbOri.BaudRate = br;
    bool fSuccess = SetCommState(hComm, &dcbOri);
    if (!fSuccess) {
        printf("SetCommState 115200 fail!!\n");
        return fSuccess;
    }
    else {
        printf("SetCommState 115200 successful!\n");
    }
#endif
    return 1;
}
int com_port_open()
{
#ifdef __linux__
    fd = open(com_port, O_RDWR | O_NOCTTY);
    if(fd == -1)
    {
        printf("open_port: Unable to open %s",com_port);
        return 0;
    }

    tcgetattr(fd, &hComm);

    cfsetispeed(&hComm, B115200);
    cfsetospeed(&hComm, B115200);

    hComm.c_cflag |= (CLOCAL | CREAD);
    hComm.c_cflag &= ~PARENB;
    hComm.c_cflag &= ~CSTOPB;
    hComm.c_cflag &= ~CSIZE;
    hComm.c_cflag |= CS8;
    hComm.c_cflag &= ~CRTSCTS;
    hComm.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    hComm.c_iflag &= ~INPCK;
    hComm.c_iflag &= ~(IXON | IXOFF | IXANY);
    hComm.c_oflag &= ~OPOST;

    hComm.c_cc[VMIN] = 200;
    hComm.c_cc[VTIME] = 1;    // time base 0.1s

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &hComm);
    return 1;
#elif _WIN32
    hComm = CreateFileA(com_port,                //port name
                        GENERIC_READ | GENERIC_WRITE, //Read/Write
                        0,                            // No Sharing
                        NULL,                         // No Security
                        OPEN_EXISTING,// Open existing port only
                        0,            // Non Overlapped I/O
                        NULL);        // Null for Comm Devices

    if (hComm == INVALID_HANDLE_VALUE) {
        printf("Error in opening serial port");
        return 0;
    }
    else
    {
        printf("opening serial port successful");

        //get initial state
        bool fSuccess;
        fSuccess = GetCommState(hComm, &dcbOri);
        if (!fSuccess) {
            printf("GetCommState fail!!\n");
            return fSuccess;
        }
        else {
            printf("GetCommState successful!\n");
        }

        dcbOri.BaudRate = 115200;
        dcbOri.Parity = NOPARITY;
        dcbOri.ByteSize = (BYTE)8;
        dcbOri.StopBits = ONESTOPBIT;
        dcbOri.fOutxCtsFlow = false;
        dcbOri.fOutxDsrFlow = false;
        dcbOri.fOutX = false;
        dcbOri.fDtrControl = DTR_CONTROL_DISABLE;
        dcbOri.fRtsControl = RTS_CONTROL_DISABLE;
        fSuccess = SetCommState(hComm, &dcbOri);
        delay(600);
        if (!fSuccess) {
            printf("SetCommState 115200 fail!!\n");
            return fSuccess;
        }
        else {
            printf("SetCommState 115200 successful!\n");
        }

        fSuccess = GetCommState(hComm, &dcbOri);
        if (!fSuccess) {
            printf("GetCommState fail!!\n");
            return fSuccess;
        }
        else {
            printf("GetCommState successful!\n");
        }

        COMMTIMEOUTS timeouts={0};
        timeouts.ReadIntervalTimeout=50;
        timeouts.ReadTotalTimeoutConstant=50;
        timeouts.ReadTotalTimeoutMultiplier=10;
        timeouts.WriteTotalTimeoutConstant=50;
        timeouts.WriteTotalTimeoutMultiplier=10;
        fSuccess = SetCommTimeouts(hComm, &timeouts);
        if(!SetCommTimeouts(hComm, &timeouts)) {
            printf("SetCommTimeouts fail\n");//error occureed. Inform user
            return fSuccess;
        }
    }
    return 1;
#endif
}
void com_port_close()
{
#ifdef __linux__
	if(fd > 0)
		close (fd);
#elif _WIN32
	if(hComm > 0)
		CloseHandle(hComm);//Closing the Serial Port
#endif
}
void com_port_file_send(uint32_t cmd, char* p_buf, long file_size, uint32_t index)
{
    MsgHdr msghdr;
    uint8_t fw_buf[UART_FIFO_SIZE+MSG_HDR_SIZE];
    char c[MSG_HDR_SIZE]={};
    int dfw_count = file_size / UART_FIFO_SIZE;
	uint32_t len = file_size<UART_FIFO_SIZE ? file_size:UART_FIFO_SIZE;
	//printf("******  unit length %d  ******\n",len);
    if(((file_size % UART_FIFO_SIZE)!=0)||(file_size==0))
        dfw_count++;
    for(int repeat=0; repeat < dfw_count; repeat++)
    {
        #ifdef DEBUG_MSG
        printf("=========================================\n");
        #endif
        memcpy(&fw_buf[MSG_HDR_SIZE], (p_buf+(repeat*UART_FIFO_SIZE)), len);
        msghdr.preamble=MSG_HDR_CMD;
        msghdr.crc16 = 0;
        msghdr.cmd = cmd;
        msghdr.addr = index+(repeat*UART_FIFO_SIZE);
        msghdr.len = len;
        memcpy(&fw_buf[0],&msghdr, MSG_HDR_SIZE);
        msghdr.crc16 = gen_crc16(&fw_buf[4],(len+MSG_HDR_SIZE-4));
        memcpy(&fw_buf[2],&msghdr.crc16, 2);
        #ifdef DEBUG_MSG
        for(int i=0; i<MSG_HDR_SIZE; i++)
        {
            printf("%X ",fw_buf[i]);
            if((i!=0)&&((i%32)==0))
                printf("\n");
        }
        #endif
        write_port(hComm, (uint8_t *)&fw_buf[0], len+MSG_HDR_SIZE);
        #ifdef DEBUG_MSG
        printf("\n=========================================\n");
        #endif
        while(1) //wait ACK
        {
            if(read_port(hComm, (uint8_t *)&c[0], 100))
            {
                #ifdef DEBUG_MSG
                for(int i=0; i<MSG_HDR_SIZE; i++)
                    printf("%X ",(uint8_t)c[i]);
                #endif
                if(c[4]==CMD_ACK)
                    break;
            }
        }
        #ifdef DEBUG_MSG
        printf("\n=========================================\n");
        #endif
    }
}

int main()
{
    if(com_port_open())
    {
        #ifdef DEBUG_MSG
        printf("SetCommTimeouts successful\n");
        #endif

        printf("Please press RESET btn!!......\n");
        while(1)
        {
            char c[1000]={};
            char *cc = &c[0];
            bool fSuccess;
            //read_port(hComm, &cc, 100); // read a char
            if(read_port(hComm, (uint8_t *)cc, 100)) printf("%s\n",cc);
            string str(c);
            //if(str.find("[1-2]:") != string::npos)
            if(str.find("]:") != string::npos)
            {
                printf("send 2 successful\n");
                write_port(hComm, (uint8_t *)"2\r", 2);
                delay(100);
            }
            if(str.find("C") != string::npos)
            {
                int st;
                printf("Sending Minion through XModem...\n");
                /* the following should be changed for your environment:
                   0x30000 is the download address,
                   12000 is the maximum size to be send from this address
                */
                bool quit_requested = false;
                char* p_buf = new char[DFW_FW_SIZE];
                long file_size;
                p_buf = read_file_to_buffer(DFW_MINION_FILE,&file_size);

                #ifdef DEBUG_MSG
                printf("reading file to buf done:%d...\n", file_size);
                #endif
                st = xmodemTransmit((unsigned char *)p_buf, file_size, &quit_requested);
                if (st < 0) {
                    printf ("Xmodem transmit error: status: %d\n", st);
                }
                else  {
                    printf ("Xmodem successfully transmitted %d bytes\n", st);
                }
                delay(200);
                /* set baudrate to 921600 */
                //dcbOri.BaudRate = 921600;
                fSuccess = com_port_set_baudrate(921600);//SetCommState(hComm, &dcbOri);
                delay(600);
                if (!fSuccess) {printf("SetCommState 921600 fail!!\n");}
                else {printf("SetCommState 921600 successful\n");}

                /* prepare to DFW scpu/ncpu.bin */
                delay(200);
                uint8_t fw_buf[UART_FIFO_SIZE+MSG_HDR_SIZE];
                //uint8_t *buf_tmp = &fw_buf[MSG_HDR_SIZE];
                printf("Sending SCPU bin to addr 0x%X...\n",SCPU_START_ADDRESS);
                p_buf = read_file_to_buffer(DFW_SCPU_FILE,&file_size);
                com_port_file_send(CMD_FLASH_WRITE, p_buf, file_size, SCPU_START_ADDRESS);

                printf("Sending NCPU bin to addr 0x%X...\n",NCPU_START_ADDRESS);
                p_buf = read_file_to_buffer(DFW_NCPU_FILE,&file_size);
                com_port_file_send(CMD_FLASH_WRITE, p_buf, file_size, NCPU_START_ADDRESS);

                printf("Run code from addr 0x%X...\n",SCPU_START_ADDRESS);
                //com_port_file_send(CMD_SCPU_RUN, NULL, 0, SCPU_START_ADDRESS);
                MsgHdr msghdr;
                msghdr.preamble=MSG_HDR_CMD;
                msghdr.crc16 = 0;
                msghdr.cmd = CMD_SCPU_RUN;
                msghdr.addr = SCPU_START_ADDRESS;
                msghdr.len = 0;
                memcpy(&fw_buf[0],&msghdr, MSG_HDR_SIZE);
                msghdr.crc16 = gen_crc16(&fw_buf[4],12);
                memcpy(&fw_buf[2],&msghdr.crc16, 2);
                #ifdef DEBUG_MSG
                for(int i=0; i<MSG_HDR_SIZE; i++)
                {
                    printf("%X ",fw_buf[i]);
                    if((i!=0)&&((i%32)==0))
                        printf("\n");
                }
                #endif
                write_port(hComm, (uint8_t *)&fw_buf[0], MSG_HDR_SIZE);
                while(1) //wait ACK
                {
                    if(read_port(hComm, (uint8_t *)cc, MSG_HDR_SIZE))
                    {
                        #ifdef DEBUG_MSG
                        for(int i=0; i<MSG_HDR_SIZE; i++)
                            printf("%X ",(uint8_t)c[i]);
                        #endif
                        if(c[4]==CMD_ACK)
                            break;
                    }
                }
                /* set baudrate to 115200 */
                //dcbOri.BaudRate = 115200;
                fSuccess = com_port_set_baudrate(115200);//SetCommState(hComm, &dcbOri);
                if (!fSuccess) {printf("SetCommState 115200 fail!!\n");}
                else {printf("SetCommState 115200 successful\n");}
                while(1) //wait till scpu starts running
                {
                    if(read_port(hComm, (uint8_t *)cc, 1000))
                    {
                        printf("\n%s\n",cc);
                        break;
                    }
                }
                break;
            }
        }
    }
    printf("CloseHandle...\n");
    com_port_close();
    return 0;
}
