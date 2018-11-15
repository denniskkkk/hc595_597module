#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/wait.h>
#include <signal.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/time.h>
#include <fnmatch.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <sys/io.h>

#ifdef __WIN32__
#define sleep(x) _sleep(x)
#endif
#include <ftdi.h>
#include <math.h>


//#include <usb.h>
//#include <ustat.h>

char buf[65536];
char buf1[65536];
char buf2[65536];
char buf3[65536];

char rbuf[65536];
char rbuf1[65536];
char rbuf2[65536];
char rbuf3[65536];

int delay = 50; // scanning delay time in msec 50 msec

struct ftdi_context ftdic, ftdic1, ftdic2, ftdic3;
struct ftdi_context ftdic_b, ftdic1_b, ftdic2_b, ftdic3_b;

long errcount = 0L;

// global threads
pthread_t thread[4];

// PRA port thread
pthread_mutex_t pra_mutex;
pthread_cond_t pra_cond;
pthread_mutex_t pra_mutex;
pthread_cond_t pra_cond;
pthread_attr_t pra_attr;

// PRB port thread
pthread_mutex_t prb_mutex;
pthread_cond_t prb_cond;
pthread_mutex_t prb_mutex;
pthread_cond_t prb_cond;
pthread_attr_t prb_attr;

// PRC port thread
pthread_mutex_t prc_mutex;
pthread_cond_t prc_cond;
pthread_mutex_t prc_mutex;
pthread_cond_t prc_cond;
pthread_attr_t prc_attr;

// PRD port thread
pthread_mutex_t prd_mutex;
pthread_cond_t prd_cond;
pthread_mutex_t prd_mutex;
pthread_cond_t prd_cond;
pthread_attr_t prd_attr;

void *testPRA() {
	int size = 0;
	int z =1;
	int y =0;
	while (1)
	{
			//	for (y =0; y < 4096 ; y ++) {
				buf[size++] = 0x80 & 0xff;
				buf[size++] = z & 0xff;
				buf[size++] = 0xff; // 11111011 , 1=out 0=in

				buf[size++] = 0x82 & 0xff; //
				buf[size++] = (z >> 8) & 0xff ;
				buf[size++] = 0xff; // 11111011 , 1=out 0=in
				usleep (delay);
			//	z++;
				z = z << 1; if ( z > 0x8000)  z =1;
			//	}
			//	buf[size++] = 0x87 & 0xff; // imediate send
				ftdi_write_data(&ftdic, buf, size); // write commands
			//	printf("PRA buffer size %d, bit %d,  count %d, write 0x%08X\n",
			//			size, z, p, data);
				size = 0;

	}
}

void *testPRB() {
	int size = 0;
	long z =1;
	int y =0;
	while (1)
	{
			//	for (y =0; y < 4096 ; y ++) {
				buf1[size++] = 0x80 & 0xff;
				buf1[size++] = z & 0xff;
				buf1[size++] = 0xff; // 11111011 , 1=out 0=in

				buf1[size++] = 0x82 & 0xff; //
				buf1[size++] = (z >> 8) & 0xff ;
				buf1[size++] = 0xff; // 11111011 , 1=out 0=in
				usleep(delay);
				//z++;
				z = z << 1; if ( z > 0x8000)  z =1;
			//	}
			//	buf1[size++] = 0x87 & 0xff; // imediate send
				ftdi_write_data(&ftdic1, buf1, size); // write commands
			//	printf("PRB buffer size %d, bit %d,  count %d, write 0x%08X\n",
			//			size, z, p, data);
				size = 0;


	}
}

void *testPRC() {

}

void *testPRD() {

}

