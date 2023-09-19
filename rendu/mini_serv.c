#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define WRONG "Wrong number of arguments\n"
#define FATAL "Fatal error\n"
#define BUF 200000
#define PORT 65000

int maxfd, id, sockfd = 0;
int arr[PORT];
char buff[BUF + 100];
fd_set aset, rset, wset; 

void	fatal(char *er)
{
	if (sockfd > 2)
		close(sockfd);
	write(2, er, strlen(er));
	exit(1);
}

void	send_all(int connfd)
{
	for(int fd = 2; fd <= maxfd; fd++)
		if (fd != connfd && FD_ISSET(fd, &wset))
			if (send(fd, buff, strlen(buff), 0) < 0)
				fatal(FATAL);
}

int main(int ac, char **av)
{
	if (ac != 2)
		fatal(WRONG);

	int connfd;
	socklen_t len;
	struct sockaddr_in servaddr, cli; 

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
		fatal(FATAL);
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		fatal(FATAL);

	if (listen(sockfd, 10) != 0)
		fatal(FATAL);

	len = sizeof(cli);
	id = 0;
	maxfd = sockfd;
	FD_ZERO(&aset);
	FD_SET(sockfd, &aset);

	while(1)
	{
		rset = wset = aset;
		if (select(maxfd + 1, &rset, &wset, 0, 0) < 0)
			continue;
		if (FD_ISSET(sockfd, &rset))
		{
			connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
			if (connfd < 0)
				fatal(FATAL);
			sprintf(buff, "server: client %d just arrived\n", id);
			FD_SET(connfd, &aset);
			arr[connfd] = id++;
			send_all(connfd);
			maxfd = connfd > maxfd ? connfd : maxfd;
			continue;
		}

		for (int fd = 2; fd <= maxfd; ++fd)
		{
			if (FD_ISSET(fd, &rset))
			{
				int r = 1;
				char msg[BUF];
				bzero (&msg, sizeof(msg));
				while(r == 1 && msg[strlen(msg) - 1] != '\n')
					r = recv(fd, msg + strlen(msg), 1, 0);
				if (r <= 0)
				{
					sprintf(buff, "server: client %d just left\n", arr[fd]);
					FD_CLR(fd, &aset);
					close(fd);
				}
				else
					sprintf(buff, "client %d: %s", arr[fd], msg);
				send_all(fd);
			}
		}
	}
	return (0);
}