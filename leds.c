#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/types.h>  
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#define NUM_LEDS 120
#define SPI_CLK_SPEED 8E6

typedef struct {
  uint8_t brightness; //leading three bits must be ones.
  uint8_t blue;
  uint8_t green;
  uint8_t red;
} __attribute__((packed)) led_frame_t;

typedef struct spi_ioc_transfer spi_ioc_transfer_t;

const uint8_t start_frame[4] = {0};
led_frame_t led_frames[NUM_LEDS] = {0xFF};
const uint8_t end_frame[8] = {0};

struct spi_ioc_transfer transfers[3];

void init_transfers(spi_ioc_transfer_t * transfers){
  transfers[0].tx_nbits = 8;
  transfers[0].delay_usecs = 0;
  transfers[0].speed_hz = SPI_CLK_SPEED;
  transfers[0].bits_per_word = 8;
  transfers[0].len = sizeof(start_frame);
  transfers[0].tx_buf = (unsigned long)start_frame;
  transfers[0].cs_change = 0;

  transfers[1].tx_nbits = 8;
  transfers[1].delay_usecs = 0;
  transfers[1].speed_hz = SPI_CLK_SPEED;
  transfers[1].bits_per_word = 8;
  transfers[1].len = sizeof(led_frames);
  transfers[1].tx_buf = (unsigned long)led_frames;
  transfers[2].cs_change = 0;

  transfers[2].tx_nbits = 8;
  transfers[2].delay_usecs = 0;
  transfers[2].speed_hz = SPI_CLK_SPEED;
  transfers[2].bits_per_word = 8;
  transfers[2].len = sizeof(end_frame);
  transfers[2].tx_buf = (unsigned long)end_frame;
  transfers[2].cs_change = 0;
}

void init_led_frames(led_frame_t * frames){
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    frames[i].brightness = 0xFF;
    frames[i].red = 0x00;
    frames[i].green = 0x00;
    frames[i].blue = 0x0F;
  }
}

void init_spi(uint32_t * pFd){
  int32_t err;
  int32_t mode = SPI_MODE_0;
  uint8_t bits = 8;

  *pFd = open("/dev/spidev1.0", O_RDWR);
  printf("fd %i\n", *pFd);

  err = ioctl(*pFd, SPI_IOC_WR_MODE32, &mode);
	if (err == -1)
    printf("can't set mode bits %i\n", err);

  err = ioctl(*pFd, SPI_IOC_RD_MODE32, &mode);
	if (err == -1)
    printf("can't set mode bits %i\n", err);

  err = ioctl(*pFd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (err == -1)
    printf("can't set num bits %i\n", err);

  err = ioctl(*pFd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (err == -1)
    printf("can't set num bits %i\n", err);
}

int32_t main(){
  int32_t fd;
  init_spi(&fd);
  init_transfers(transfers);
  init_led_frames(led_frames);

  int32_t err = ioctl(fd, SPI_IOC_MESSAGE(3), transfers);
  printf("bytes sent: %i\n", err);

	close(fd);
}
