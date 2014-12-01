/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * main.c: Hauptprogramm des Servers
 */

#include "common/util.h"
#include "catalog.h"
#include "login.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <linux/limits.h>
#include <netinet/in.h>
#include <getopt.h>

#define LOCK_FILE "/tmp/quiz_gruppe06_server.lock"

void printUsage();
int lock();
void unlock();

int main(int argc, char **argv)
{
    int option = 0;
    int lockFileHandle = 0;
    uint16_t port = 54321;

	setProgName(argv[0]);
	infoPrint("Server Gruppe 06");

	while ((option = getopt(argc, argv,"p:")) != -1) {
		switch (option) {
			case 'p' :
				port = atoi(optarg);
				break;
			default:
				printUsage();
				exit(EXIT_FAILURE);
		}
	}

	if ((lockFileHandle = lock()) < 0) {
		printf("Server is already running.\n");
		return EXIT_FAILURE;
	}

	server_start(port);
	unlock(lockFileHandle);

	return 0;
}

void printUsage()
{
	infoPrint("-c KATALOG_VERZEICHNIS [-d] [-r] [-s] [-l PFAD_ZUM_LOADER] [-p PORTNUMMER]\n");
}

int lock()
{
	char s[32];
	int fd;
	struct flock lock;

	fd = open(LOCK_FILE, O_TRUNC | O_CREAT | O_WRONLY, 0644);
	if (fd < 0) {
		perror("Cannot create pid file");
		exit(1);
	}

	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	if (fcntl(fd, F_SETLK, &lock) < 0) {
		perror("Cannot lock pid fd");
		exit(1);
	}

	snprintf(s, sizeof(s), "%d\n", (int)getpid());

	if (write(fd, s, strlen(s)) < strlen(s)) {
		perror("write");
	}

	if (fsync(fd) < 0) {
		perror("fsync");
	}

	return fd;
}

void unlock(int fd)
{
	close(fd);
	if (unlink(LOCK_FILE) < 0) {
		perror("unlink");
	}
}
