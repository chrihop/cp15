/**
 * @file cp15.h
 * @author Hao (chrihop@gmail.com)
 * @brief common types between kernel module and client
 * @version 0.1
 * @date 2019-08-28
 */

#define DEV		"cp15"

enum direction_t
{
	CP15_MRC,
	CP15_MCR,
};

struct cp15_request_t
{
	enum direction_t	direction;
	unsigned char		cn;
	unsigned char		cm;
	unsigned char		op0;
	unsigned char		op1;
	unsigned int		val;
};

#define SIZEOF_REQUEST		sizeof(struct cp15_request_t)

enum cp15_status_t
{
	CP15_OK,
	CP15_FAIL,
};

struct cp15_response_t
{
	enum cp15_status_t	status;
	unsigned int		result;
};

#define SIZEOF_RESPONSE		sizeof(struct cp15_response_t)