void initPRA() {
	int size = 0;
	int f;
	printf("PRA threading......\n");

	// channel A -------------------------------------
	if (ftdi_init(&ftdic) < 0) {
		fprintf(stderr, "PRA ftdi init failed \n");
		return (EXIT_FAILURE);
	}
	//
	//ftdi_set_interface(&ftdic, INTERFACE_A);
	//f = ftdi_usb_open(&ftdic, 0x0403, 0x6011);
	f = ftdi_usb_open_string(&ftdic, "i:0x403:0x6014:0");
	if (f < 0 && f != -5) {
		fprintf(stderr, "unable to open ftdi device PRA: %d (%s)\n", f,
				ftdi_get_error_string(&ftdic));
		exit(-1);
	}
	ftdi_usb_reset(&ftdic);
	//ftdic.usb_read_timeout = 60000;
	//ftdic.usb_write_timeout = 60000;
	//ftdic.max_packet_size = 65536;
	//ftdic.async_usb_buffer_size = 65536;
	//ftdic.readbuffer_chunksize = 65536;
	//ftdic.writebuffer_chunksize = 65536;
	//ftdic.readbuffer_offset =0;
	printf("ftdi port PRA open succeeded(channel 1): %d\n", f);
	ftdi_setflowctrl(&ftdic, SIO_RTS_CTS_HS);

	ftdi_set_latency_timer(&ftdic, 1);

	ftdi_usb_purge_buffers(&ftdic);

	printf("enabling bitmode PRA RESET(channel 1)\n");
	ftdi_set_bitmode(&ftdic, 0xfb, 0x00);
	//ftdi_set_bitmode(&ftdic, 0xFF, BITMODE_BITBANG);
	printf("enabling bitmode PRA MPSSE mode(channel 1)\n");
	ftdi_set_bitmode(&ftdic, 0xfb, BITMODE_MPSSE);

	buf[size++] = 0x8a & 0xff; // disable divid by 5 60Mhz clock, 0x8B= enable
	buf[size++] = 0x97 & 0xff; // turn off adaptive clocking, 0x96=on
	buf[size++] = 0x8d & 0xff; // disable 3 phase, 0x8c=enable

	buf[size++] = 0x86 & 0xff; // set TCLK
	buf[size++] = 0x00 & 0xff; // lo 0x0003=7.5Mhz, 0xffff=457hz
	buf[size++] = 0x00 & 0xff; // hi
	ftdi_write_data(&ftdic, buf, size);
	size =0;
	printf("PRA buffer size %d\n", size);

}

void initPRB() {
	int size = 0;
	int f;
	printf("PRB threading.....\n");
	// channel B -------------------------------------
	if (ftdi_init(&ftdic1) < 0) {
		fprintf(stderr, "PRB ftdi init failed \n");
		return (EXIT_FAILURE);
	}
	//
	//ftdi_set_interface(&ftdic1, INTERFACE_A);
	f = ftdi_usb_open_string(&ftdic1, "i:0x403:0x6014:1");
	if (f < 0 && f != -5) {
		fprintf(stderr, "unable to open PRB ftdi device: %d (%s)\n", f,
				ftdi_get_error_string(&ftdic1));
		exit(-1);
	}
	ftdi_usb_reset(&ftdic1);
	//ftdic1.usb_read_timeout = 60000;
	//ftdic1.usb_write_timeout = 60000;
	//ftdic1.max_packet_size = 65536;
	//ftdic1.async_usb_buffer_size = 65536;
	//ftdic1.readbuffer_chunksize = 65536;
	//ftdic1.writebuffer_chunksize = 65536;
	//ftdic1.readbuffer_offset =0;
	printf("PRB ftdi port B open succeeded(channel 1): %d\n", f);
	ftdi_setflowctrl(&ftdic1, SIO_RTS_CTS_HS);

	ftdi_set_latency_timer(&ftdic1, 1);
	ftdi_usb_purge_buffers(&ftdic1);

	printf("PRB enabling bitmode RESET(channel 2)\n");
	ftdi_set_bitmode(&ftdic1, 0xfb, 0x00);
	//ftdi_set_bitmode(&ftdic1, 0xFF, BITMODE_BITBANG);
	printf("PRB enabling bitmode MPSSE mode(channel 2)\n");
	ftdi_set_bitmode(&ftdic1, 0xfb, BITMODE_MPSSE);
	size = 0;
	buf1[size++] = 0x8a & 0xff; // disable divid by 5 60Mhz clock, 0x8B= enable
	buf1[size++] = 0x97 & 0xff; // turn off adaptive clocking, 0x96=on
	buf1[size++] = 0x8d & 0xff; // disable 3 phase, 0x8c=enable

	buf1[size++] = 0x86 & 0xff; // set TCLK
	buf1[size++] = 0x00 & 0xff; // lo 0x0003=7.5Mhz, 0xffff=457hz
	buf1[size++] = 0x00 & 0xff; // hi

	ftdi_write_data(&ftdic1, buf1, size);
	size =0;
	printf("PRB buffer size %d\n", size);

}

