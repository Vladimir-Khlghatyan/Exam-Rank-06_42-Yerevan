#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

int maxfd, id = 0, sockfd = 0;
int arr[65000];
char buff[200100];
fd_set aset, rset, wset; 

void fatal(char *error)
{
	if (sockfd > 2)
		close(sockfd);
	write(2, error, strlen(error));
	exit(1);
}

void	sending(int connfd)
{
	for (int fd = 3; fd <= maxfd; ++fd)
		if (fd != connfd && FD_ISSET(fd, &wset))
			if (send(fd, buff, strlen(buff), 0) < 0)
				fatal("Fatal error\n");

	bzero(&buff, sizeof(buff));
}

int main(int ac, char **av)
{
	if (ac == 1)
		fatal("Wrong number of arguments\n");

	int connfd;
	socklen_t len;
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
		fatal("Fatal error\n");
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		fatal("Fatal error\n");

	if (listen(sockfd, 10) != 0)
		fatal("Fatal error\n");

	len = sizeof(cli);
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
				fatal("Fatal error\n");
			FD_SET(connfd, &aset);
			sprintf(buff, "server: client %d just arrived\n", id);
			arr[connfd] = id++;
			sending(connfd);
			maxfd = connfd > maxfd ? connfd : maxfd;
			continue;
		}

		for (int fd = 3; fd <= maxfd; ++fd)
		{
			if (FD_ISSET(fd, &rset))
			{
				int r = 1;
				char msg[20000];
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

				sending(fd);
			}
		}
	}
	return (0);
}