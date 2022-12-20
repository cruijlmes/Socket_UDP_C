/*
 ============================================================================
 Name        : UDPClientSocket.c
 Author      : cruijlmes
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#ifdef WIN32
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>//USO DELLE FUNZIONI RELATIVE ALLE OPERAZIONI SULLE STRINGHE E
					//USO DI MEMSET PER LA PULIZIA DEL BUFFER

#define BUFFERSIZE 255
#define PORT 48000

void ClearWinSock();
void ErrorHandler(char *errorMessage);


int main(){


	int client_socket;
	struct sockaddr_in server_address;
	struct hostent *host;//STRUCTURE PER IL MODULO GETHOSTBYNAME
	struct sockaddr_in from_addr;
	unsigned int from_size;
	char vocali[] = {'a', 'e', 'i', 'o', 'u', 'A', 'E', 'I', 'O', 'U'};
	char end[] = "reset0"; //stringa per terminare la connessione
	char ciao[] = "ciao";//stringa da inviare al server
	char string_A[BUFFERSIZE]; //STRINGA DA CUI SI FARA' LA CONTA DELLE VOCALI
	char echo_string[BUFFERSIZE];
	char echo_buffer[BUFFERSIZE];
	char hostname[BUFFERSIZE];
	int n_vocali = 9;
	int ciao_len = strlen(ciao);
	int end_len = strlen(end);
	int port;
	int string_vocali = 0; //CONTATORE VOCALI
	int string_size; //CONTATORE DELLA LUNGHEZZA DI STRING_A


	int flag = 0;

	//RICHIEDE NOME HOST E PORTA FINCHè NON SI INSERISCE "LOCALHOST"
	//SULLA PORTA NON C'E' NESSUN CONTROLLO. SUL SERVER E' IMPOSTATA PORTA 48000
	//CONTROLLO MEDIANTE FLAG
	do{

#ifdef WIN32
	WSADATA wsaData;

	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

	if(iResult != 0){
		ErrorHandler("Error at WSAStartup() \n");
		return 0;
	}
#endif

		system("cls");
		memset(hostname, 0, BUFFERSIZE);
		printf("Inserisci nome host del server : ");
		scanf("%s", hostname);

		port = 0;
		printf("Inserisci numero di porta del server(porta settata su serveR = 48000): ");
		scanf("%d", &port);


		//CREAZIONE DELLA SOCKET
		client_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(client_socket < 0){
			ErrorHandler("Socket creation failed.\n");
			system("pause");
			closesocket(client_socket);
			ClearWinSock();
			return -1;
		}else{
			printf("Socket creata correttamente.\n");
		}


		//COSTRUZIONE DELL'INDIRIZZO DEL SERVER
		memset(&server_address, 0, sizeof(server_address));
		server_address.sin_family = PF_INET;
		server_address.sin_port = htons(port);

		host = gethostbyname(hostname);
		if(host == NULL){
			ErrorHandler("GethostByName failed.\n");
			system("pause");
			closesocket(client_socket);
			ClearWinSock();

		}else{

			server_address.sin_addr.s_addr = *((long *) host->h_addr);

			//INVIA CIAO AL SERVER
			if(sendto(client_socket, ciao, ciao_len, 0, (struct sockaddr*) &server_address,
					sizeof(server_address)) != ciao_len){
				ErrorHandler("send() sent a different number of bytes than expected");
				system("pause");
				closesocket(client_socket);
				ClearWinSock();
				return -1;
			}

			//CONTROLLO SULLA CORRISPONENZA DELLA PORTA
			//SE CORRISPONDE, ESCE DAL CICLO
			if(port == PORT){

				flag = 1;
			}else{
				printf("\nPorta sbagliata. Inserisci la porta giusta.");
				system("pause");
			}

		}


	}while(flag == 0);


	//RICEZIONE OK DAL SERVER
	from_size = sizeof(from_addr);
	memset(echo_buffer, 0, BUFFERSIZE);
	recvfrom(client_socket, echo_buffer, BUFFERSIZE, 0 ,
				(struct sockaddr*)&from_addr, &from_size);
	if(server_address.sin_addr.s_addr != from_addr.sin_addr.s_addr){
		printf("Pacchetto da fonte sconosciuta");
	}else{
		printf("Ricevuto : %s\n", echo_buffer);
	}

	//RICHIESTA STRINGA A
	//ECHO_BUFFER FUNGE DA CONTENITORE A TUTTE LE VOCALI IN STRINGA A
	//CHE VENGONO CONTATE E SALVATE IL TOTALE IN STRING_VOCALI
	memset(echo_buffer, 0, BUFFERSIZE);
	memset(string_A, 0, BUFFERSIZE);
	printf("\nInserisci Stringa A : ");
	scanf("%s", string_A);
	string_size = strlen(string_A);

	//CONTA DEL NUMERO DI VOCALI
	for(int i = 0; i < string_size; i++){

		if(isalpha(string_A[i]) != 0){

			for(int k = 0; k < n_vocali; k++){

				if(string_A[i] == vocali[k]){

					echo_buffer[string_vocali] = string_A[i];
					string_vocali = string_vocali + 1;
					k = n_vocali;

				}
			}
		}
	}

	printf("\nNumero vocali %d\n", string_vocali);
	system("pause");

	//INVIO DELLE VOCALI AL SERVER
	for(int i = 0; i < string_vocali; i++){


		if(sendto(client_socket, &echo_buffer[i], 1, 0, (struct sockaddr*) &server_address,
				sizeof(server_address)) != 1){
			ErrorHandler("send() sent a different number of bytes than expected");
			system("pause");
			closesocket(client_socket);
			ClearWinSock();
			return -1;
		}

		//RICEZIONE DELLE STESSA VOCALE MA IN MAIUSCOLO
		memset(echo_string, 0, BUFFERSIZE);
		recvfrom(client_socket, echo_string, BUFFERSIZE, 0 ,
						(struct sockaddr*)&from_addr, &from_size);

		printf("Ricevuto : %s \n", echo_string);


	}

	//INVIO NOTIFICA AL SERVER CHE NON NON SI DEVONO PASSARE ALTRI DATI
	if(sendto(client_socket, end, end_len, 0, (struct sockaddr*)&server_address, sizeof(server_address)) != end_len){

		ErrorHandler("send() sent a different number of bytes than expected");
		system("pause");
		closesocket(client_socket);
		ClearWinSock();
		return -1;

	}

	printf("\nFine connessione.\n");
	closesocket(client_socket);
	ClearWinSock();
	system("pause");

	return 0;

}















void ErrorHandler(char *errorMessage){
	printf("%s", errorMessage);
}

void ClearWinSock(){
#if defined WIN32
	WSACleanup();
#endif
}
