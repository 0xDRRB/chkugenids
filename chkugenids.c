/*
 * Copyright (c) 2022 Denis Bbodor
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/ioctl.h>
#include <dev/usb/usb.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void printhelp();
void printinfo(struct usb_device_info *);


/*
 * Print usage
 */
void
printusage()
{
	fprintf(stderr, "usage: %s -f FILE [-abchilmpqsuv]\n", getprogname());
}

/*
 * Print help
 */
void
printhelp()
{

	printf("chkugenids v0.1\n");
	printf("Copyright (c) 2022 - Denis Bodor\n\n");
	printf("Usage : chkugenids [OPTIONS]\n");
	printf(" -f /dev/ugenN.EE  path to ugen dev file\n");
	printf(" -i                print VendorID:ProductID\n");
	printf(" -v                print vendor name\n");
	printf(" -p                print product name\n");
	printf(" -s                print serial number\n");
	printf(" -b                print bus.address\n");
	printf(" -m                print power in mA\n");
	printf(" -u                print USB Speed (n: speed name)\n");
	printf(" -c                print class.subclass\n");
	printf(" -a                print all info in pretty form\n");
	printf(" -l                print all info in one line (VendorID:ProductID:vendor:product:serial:bus:address:class:subclass:power:speed)\n");
	printf(" -q                be quiet and fail silently (nothing out on stderr)\n");
	printf(" -h                show this help\n");
}

/*
 * Print information about USB device in readable form
 */
void printinfo(struct usb_device_info *devinfo)
{

	printf("IDs     : %04x:%04x\n", devinfo->udi_vendorNo, devinfo->udi_productNo);
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
	case USB_SPEED_SUPER_PLUS:	printf("(Super Speed+ - 10 Gbps)"); break;
	default: printf("(unknown speed)");
	}
	printf("\n");
}



/*
 * Print information about USB device on one line
 */
void
printinfoline(struct usb_device_info *devinfo)
{

	printf("%04x:%04x:%s:%s:%s:%u:%u:%u:%u:%d:%u\n",
			devinfo->udi_vendorNo, devinfo->udi_productNo,
			devinfo->udi_vendor, devinfo->udi_product,
			devinfo->udi_serial, devinfo->udi_bus,
			devinfo->udi_addr, devinfo->udi_class,
			devinfo->udi_subclass, devinfo->udi_power,
			devinfo->udi_speed);
}

/*
 * Query /dev/ugenN.EE and display information about USB device
 */
int
main(int argc, char **argv)
{
	char *filepath = NULL;
	opterr = 0;
	int retopt;
	int optbus = 0;
	int optclass = 0;
	int optid = 0;
	int optinfo = 0;
	int optinfoline = 0;
	int optpname = 0;
	int optpower = 0;
	int optquiet = 0;
	int optserial = 0;
	int optspeed = 0;
	int optvname = 0;
	int fd;
	struct usb_device_info info;

	setprogname(argv[0]);

	while ((retopt = getopt(argc, argv, "abcf:hilmpqsuv")) != -1) {
		switch (retopt) {
		case 'a':
			optinfo = 1;
			break;
		case 'b':
			optbus= 1;
			break;
		case 'c':
			optclass = 1;
			break;
		case 'f':
			filepath = strdup(optarg);
			break;
		case 'h':
			printhelp();
			return EXIT_SUCCESS;
		case 'i':
			optid = 1;
			break;
		case 'l':
			optinfoline = 1;
			break;
		case 'm':
			optpower = 1;
			break;
		case 'p':
			optpname = 1;
			break;
		case 'q':
			optquiet = 1;
			break;
		case 's':
			optserial = 1;
			break;
		case 'u':
			optspeed = 1;
			break;
		case 'v':
			optvname = 1;
			break;
		default:
			printusage();
			exit(EXIT_FAILURE);
		}
	}

	if (argc < 2 || optind != argc) {
		printusage();
		exit(EXIT_FAILURE);
	}

	if (!filepath) {
		errx(EXIT_FAILURE, "You must specify a valid /dev/ugenN.EE!");
	}

	if ((fd=open(filepath, O_RDONLY)) == -1) {
		if(!optquiet)
			err(EXIT_FAILURE, "Open Error");
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, USB_GET_DEVICEINFO, &info) == -1) {
		close(fd);
		if(!optquiet)
			err(EXIT_FAILURE, "USB_GET_DEVICEINFO Error");
		exit(EXIT_FAILURE);
	}

	if (optinfo) {
		printinfo(&info);
	}

	if (optinfoline) {
		printinfoline(&info);
	}

	if (optid)
		printf("%04x:%04x\n", info.udi_vendorNo, info.udi_productNo);

	if (optvname)
		printf("%s\n", info.udi_vendor);

	if (optpname)
		printf("%s\n", info.udi_product);

	if (optserial) {
		if (info.udi_serial[0]=='\0')
			printf("n/a\n");
		else
			printf("%s\n", info.udi_serial);
	}

	if (optbus)
		printf("%u.%u\n", info.udi_bus, info.udi_addr);

	if (optpower)
		printf("%d\n", info.udi_power);

	if (optspeed) {
		switch(info.udi_speed) {
		case USB_SPEED_LOW:
			printf("%u: Low Speed - 1.5 Mbps\n", info.udi_speed); break;
		case USB_SPEED_FULL:
			printf("%u: Full Speed - 12 Mbps\n", info.udi_speed); break;
		case USB_SPEED_HIGH:
			printf("%u: High Speed - 480 Mbps\n", info.udi_speed); break;
		case USB_SPEED_SUPER:
			printf("%u: Super Speed - 5 Gbps\n", info.udi_speed); break;
		case USB_SPEED_SUPER_PLUS:
			printf("%u: Super Speed+ - 10 Gbps\n", info.udi_speed); break;
		default:
			printf("%u: unknown speed\n", info.udi_speed);
		}
	}

	if (optclass)
		printf("%u.%u\n", info.udi_class, info.udi_subclass);

	close(fd);

	return EXIT_SUCCESS;
}
