#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/types.h>  
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#define NUM_LEDS 120

typedef struct {
  uint8_t brightness; //leading three bits must be ones.
  uint8_t blue;
  uint8_t green;
  uint8_t red;
} __attribute__((packed)) led_frame_t;

const uint8_t start_frame[4] = {0};
led_frame_t led_frames[NUM_LEDS] = {0xFF};
const uint8_t end_frame[8] = {0};


void transfer_led_frames(int32_t fd){
  uint8_t rx[sizeof(led_frames)] = {0};
  uint16_t led_frame_bytes = sizeof(led_frames);

  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    led_frames[i].brightness = 0xFF;
    led_frames[i].red = 0x00;
    led_frames[i].green = 0x00;
    led_frames[i].blue = 0x0F;
  }

  struct spi_ioc_transfer tr = {
    .tx_buf = (unsigned long)led_frames,
    .tx_nbits = 8,
    .rx_buf = (unsigned long)rx,
    .len = led_frame_bytes,
    .delay_usecs = 0,
    .speed_hz = 10000000,
    .bits_per_word = 8, 
  };

  int32_t err = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if(err == -1){
    printf("error led frame %i\n", err);
  }else {
    printf("wrote %i bytes\n", err);
  }
}

void transfer_end_frame(int32_t fd){
  uint8_t rx[sizeof(end_frame)] = {0};

  struct spi_ioc_transfer tr = {
    .tx_buf = (unsigned long)end_frame,
    .tx_nbits = 8,
    .rx_buf = (unsigned long)rx,
    .len = sizeof(end_frame),
    .delay_usecs = 0,
    .speed_hz = 4000000,
    .bits_per_word = 8, 
  };

  int32_t err = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if(err == -1){
    printf("error sending end frame %i\n", err);
  }else {
    printf("wrote %i bytes\n", err);
  }

}


void transfer_start_frame(int32_t fd){

  uint8_t rx[sizeof(start_frame)] = {0};

  struct spi_ioc_transfer tr = {
    .tx_buf = (unsigned long)start_frame,
    .tx_nbits = 8,
    .rx_buf = (unsigned long)rx,
    .len = sizeof(start_frame),
    .delay_usecs = 0,
    .speed_hz = 4000000,
    .bits_per_word = 8, 
  };

  int32_t err = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if(err == -1){
    printf("error sending start frame %i\n", err);
  }else {
    printf("wrote %i bytes\n", err);
  }
}

int32_t main(){
  int32_t err;
  int32_t mode = SPI_MODE_0;
  uint8_t bits = 8;

  uint8_t tx[4] = {0xAA};
  uint8_t rx[sizeof(tx)] = {0};

  struct spi_ioc_transfer tr = {
    .tx_buf = (unsigned long)tx,
    .tx_nbits = 8,
    .rx_buf = (unsigned long)rx,
    .len = sizeof(tx),
    .delay_usecs = 0,
    .speed_hz = 4000000,
    .bits_per_word = 8, 
  };



  int32_t fd = open("/dev/spidev1.0", O_RDWR);
  printf("fd %i\n", fd);

  err = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
	if (err == -1)
    printf("can't set mode bits %i\n", err);

  err = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
	if (err == -1)
    printf("can't set mode bits %i\n", err);

  err = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (err == -1)
    printf("can't set num bits %i\n", err);

  err = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (err == -1)
    printf("can't set num bits %i\n", err);

  //err = write(fd, wr_buf, sizeof(wr_buf));

  transfer_start_frame(fd);
  transfer_led_frames(fd);
  transfer_end_frame(fd);

	close(fd);

}
