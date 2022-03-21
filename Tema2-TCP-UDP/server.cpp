#include "helpers.h"

typedef struct {
    char topic[50];
    char type[1];
    char description[MAX_DESCRIPTION_LEN];
} msg;

// se creeaza mesaj de tip INT pentru trimitere la tcp
string get_message_int(char *ip, int port, msg message) {
    string mesg;
    string data_type = "INT";
    char sign = message.description[0];
    char a[20];
    sprintf(a, "%d", *((int *) (message.description + 1)));

    int r = atoi(a);
    if (sign == 0) {
        mesg = string(ip) + ":" + to_string(port) + " - " + message.topic
                + " - " + data_type + " - " + to_string(ntohl(r));
    } else {
        mesg = string(ip) + ":" + to_string(port) + " - " + message.topic
                + " - " + data_type + " - -" + to_string(ntohl(r));
    }
    return mesg;
}

// se creeaza mesaj de tip SHORT_REAL pentru trimitere la tcp
string get_message_short_real(char *ip, int port, msg message) {
    string mesg;
    string data_type = "SHORT_REAL";
    char a[20];
    sprintf(a, "%d", *((unsigned short *) (message.description)));

    unsigned short r = (unsigned short)atoi(a);
    r = ntohs(r);
    double num = r * 1.00 / 100;

    memset(a, 0, 20);
    sprintf(a, "%.2f", num);

    mesg = string(ip) + ":" + to_string(port) + " - " + message.topic
            + " - " + data_type + " - " + string(a);
    return mesg;
}

// se creeaza mesaj de tip FLOAT pentru trimitere la tcp
string get_message_float(char *ip, int port, msg message) {
    string mesg;
    string data_type = "FLOAT";
    char a[20];
    char sign = message.description[0];
    
    char nr[4];
    nr[0] = message.description[1];
    nr[1] = message.description[2];
    nr[2] = message.description[3];
    nr[3] = message.description[4];
    sprintf(a, "%d", *((int *) (nr)));

    int r = atoi(a);
    r = ntohl(r);
    char dec[20];
    sprintf(dec, "%d", *((char *) (message.description + 5)));

    double num;
    memset(a, 0, 20);
    if (atoi(dec) != 0) {
        num = (double)(r) / pow(10, atoi(dec));
        sprintf(a, "%.*f", atoi(dec), num);
    } else {
        num = r;
        sprintf(a, "%.0f", num);
    }

    if (sign == 0) {
        mesg = string(ip) + ":" + to_string(port) + " - " + message.topic
                + " - " + data_type + " - " + string(a);
    } else {
        mesg = string(ip) + ":" + to_string(port) + " - " + message.topic
                + " - " + data_type + " - -" + string(a);
    }

    return mesg;
}

// se creeaza mesaj de tip STRING pentru trimitere la tcp
string get_message_string(char *ip, int port, msg message) {
    string mesg;
    string data_type = "STRING";

    mesg = string(ip) + ":" + to_string(port) + " - " + message.topic
            + " - " + data_type + " - " + string(message.description);
    return mesg;
}

// se verifica de ce tip e mesajul si se distribuie la functia potrivita
string get_message(char *ip, int port, msg message) {
    if (message.type[0] == 0) {
        return get_message_int(ip, port, message);
    }
    if (message.type[0] == 1) {
        return get_message_short_real(ip, port, message);
    }
    if (message.type[0] == 2) {
        return get_message_float(ip, port, message);
    }
    if (message.type[0] == 3) {
        return get_message_string(ip, port, message);
    }
    return "";
}

// se verifica daca clientul cu id-ul dat este in lista de id-uri inactive
bool non_active_id(string id, map <string, string> non_active_ids) {
    for(auto i = non_active_ids.begin(); i != non_active_ids.end(); ++i) {
        if (i->first == id) {
            return true;
        }
    }
    return false;
}

// se verifica daca clientul cu id-ul dat este in lista de id-uri active
bool is_active(string id, map<int, string> ids_index) {
    for (auto i : ids_index) {
        if (i.second == id) {
            return true;
        }
    }
    return false;
}

