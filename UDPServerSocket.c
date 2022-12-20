/*
 ============================================================================
 Name        : UDPServerSocket.c
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


void ErrorHandler(char *errorMessage);
void ClearWinSock();

int main(){

#ifdef WIN32
	WSADATA wsaData;

	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

	if(iResult != 0){
		ErrorHandler("Error at WSAStartup() \n");
		return 0;
	}
#endif

	int server_socket;
	struct sockaddr_in server_address; //structure for the server address
	struct sockaddr_in client_address; //structure for the client address
	struct hostent *host; //STRUCTURE PER IL MODULO GETHOSTBYADDR
	unsigned int client_addr_len;
	char echo_buffer[BUFFERSIZE];
	char end[] = "reset0";//STRINGA CHE FA TERMINARE LA CONNESSIONE
	char echo_string[BUFFERSIZE];
	//int echo_len = 0;
	char ok[] = "OK";//STRINGA DA INVIARE AL CLIENT
	int ok_len = strlen(ok);
	int recv_msg_size;//CONTATORE CARATTERI RICEVUTI
	int flag ;


	//CREAZIONE DELLA SOCKET DEL SERVER
	server_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(server_socket < 0){
		ErrorHandler("Socket creation failed.\n");
		ClearWinSock();
		return -1;
	}else{
		printf("Creazione corretta della socket.\n");
	}

	//ASSEGNAZIONE DI UN INDIRIZZO ALLA SOCKET
	memset(&server_address, 0 , sizeof(server_address)); // ensures that extra bytes contain 0
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

	//BIND THE SCOKET TO OUR SPECIFIED IP AND PORT
	if(bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0){
		ErrorHandler("Bind() failed.\n");
		closesocket(server_socket);
		ClearWinSock();
		return -1;
	}else{
		printf("Bindin effettuato correttamente.\n");
	}

	//LOOP PER COMUNICAZIONI
	while(1){

		flag = 0;
		client_addr_len =sizeof(client_address);
		memset(echo_buffer, 0, BUFFERSIZE);
		//RICEZIONE DI UNA COMUNICAZIONE DAL CLIENT
		recv_msg_size = recvfrom(server_socket, echo_buffer, BUFFERSIZE, 0 ,
				(struct sockaddr*)&client_address, &client_addr_len);

		echo_buffer[recv_msg_size] = 0;

		//IDENTIFICAZIONE DELL'HOST TRAMITE IP
		host = gethostbyaddr((char*)&client_address.sin_addr, sizeof(client_address.sin_addr), AF_INET);
		if(host == NULL){

			ErrorHandler("GethostByAddr failed.\n");
			system("pause");
			closesocket(server_socket);
			ClearWinSock();
			return -1;
		}


		printf("Handling client %s \n", host->h_name);
		printf("Messaggio ricevuto : %s \n\n", echo_buffer);

		//INVIA "OK" AL SERVER
		if(sendto(server_socket, ok, ok_len, 0, (struct sockaddr*)&client_address,
				sizeof(client_address)) != ok_len){
			ErrorHandler("send() sent a different number of bytes than expected");
			system("pause");
			closesocket(server_socket);
			ClearWinSock();
			return -1;
		}


		//RICEZIONE DELLE VOCALI SINGOLARMENTE, ED INVIO SINGOLARMENTE DEL CORRISPETTIVO MAIUSCOLO
		do{

			memset(echo_buffer, 0, BUFFERSIZE);
			memset(echo_string, 0, BUFFERSIZE);

			recv_msg_size = recvfrom(server_socket, echo_buffer, BUFFERSIZE, 0 ,
							(struct sockaddr*)&client_address, &client_addr_len);

			echo_buffer[recv_msg_size] = 0;

			//SE COMPARE LA STRING END, TERMINA LA CONNESIONE, CAMBIANDO FLAG
			if(strcmp(echo_buffer, end) != 0){

				printf("Messaggio ricevuto : %s \n", echo_buffer);

				//INVIA VOCALI MAIUSCOLA AL CLIENT
				memset(echo_string, 0, BUFFERSIZE);
				echo_string[0] = toupper(echo_buffer[0]);
				if(sendto(server_socket, &echo_string[0], 1, 0, (struct sockaddr*)&client_address,
						sizeof(client_address)) != 1){
					ErrorHandler("send() sent a different number of bytes than expected");
					system("pause");
					closesocket(server_socket);
					ClearWinSock();
					return -1;
				}
			}else{
				flag = 1;
			}

		}while(flag == 0);

		printf("\n\nNuova connessione.\n");
	}



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
