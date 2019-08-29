/**
 * @file cp15_client.c
 * @author Hao (chrihop@gmail.com)
 * @brief command line tool to interact with CP15 kernel module
 * @version 0.1
 * @date 2019-08-28
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "cp15.h"

char buffer[255];


int main(int argc, char ** argv)
{
	int opt;
	char *endptr;

	unsigned char cn = 255, cm = 255, op1 = 255, op2 = 255, wr = 0, val;
	while ((opt = getopt(argc, argv, ":w:n:m:1:2:h")) != -1)
	{
		switch (opt)
		{
			case 'n': cn  = strtoul(optarg, &endptr, 10); break;
			case 'm': cm  = strtoul(optarg, &endptr, 10); break;
			case '1': op1 = strtoul(optarg, &endptr, 10); break;
			case '2': op2 = strtoul(optarg, &endptr, 10); break;
			case 'w': wr  = 1; val = strtoul(optarg, &endptr, 16); break;
			case 'h': printf("%s -1 <op1> -n <cn> -m <cm> -2 <op2> [-w <hex>]\n"
							"  read cp 15 register by 'mrc op1, cn, cm, op2'\n"
							"  -w <hex> to write tge value (in hex format) into the cp15 register\n",
							argv[0]);
			case '?': /* ignored */ break;
		}
	}

	if (cn == 255 || cm == 255 || op1 == 255 || op2 == 255)
	{
		return 1;
	}

	int cp15_dev;
	cp15_dev = open("/dev/" DEV, O_RDWR);

	struct cp15_request_t *r = (struct cp15_request_t *) buffer;
	r->direction = (wr == 1 ? CP15_MCR : CP15_MRC);
	r->cn = cn;
	r->cm = cm;
	r->op0 = op1;
	r->op1 = op2;
	r->val = val;

	ssize_t count;
	/* send request */
	count = write(cp15_dev, buffer, SIZEOF_REQUEST);
	if (count != SIZEOF_REQUEST)
	{
		return 2;
	}

	/* read response */
	count = read(cp15_dev, buffer, SIZEOF_RESPONSE);
	if (count != SIZEOF_RESPONSE)
	{
		return 3;
	}

	struct cp15_response_t *v = (struct cp15_response_t *) buffer;
	if (v->status == CP15_OK)
	{
		printf("0x%08x\n", v->result);
	}
	else
	{
		return 4;
	}
	return 0;
}