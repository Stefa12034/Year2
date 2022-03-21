#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s id server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[]) {
	
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    int sockfd, n, ret;
	struct sockaddr_in cli_addr;
	char buffer[BUFLEN];

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar

	if (argc < 4) {
		usage(argv[0]);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

    // dezactivare algoritm Neagle
    int flag = 1;
    int res = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
    DIE(res < 0, "setsocktcp");

	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &cli_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &cli_addr, sizeof(cli_addr));
	DIE(ret < 0, "connect");

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	FD_SET(sockfd, &read_fds);
	FD_SET(STDIN_FILENO, &read_fds);

    bool ok = true;

	while (1) {
        if (ok) {
            // Trimitere id
            n = send(sockfd, argv[1], BUFLEN, 0);
            DIE(n < 0, "send");
            ok = false;
        } else {

            tmp_fds = read_fds; 
            
            ret = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
            DIE(ret < 0, "select");

            if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
                // se citeste de la stdin
                memset(buffer, 0, BUFLEN);
                fgets(buffer, BUFLEN - 1, stdin);

                if (strncmp(buffer, "exit", 4) == 0) {
                    break;
                }

                if (strncmp(buffer, "subscribe", 9) == 0) {
                    cout << "Subscribed to topic.\n";
                    // se trimite mesaj la server
                    n = send(sockfd, buffer, BUFLEN, 0);
                    DIE(n < 0, "send");
                }

                if (strncmp(buffer, "unsubscribe", 11) == 0){
                    cout << "Unsubscribed from topic.\n";
                    // se trimite mesaj la server
                    n = send(sockfd, buffer, BUFLEN, 0);
                    DIE(n < 0, "send");
                }
            }

            if (FD_ISSET(sockfd, &tmp_fds)) {
                // se primeste mesaj, se verifica daca este pentru iesire
                // si se afiseaza in caz ca nu este
                memset(buffer, 0, BUFLEN);
                n = recv(sockfd, buffer, BUFLEN, 0);
                DIE(n < 0, "recv");
                if (string(buffer).length() == 0 || string(buffer) == "exit") {
                    break;
                }
                cout << buffer;
            }
        }
	}

	close(sockfd);

	return 0;
}