void initPRC() {
	int size = 0;
	printf("PRC threading.....\n");
}

void initPRD() {
	int size = 0;
	printf("PRD threading.....\n");
}

void signalhandler(int sig) {
	/*
	 usleep(200000);
	 pthread_mutex_lock ( &cmdproc_mutex );
	 int condgpio = pthread_cond_broadcast(&gpiocmdproc_cond); // signal cmdproc to process queue wakeup
	 if (condgpio < 0) {
	 #ifdef DEBUG
	 dprintf("signal to process command error");
	 #endif
	 }
	 int condssr = pthread_cond_broadcast(&cmdproc_cond); // signal cmdproc to process queue wakeup

	 if (condssr < 0) {
	 dprintf("signal to process command error");
	 }  */
	exit(-1);
}

void regsignal() {
	signal(SIGQUIT, signalhandler);
	signal(SIGTERM, signalhandler);
	signal(SIGHUP, signalhandler);
	signal(SIGINT, signalhandler);
}

int main(int argc, char **argv) {
	int err;
	int threadgroup;
	// check parameter //
	if (argc > 1) {
		//	printf ("parameter: %s\n", argv[1]);
		delay = atoi(argv[1]);
		if (delay < 1)
			delay = 50; // default delay
	}
	printf("delay = %d\n", delay);
	/* int result = setpriority(PRIO_PROCESS, 0, 0);
	 if (result == -1) {
	 #ifdef DEBUG
	 dprintf("cannot change to higher priority\n");
	 #endif
	 } */
	// register signal from OS
	regsignal();
	// init PRA and PRB....
	initPRA();
	initPRB();
	// pra thread init
	pthread_mutex_init(&pra_mutex, NULL);
	pthread_cond_init(&pra_cond, NULL);
	pthread_mutex_init(&pra_mutex, NULL);
	pthread_cond_init(&pra_cond, NULL);
	pthread_attr_init(&pra_attr);
	pthread_attr_setdetachstate(&pra_attr, PTHREAD_CREATE_JOINABLE);
	// prb thread init
	pthread_mutex_init(&prb_mutex, NULL);
	pthread_cond_init(&prb_cond, NULL);
	pthread_mutex_init(&prb_mutex, NULL);
	pthread_cond_init(&prb_cond, NULL);
	pthread_attr_init(&prb_attr);
	pthread_attr_setdetachstate(&prb_attr, PTHREAD_CREATE_JOINABLE);
	// create PRA thread
	if ((err = pthread_create(&thread[0], &pra_attr, testPRA, NULL))) {
		// create without attribute
		if (err != EPERM || pthread_create(&thread[0], NULL, testPRA, NULL))
			printf("unable create PRA thread");
		printf("create PRA thread without scheduling\n");
	}
	// create PRB thread
	if ((err = pthread_create(&thread[1], &prb_attr, testPRB, NULL))) {
		// create without attribute
		if (err != EPERM || pthread_create(&thread[1], NULL, testPRB, NULL))
			printf("unable create PRB thread");
		printf("create PRB thread without scheduling\n");
	}

	// thread group join and wait thread destory
	// pthread_join(thread[0], &threadgroup); // cmd procesing thread join
	// pthread_join(thread[1], &threadgroup); // gpio processing thread join
	//destory PRA thread
	pthread_attr_destroy(&pra_attr);
	pthread_mutex_destroy(&pra_mutex);
	pthread_cond_destroy(&pra_cond);
	pthread_mutex_destroy(&pra_mutex);
	pthread_cond_destroy(&pra_cond);
	//destory PRB thread
	pthread_attr_destroy(&prb_attr);
	pthread_mutex_destroy(&prb_mutex);
	pthread_cond_destroy(&prb_cond);
	pthread_mutex_destroy(&prb_mutex);
	pthread_cond_destroy(&prb_cond);
	//
	pthread_exit(NULL);
	return 0;
}