// se verifica daca clientul cu id-ul dat este abonat la topicul dat
bool already_subscribed (string topic, string id, map <string, vector <pair <string, char>>> subscriptions) {
    for (auto i = subscriptions[id].begin(); i != subscriptions[id].end(); ++i) {
        if (i->first == topic) {
            return true;
        }
    }
    return false;
}

// adauga sau sterge topicul dat din lista de topicuri a id-ului
void update_subscriptions(char *subs, char *topic, char *sf, map <string, vector <pair <string, char>>> *subscriptions, string id) {
    if (string(subs) == "subscribe") {
        // adauga topic
        if (!already_subscribed(string(topic), id, *subscriptions)) {
            (*subscriptions)[id].push_back(pair<string, char>(string(topic), sf[0]));
        }
    } else {
        // strege topic
        topic[strlen(topic) - 1] = '\0';
        for (auto i = (*subscriptions)[id].begin(); i != (*subscriptions)[id].end(); ++i) {
            if (i->first == topic) {
                (*subscriptions)[id].erase(i);
                i--;
                break;
            }
        }
    }
}

// verifica daca sf este setat pe 1 pentru topicul din lista id-ului 
bool sf_not_zero(map <string, vector <pair <string, char>>> subscriptions, string id, string topic) {
    for (auto i = subscriptions[id].begin(); i != subscriptions[id].end(); ++i) {
        if (i->first == topic) {
            if (i->second == '1') {
                return true;
            }
            break;
        }
    }
    return false;
}

