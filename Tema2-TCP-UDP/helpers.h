#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <math.h>
#include <iterator>
#include <map>
#include <list>
#include <vector>
#include <sys/stat.h>
#include <netinet/tcp.h>

using namespace std;

/*
 * Macro de verificare a erorilor
 * Exemplu:
 *     int fd = open(file_name, O_RDONLY);
 *     DIE(fd == -1, "open failed");
 */

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#define BUFLEN 1500	// dimensiunea maxima a calupului de date
#define MAX_CLIENTS	5 // numarul maxim de clienti in asteptare
#define ID_LEN 11 // dimensiunea maxima a id-ului
#define MAX_DESCRIPTION_LEN 1500 // dimensiunea maxima a descrierii mesajului

#endif
