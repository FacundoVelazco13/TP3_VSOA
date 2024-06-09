#include "sp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char private_name[MAX_PRIVATE_NAME] = "user-";
static char* spread_name = NULL;
static char private_group[MAX_GROUP_NAME];
static  mailbox Mbox;

#define MAX_MSG_LEN	    1024
#define MAX_GROUP_N	    16
#define ERROR_MESS		0
#define ERROR_MESS_STR	"error"

//Menu options
#define JOIN_GROUP  'j'
#define LEAVE_GROUP  'l'
#define SEND_MSG    's'
#define RECEIVE_MSG 'r'
#define QUIT        'q'
//Service type
#define UNRELIABLE_MESS_STR "unrel"
#define RELIABLE_MESS_STR	"rel"
#define FIFO_MESS_STR		"fifo"
#define CAUSAL_MESS_STR		"causal"
#define AGREED_MESS_STR		"atom"
#define SAFE_MESS_STR		"safe"

void printMenu();
int optManager();
void joinGroup(char* group_name);
void leaveGroup(char* group_name);
void sendMessage(char* message, int16 service, char* group_name);
void receiveMessages();
void receiveSingleMessage();
int16 getServiceId(char* service_type);
char* getServiceType(int16 service_type);

int main( int argc, char *argv[] ){
    int	ret;

    if(argc == 2){
        strcat(private_name,argv[1]);
    }else{
        printf("Format %s <Numero de nodo> \n",argv[0]);
        return -1;
    }
    
	ret = SP_connect(spread_name, private_name,0,1,&Mbox,private_group);
	if( ret < 0 ){
		SP_error( ret );
		exit(0);
	}
    do{
        printMenu();
        ret = optManager();
    }while(ret == 0);
    
    SP_disconnect(Mbox);
    return 0;
}

void printMenu(){
	printf("----------------------------------------------------------------\n");
	printf("|                         SPREAD APP                           |\n");
	printf("----------------------------------------------------------------\n\n");
	printf("                        %c - UNIRSE A GRUPO\n", JOIN_GROUP);
	printf("                        %c - SALIR DE GRUPO\n", LEAVE_GROUP);
	printf("                        %c - ENVIAR MENSAJE\n", SEND_MSG);
	printf("                        %c - RECIBIR MENSAJE\n\n", RECEIVE_MSG);
	printf("                        %c - SALIR\n\n", QUIT);
}

