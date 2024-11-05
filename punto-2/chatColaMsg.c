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

#define MSG_SIZE 1000

struct msg_buffer {
  long msg_type;
  char msg_text[MSG_SIZE];
};

void chat(int msgid, long send_type, long receive_type) {
  struct msg_buffer mensaje;
  char buffer[MSG_SIZE];

  pid_t pid = fork();

  if (pid == 0) {
    while (1) {
      memset(mensaje.msg_text, 0, MSG_SIZE);
      msgrcv(msgid, &mensaje, sizeof(mensaje.msg_text), receive_type, 0);
      if (strcmp(mensaje.msg_text, "bye\n") == 0) {
        printf("El chat ha finalizado\n");
        break;
      } else {
        printf("\n- %s", mensaje.msg_text);
      }
    }
    exit(0); 
  } else {  
    while (1) {
      fgets(buffer, MSG_SIZE, stdin);
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

  key = ftok("chatColaMsg.c", 'B');
  if (key == -1) {
    perror("Error al generar la clave");
    exit(1);
  }

  msgid = msgget(key, 0666 | IPC_CREAT);
  if (msgid == -1) {
    perror("Error al crear la cola de mensajes");
    exit(1);
  }

  struct msg_buffer mensaje;
  while (msgrcv(msgid, &mensaje, sizeof(mensaje.msg_text), 0, IPC_NOWAIT) !=
         -1) {
  }

  printf("Chat iniciado. Escriba 'bye' para salir.\n");

  if (argc < 2) {
    fprintf(stderr, "Uso: %s <proceso>\n", argv[0]);
    exit(1);
  }

  if (strcmp(argv[1], "1") == 0) {
    chat(msgid, 1, 2);
  } else if (strcmp(argv[1], "2") == 0) {
    chat(msgid, 2, 1);
  } else {
    fprintf(stderr, "Parámetro inválido. Use '1' o '2'.\n");
    exit(1);
  }

  if (strcmp(argv[1], "2") ==
      0) {
    msgctl(msgid, IPC_RMID, NULL);
  }

  return 0;
}
