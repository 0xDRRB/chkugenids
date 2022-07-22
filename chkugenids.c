#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <dev/usb/usb.h>

void printinfo(struct usb_device_info *devinfo) {
	printf("ID      : %04x:%04x\n", devinfo->udi_vendorNo, devinfo->udi_productNo);
	printf("Vendor  : %s\n", devinfo->udi_vendor);
	printf("Product : %s\n", devinfo->udi_product);
	printf("Serial  : %s\n", devinfo->udi_serial);
	printf("Bus     : %u\n", devinfo->udi_bus);
	printf("Address : %u\n", devinfo->udi_addr);
	printf("Class   : %u\n", devinfo->udi_class);
	printf("Sudclass: %u\n", devinfo->udi_subclass);
	printf("Power   : %d mA\n", devinfo->udi_power);
	printf("Speed   : %u ", devinfo->udi_speed);
	switch(devinfo->udi_speed) {
		case USB_SPEED_LOW:	printf("(Low Speed - 1.5 Mbps)"); break;
		case USB_SPEED_FULL:	printf("(Full Speed - 12 Mbps)"); break;
		case USB_SPEED_HIGH:	printf("(High Speed - 480 Mbps)"); break;
		case USB_SPEED_SUPER:	printf("(Super Speed - 5 Gbps)"); break;
		case USB_SPEED_SUPER_PLUS:	printf("(Super Speed+ - 10 Gbps )"); break;
	}
	printf("\n");
}

int main(int argc, char **argv) {
	int fd;
	struct usb_device_info info;
	unsigned int vid=0, pid=0;
	char *ep;

	if (argc != 4 && argc != 2) {
		fprintf(stderr,"Please use:\n  chkugenvp /dev/ugenN.EE\n or\n  chkugenvp /dev/ugenN.EE VendorID ProductID !\n");
		exit(EXIT_FAILURE);
	}

	if(argc == 4) {
		// check VendorID input
		vid = (unsigned int)strtol(argv[2], &ep, 16);
		if (ep == argv[2] || *ep != '\0') {
			fprintf(stderr,"Invalid VendorID !\n");
			exit(EXIT_FAILURE);
		}

		if (vid > 0xffff) {
			fprintf(stderr,"Invalid VendorID ! Must be 0000..ffff.\n");
			exit(EXIT_FAILURE);
		}

		pid = (unsigned int)strtol(argv[3], &ep, 16);
		if (ep == argv[3] || *ep != '\0') {
			fprintf(stderr,"Invalid ProductID !\n");
			exit(EXIT_FAILURE);
		}

		if (pid > 0xffff) {
			fprintf(stderr,"Invalid ProductID ! Must be 0000..ffff.\n");
			exit(EXIT_FAILURE);
		}
	}

	if ((fd=open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr,"Open Error : %s (%d)\n", strerror(errno),errno);
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, USB_GET_DEVICEINFO, &info) < 0) {
		fprintf(stderr,"USB_GET_DEVICEINFO Error : %s (%d)\n", strerror(errno),errno);
		close(fd);
		exit(EXIT_FAILURE);
	}

	if(argc == 4) {
		if (info.udi_vendorNo != (uint16_t)vid || info.udi_productNo!=(uint16_t)pid) {
			close(fd);
			exit(EXIT_FAILURE);
		}
	} else {
		printinfo(&info);
	}

	close(fd);

	return(EXIT_SUCCESS);
}