int optManager(){
	
    char msg[MAX_MSG_LEN];
	int16 service_type_id;
	char group_name[MAX_GROUP_NAME];
	
	char opc = QUIT;
	char service_type[8];

	printf("OPCION: ");
	scanf(" %c", &opc);
	printf("----------------------------------------------------------------\n\n");
	switch (opc){
	case JOIN_GROUP:
		printf("Grupo: ");
		scanf(" %s", group_name);
		joinGroup(group_name);
		break;
	case LEAVE_GROUP:
		printf("Grupo: ");
		scanf(" %s", group_name);
		leaveGroup(group_name);
		break;
	case SEND_MSG:
		printf("Mensaje: ");
		getchar();
		fgets(msg, MAX_MSG_LEN, stdin);
    
		do{
			printf("Tipo de servicio (%s, %s, %s, %s, %s, %s): ", UNRELIABLE_MESS_STR, RELIABLE_MESS_STR, FIFO_MESS_STR, CAUSAL_MESS_STR, AGREED_MESS_STR, SAFE_MESS_STR);
			scanf(" %s", service_type);
			service_type_id = getServiceId(service_type);
			if (service_type_id == ERROR_MESS)
				printf("Error: el tipo de servicio especificado no es valido\n");
		
        }while (service_type_id == ERROR_MESS);
        
		printf("Grupo: ");
		scanf(" %s", group_name);
		
		sendMessage(msg, service_type_id, group_name);
		break;
	case RECEIVE_MSG:
		receiveMessages();
		break;
	case QUIT:
		return 1;
		break;
	default:
		printf("Opcion invalida\n\n");
		break;
	}
    return 0;
}
void joinGroup(char* group_name){
	int error = SP_join(Mbox, group_name);
	if (error < 0){
		printf("Error: no se pudo unir al grupo\n");
		SP_error(error);
	}
	else
		printf("Unido al grupo exitosamente\n");
}
void leaveGroup(char* group_name){
	int error = SP_leave(Mbox, group_name);
	if (error < 0){
		printf("Error: no se pudo salir del grupo\n");
		SP_error(error);
	}
	else
		printf("Se salio del grupo exitosamente\n");
}
void sendMessage(char* message, int16 service, char* group_name){
	int error = SP_multicast(Mbox, service, group_name, 1, strlen(message), message);
	if (error < 0){
		printf("Error: no pudo enviarse el mensaje\n");
		SP_error(error);
	}
	else
		printf("Mensaje enviado exitosamente\n");
}
void receiveMessages(){
	if (SP_poll(Mbox) > 0)
		while (SP_poll(Mbox) > 0)
			receiveSingleMessage();
	else
		printf("Ningun mensaje a la vista!\n");
}
void receiveSingleMessage(){
	char message[MAX_MSG_LEN] = {' '};
	char sender[MAX_GROUP_NAME];
	char groups[MAX_GROUP_N][MAX_GROUP_NAME];
	int	groups_num = -1;
	membership_info membership_info;
	int	service_type;
	int16 message_type = -1;
	int	endian_mismatch;
	
	int error = SP_receive(Mbox, &service_type, sender, MAX_GROUP_N, &groups_num, groups, &message_type, &endian_mismatch, MAX_MSG_LEN, message);
	if (error < 0){
		printf("No se pudo recibir el mensaje \n");
		SP_error(error);
	}
	else{
		if (Is_regular_mess(service_type)){
			printf("====== Mensaje regular ======\n");
			printf("-- Tipo de servicio: %s\n", getServiceType(service_type));
			printf("-- Remitente: %s\n", sender);
			printf("-- Grupo: %s\n", &groups[0][0]);
			printf("-- Mensaje: %s\n", message);
			printf("-- Size: %li bytes\n\n", strlen(message));
		}
		else if (Is_membership_mess(service_type)){
			error = SP_get_memb_info(message, service_type, &membership_info);
			if (error < 0){
				printf("Mensaje de membresia invalido\n");
				SP_error(error);
			}
			else{
				if (Is_reg_memb_mess(service_type)){
					printf("====== Mensaje de membresia regular ======\n");
					if (Is_caused_join_mess(service_type))
						printf("El miembro: %s ,se unio al grupo\n", membership_info.changed_member);
					else if (Is_caused_leave_mess(service_type))
						printf("El miembro: %s ,salio del grupo\n", membership_info.changed_member);
					else if (Is_caused_disconnect_mess(service_type))
						printf("El miembro: %s ,se desconecto del grupo.\n", membership_info.changed_member);
					else if (Is_caused_network_mess(service_type))
						printf("Problema con la red de un usuario spread\n");

					printf("Grupo: %s\n", sender);
					printf("Miembros:\n");
					for (int i = 0; i < groups_num; i++)
						printf("\t%i - %s\n", i + 1, &groups[i][0]);
					printf("\n");
				}
				else if (Is_transition_mess(service_type)){
					printf("====== Mensaje de membresia transicional ======\n");
					printf("Grupo: %s\n\n", sender);
				}
				else if (Is_caused_leave_mess(service_type))
					/*
						Ver bien este mensaje.
					
					*/
					printf("## Miembro actual deja el grupo \n");
				else
					printf("## Mensaje de membresia invalido\n\n");
			}
		}
	}
}
int16 getServiceId(char* service_type){
	if (strcmp(service_type, UNRELIABLE_MESS_STR) == 0)
		return UNRELIABLE_MESS;
	else if (strcmp(service_type, RELIABLE_MESS_STR) == 0)
		return RELIABLE_MESS;
	else if (strcmp(service_type, FIFO_MESS_STR) == 0)
		return FIFO_MESS;
	else if (strcmp(service_type, CAUSAL_MESS_STR) == 0)
		return CAUSAL_MESS;
	else if (strcmp(service_type, AGREED_MESS_STR) == 0)
		return AGREED_MESS;
	else if (strcmp(service_type, SAFE_MESS_STR) == 0)
		return SAFE_MESS;
	else
		return ERROR_MESS;
}
char* getServiceType(int16 service_type){
	if (Is_unreliable_mess(service_type))
		return UNRELIABLE_MESS_STR;
	else if (Is_reliable_mess(service_type))
		return RELIABLE_MESS_STR;
	else if (Is_fifo_mess(service_type))
		return FIFO_MESS_STR;
	else if (Is_causal_mess(service_type))
		return CAUSAL_MESS_STR;
	else if (Is_agreed_mess(service_type))
		return AGREED_MESS_STR;
	else if (Is_safe_mess(service_type))
		return SAFE_MESS_STR;
	else
		return ERROR_MESS_STR;
}