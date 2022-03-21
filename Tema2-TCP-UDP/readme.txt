Nedelcu Stefan Laurentiu
323CA

	Tema 2
	
	
	Pentru inceput, am dezactiat buffering-ul pentru server si pentru clientii
TCP. Am pornit in server un socket TCP si unul UDP si le-am adaugat in multimea
de socketi, urmati de socketul pentru stdin.

	De la clientii tcp se pot primi:
		- cereri de conectare: se receptioneaza id-ul clientului si se verifica
		daca se afla in map-ul de clienti activi. In cazul in care se afla, se
		afiseaza mesaj si se trimite mesaj la client pentru a se inchide. In caz
		contrar, se adauga clientul nou in lista de socketi, se scoate (daca
		exista) din map-ul de id-uri inactive si i se trimit mesajele nerecep-
		tionate in timpul in care a fost deconectat. Apoi se introduce in mapul
		de id-uri active si se afiseaza mesajul corespunzator;
		- cereri de deconectare: se introduce id-ul in mapul de id-uri inactive,
		se scoate din mapul de id-uri active si se afiseaza mesaj;
		- mesaje de abonare sau dezabonare la topicuri: se foloseste functia
		update_subscriptions care actualizeaza lista de topicuri a id-ului dat
		din mapul subscriptions.
		
	De la clientii udp se pot primi mesaje pe diferite topicuri care se recep-
tioneaza cu ajutorul structurii msg. Functia get_message primeste adresa ip si
portul clientului udp, urmat de mesajul receptionat si intoarce mesajul care
trebuie trimis la clientii tcp abonati la topicul respectiv. Functia send_message
trimite mesajul clientilor activi si il stocheaza pentru cei inactivi.

	De la stdin se poate primi comanda exit care trimite mesaj de inchidere la
toti socketii conectati si inchide serverul.

	In tcp_client, am adaugat in multimea de socketi un socket tcp si cel de
stdin. Am trimis, apoi, id-ul primit ca argument la conectare la server.
	De la stdin se pot primi mesaje de subscribe sau unsubscribe pentru care se
afiseaza un mesaj sugestiv si se trimit la server pentru sa se procesa.
	Mesajul de exit se poate primi si de la stdin, si de la server, avand
acelasi efect si anume inchiderea clientului tcp.
	In cazul in care se primeste alt mesaj, in afara de cel de exit de la server,
se va afisa.