// trimitere sau salvare mesaj la clientii tcp
void send_message(string topic, string mesg, 
                    map<int, string> ids_index, 
                    map <string, vector <pair <string, char>>> subscriptions,
                    map <string, string> *non_active_ids) {

    // trimitere pentru clientii activi
    for (auto i = ids_index.begin(); i != ids_index.end(); ++i) {
        if (already_subscribed(topic, i->second, subscriptions)) {
            int n = send(i->first, mesg.c_str(), mesg.size(), 0);
            DIE(n < 0, "send");
        }
    }
    // salvare pentru clientii inactivi
    for (auto i = (*non_active_ids).begin(); i != (*non_active_ids).end(); ++i) {
        if (already_subscribed(topic, i->first, subscriptions)) {
            if (sf_not_zero(subscriptions, i->first, topic)) {
                i->second += mesg;
            }
        }
    }
}

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	int sock_tcp, newsockfd, portno, sock_udp;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr_tcp, cli_addr_udp;
	int n, i, ret, dest, chrRead;
	socklen_t clilen;

    // socket -- id
    map<int, string> ids_index;

    // id -- messages to send
    map <string, string> non_active_ids;

    // id -- [topic - sf]
    map <string, vector <pair <string, char>>> subscriptions;


	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

	if (argc != 2) {
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

    // socket tcp
	sock_tcp = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sock_tcp < 0, "socket");

    // dezactivare algoritm Neagle
    int flag = 1;
    int res = setsockopt(sock_tcp, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
    DIE(res < 0, "setsocktcp");

	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sock_tcp, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	ret = listen(sock_tcp, MAX_CLIENTS);
	DIE(ret < 0, "listen");

    // socket udp
    sock_udp = socket(PF_INET, SOCK_DGRAM, 0);
    DIE(sock_udp == -1, "Open socket");

    cli_addr_udp.sin_family = AF_INET;
    cli_addr_udp.sin_port = htons(atoi(argv[1]));
    cli_addr_udp.sin_addr.s_addr = INADDR_ANY;

    int rs = bind(sock_udp, (struct sockaddr *)(&cli_addr_udp), sizeof(cli_addr_udp));
	DIE(rs < 0, "Open bind");

    // se adauga noii file descriptori in multimea read_fds
	FD_SET(sock_tcp, &read_fds);
    FD_SET(sock_udp, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    fdmax = max(sock_tcp, sock_udp);

	while (1) {
		tmp_fds = read_fds; 

        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sock_tcp) {
					// cerere de conexiune tcp pe care o accepta serverul
                    memset(buffer, 0, BUFLEN);
					clilen = sizeof(cli_addr_tcp);
					newsockfd = accept(sock_tcp, (struct sockaddr *) &cli_addr_tcp, &clilen);
					DIE(newsockfd < 0, "accept");

                    // primire ID
                    char id_str[ID_LEN];
                    memset(id_str, 0, ID_LEN);
                    n = recv(newsockfd, id_str, sizeof(id_str), 0);
                    DIE(n < 0, "recv");
                    string id = string(id_str);

                    // verificare daca exista deja un alt client conectat cu acelasi ID
                    if (is_active(id, ids_index)) {
                        string buf = "Client " + id + " already connected.\n";
                        cout << buf;
                        memset(buffer, 0, BUFLEN);
                        sprintf(buffer, "exit");
                        send(newsockfd, buffer, strlen(buffer), 0);
                        DIE(n < 0, "send");
                    } else {
                        // adaugare client
                        FD_SET(newsockfd, &read_fds);
                        if (newsockfd > fdmax) {
                            fdmax = newsockfd;
                        }

                        if (non_active_id(id, non_active_ids)) {
                            // trimitere mesaje vechi
                            send(newsockfd, non_active_ids[id].c_str(), non_active_ids[id].size(), 0);
                            DIE(n < 0, "send");
                            non_active_ids.erase(id);
                        }
                        ids_index.insert(pair<int, string>(newsockfd, id));
                    
                        sprintf(buffer, "New client %s connected from %s:%d\n", id_str,
                                    inet_ntoa(cli_addr_tcp.sin_addr), ntohs(cli_addr_tcp.sin_port));
                                    
                        cout << buffer;
                        
                    }

				}
                if (i == STDIN_FILENO) {
					// se primesc date de la stdin
					memset(buffer, 0, BUFLEN);
                    fgets(buffer, BUFLEN - 1, stdin);
                    if (strncmp(buffer, "exit", 4) == 0) {
                        break;
                    }
                }
                if (i != sock_udp && i != sock_tcp && i != 0) {
                    // se primesc date de la unul din socketii de client
                    memset(buffer, 0, BUFLEN);
                    n = recv(i, buffer, BUFLEN, 0);
                    DIE(n < 0, "recv");

                    if (n == 0) {
                        // se inchide conexiunea
                        non_active_ids[ids_index[i]] = "";
                        string buf = "Client " + ids_index[i] + " disconnected.\n";
                        ids_index.erase(i);
                        cout << buf;
                        close(i);
                        FD_CLR(i, &read_fds);
                    } else {
                        // se actualizeaza abonarile pentru clientul de pe socketul i
                        char *subs = strtok(buffer, " ");
                        char *topic = strtok(NULL, " ");
                        char *sf = strtok(NULL, " ");

                        if (string(subs) == "subscribe" || string(subs) == "unsubscribe") {
                            update_subscriptions(subs, topic, sf, &subscriptions, ids_index[i]);
                        }
                    }
				}
                if (i == sock_udp) {
                    // cerere de conexiune udp pe care o accepta serverul
                    msg message;
                    int rf = 0;
                    memset(&message, 0, sizeof(msg));
                    rf = recv(sock_udp, &message, sizeof(msg), 0);
                    DIE(rf < 0, "recv udp");

                    // se trimit date la clientii tcp abonati
                    string mesg = get_message(inet_ntoa(cli_addr_udp.sin_addr), ntohs(cli_addr_udp.sin_port), message);
                    mesg += "\n";
                    send_message(string(message.topic), mesg, ids_index, subscriptions, &non_active_ids);
                }
            }
		}
        if (i == 0) {
            // se trimite mesaj de inchidere la toti socketii
            // ramasi activi inainte de inchiderea serverului
            memset(buffer, 0, BUFLEN);
            sprintf(buffer, "exit");
            for (int j = 1; j <= fdmax; j++) {
                if (FD_ISSET(j, &read_fds)) {
                    n = send(j, buffer, strlen(buffer), 0);
                    DIE(n < 0, "send");
                    close(j);
                    FD_CLR(j, &read_fds);
                }
            }
            close(0);
            FD_CLR(0, &read_fds);
            break;
        }
	}

	close(sock_tcp);

	return 0;
}
