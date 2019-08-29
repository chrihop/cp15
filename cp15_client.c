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

#include "cp15.h"

char buffer[255];


int main(int argc, char ** argv)
{
	int cp15_dev;
	cp15_dev = open("/dev/" DEV, O_RDWR);

	struct cp15_request_t *r = (struct cp15_request_t *) buffer;
	r->direction = CP15_MRC;
	write(cp15_dev, buffer, SIZEOF_REQUEST);
	read(cp15_dev, buffer, SIZEOF_RESPONSE);

	struct cp15_response_t *v = (struct cp15_response_t *) buffer;
	if (v->result == CP15_OK)
	{
		printf("0x%08x\n", v->result);
	}
	else
	{
		return 1;
	}
	return 0;
}