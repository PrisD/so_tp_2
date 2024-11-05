/* Desarrolle un programa de chat usando Cola de Mensajes. Repita el mismo
esquema de procesos que en el punto anterior (2 procesos independientes con un
hijo cada uno). Puede resolverse con una única cola de mensajes en donde se
escriben mensajes de distinto tipo (al menos requerirá mensajes de 2 tipos
distintos). Puede resolverlo con un único programa, del cual crea dos procesos
independientes y recibe argumentos por línea de comandos */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>


struct msg_buffer {
  long msg_type;
  char msg_text[1000];
};

void chat(int msgid, long send_type, long receive_type) {
  struct msg_buffer mensaje;
  char buffer[1000];

  pid_t pid = fork();

  if (pid == 0) {
    while (1) {
      memset(mensaje.msg_text, 0, 1000);
      msgrcv(msgid, &mensaje, sizeof(mensaje.msg_text), receive_type, 0);
      if (strcmp(mensaje.msg_text, "bye\n") == 0) {
        printf("El chat ha finalizado\n");
        break;
      } else {
        printf("- %s", mensaje.msg_text);
      }
    }
    exit(0); 
  } else {  
    while (1) {
      fgets(buffer, 1000, stdin);
      mensaje.msg_type = send_type;
      strcpy(mensaje.msg_text, buffer);
      msgsnd(msgid, &mensaje, sizeof(mensaje.msg_text), 0);

      if (strcmp(buffer, "bye\n") == 0) {
        printf("Has finalizado el chat\n");
        mensaje.msg_type = receive_type;
        strcpy(mensaje.msg_text, "bye\n");
        msgsnd(msgid, &mensaje, sizeof(mensaje.msg_text), 0);
        break;
      }
    }
    wait(NULL);
  }
}

int main(int argc, char *argv[]) {
  key_t key;
  int msgid;
  struct msg_buffer mensaje;

  key = ftok("chatColaMsg.c", 'B');
  msgid = msgget(key, 0666 | IPC_CREAT);

  printf("Chat iniciado. Escriba 'bye' para salir.\n");

  if (strcmp(argv[1], "1") == 0) { // Manejar para consola con atributo 1
    chat(msgid, 1, 2);
  } else if (strcmp(argv[1], "2") == 0) { // Manejar para consola con atributo 1
    chat(msgid, 2, 1);
    msgctl(msgid, IPC_RMID, NULL);
  } else {
    fprintf(stderr, "Parametro inexistente. \n");
    exit(1);
  }
  return 0;
}
