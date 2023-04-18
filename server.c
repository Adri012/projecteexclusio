#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <mysql.h>
#include <pthread.h>

MYSQL *conn;
int err;
MYSQL_RES *ress;
MYSQL_ROW row;

int contador;

//Estructura necesaria para acceso excluyente
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



int main(int argc, char *argv[])
{
	int sock_conn, sock_listen, ret;
	struct sockaddr_in serv_adr;
	char peticion[512];
	char res[512];
	
	char nombre[20];
	char contrasena[20];
	char mail[50];
	int puntos;
	char cons[80];
	char consid[80];
	
	conn = mysql_init(NULL);
	if (conn == NULL){
		printf("Error en la conexión: %u %s \n", mysql_errno(conn), mysql_error(conn));
		exit(1);
	}
	
	conn = mysql_real_connect(conn, "localhost","root", "mysql", "bd", 0, NULL,0);
	if (conn = NULL){
		printf("Error al iniciar la conexión: %u %s\n", mysql_errno(conn), mysql_error(conn));
		exit(1);
	}
	
	if((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Error al crear el socket");
	}
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(9030);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0){
		printf("Error bind");
	}
	if (listen(sock_listen,100)<0){
		printf("Error listen");
	}


	contador =0;
	int i;
	int sockets[100];
	pthread_t thread;
	i=0;
	
	for (i=0; i<40; i++){
		printf("Escuchando\n");
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("fino\n");
		
		ret = read(sock_conn, peticion, sizeof(peticion));
		peticion[ret]= '\0';
		
		printf("Peticion: %s\n", peticion);
		
		char * p = strtok(peticion, "/");
		printf("%s\n", p);
		int cod = atoi(p);
		printf("%d\n", cod);
		p = strtok(NULL, "/");
		//strcpy(nombre, p);
		
		
		
		if(cod == 0){
			close(sock_conn);
		}	
		
		else if(cod == 1){
			strcpy(nombre, p);
			printf("%s\n", nombre);
			p = strtok(NULL, "/");
			strcpy(mail, p);
			p = strtok(NULL, "/");
			strcpy(contrasena, p);
			
			printf("Codigo; %d, Nombre: %s, Contraseña: %s, Correo: %s\n", cod, nombre, contrasena, mail);
			
			sprintf(cons, "SELECT Jugador.NOMBRE FROM Jugador WHERE Jugador.NOMBRE ='%s' AND Jugador.CONTRASEÑA='%s';", nombre, contrasena);
			
			err = mysql_query (conn, cons);
			if (err != 0){
				printf("Error al consultar\n");
				exit(1);
			}
			
			ress = mysql_store_result (conn);
			row = mysql_fetch_row(ress);
			
			if(row == NULL){
				strcpy(consid,"SELECT MAX(Jugador.ID) FROM Jugador");
				err = mysql_query (conn, consid);
				if (err != 0){
					printf("Error al consultar");
					exit(1);
				}
				ress = mysql_store_result(conn);
				row = mysql_fetch_row(ress);
				int idj = atoi(row[0])+1;
				char porfin[100];
				sprintf(porfin,"INSERT INTO Jugador(ID,NOMBRE,CONTRASEÑA,MAIL,PUNTOS) VALUES(%d,'%s','%s','%s',0)",idj,nombre,contrasena,mail);
				
				err = mysql_query (conn, porfin);
				if(err!=0){
					printf("Error al insertar datos\n");
					exit(1);
					sprintf(porfin, "NO REGISTRADO");
				}
				else{
					sprintf(porfin,"SI");
				}
			}
			else{
				sprintf(res,"NO REGISTRADO");
			}
		}
		
		else if(cod==2){
			strcpy (nombre, p);
			p = strtok(NULL, "/");
			strcpy(contrasena, p);
			
			printf(cons,"Codigo: &d, Nombre: %s, Contraseña: %s\n",cod, nombre, contrasena);
			
			sprintf(cons, "SELECT Jugador.NOMBRE, Jugador.CONTRASEÑA FROM Jugador WHERE  Jugador.NOMBRE= '%s' AND Jugador.CONTRASEÑA = '%s'", nombre,contrasena);
			err = mysql_query(conn, cons);
			if (err !=0){
				printf("Errror al consultar %u %s\n",mysql_errno(conn), mysql_error(conn));
				exit(1);
			}
			
			ress = mysql_store_result(conn);
			row = mysql_fetch_row(ress);
			if(row == NULL){
				sprintf(res,"INCORRECTO");
			}
			else{
				sprintf(res,"SI");
			}
		}
		
		else if(cod ==3){
			printf ("Codigo: %d", cod);
			sprintf(cons, "SELECT Jugador.NOMBRE FROM Jugador WHERE Jugador.PUNTOS = MAX(Jugador.PUNTOS)");
			
			err = mysql_query(conn,cons);
			if(err != 0){
				printf("Error al consultar");
				exit(1);
			}
			ress = mysql_store_result(conn);
			row = mysql_fetch_row(ress);
			if (row == NULL){
				printf("No va");
			}
			else{
				sprintf(res,"%s",row[0]);
			}
		}
		
		printf("Respuesta: %s \n", res);
		write(sock_conn,res,strlen(res));
		}
		
		if ((codigo ==1)||(codigo==2)|| (codigo==3)) {
			pthread_mutex_lock( &mutex ); //No me interrumpas ahora
			contador = contador +1;
			pthread_mutex_unlock( &mutex); //ya puedes interrumpirme
		}
		
		
		pthread_create (&thread, NULL, AtenderCliente,&sockets[i]);
		i=i+1;
		//close(sock_conn);
	}
	//close(sock_conn);
	
	