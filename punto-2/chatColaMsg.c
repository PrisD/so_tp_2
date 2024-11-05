#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define MSG_SIZE 1000
#define MSG_TYPE_1 1
#define MSG_TYPE_2 2

// Estructura del mensaje
struct msg_buffer {
  long msg_type;
  char msg_text[MSG_SIZE];
};

void chat(int msgid, long send_type, long receive_type) {
  struct msg_buffer message;
  char buffer[MSG_SIZE];

  pid_t pid = fork();

  if (pid == 0) { // Proceso hijo (receptor)
    while (1) {
      memset(message.msg_text, 0, MSG_SIZE);
      msgrcv(msgid, &message, sizeof(message.msg_text), receive_type, 0);
      if (strcmp(message.msg_text, "bye\n") == 0) {
        printf("El chat ha finalizado\n");
        break;
      } else {
        printf("\n- %s", message.msg_text);
      }
    }
    exit(0); // Termina el proceso hijo al salir del bucle
  } else {   // Proceso padre (emisor)
    while (1) {
      fgets(buffer, MSG_SIZE, stdin);
      message.msg_type = send_type;
      strcpy(message.msg_text, buffer);
      msgsnd(msgid, &message, sizeof(message.msg_text), 0);

      if (strcmp(buffer, "bye\n") == 0) {
        printf("Has finalizado el chat\n");
        // Envía el mensaje de salida también al tipo de mensaje del receptor
        message.msg_type = receive_type;
        strcpy(message.msg_text, "bye\n");
        msgsnd(msgid, &message, sizeof(message.msg_text), 0);
        break;
      }
    }
    wait(NULL); // Espera a que el proceso hijo termine
  }
}

int main(int argc, char *argv[]) {
  key_t key;
  int msgid;

  // Crear clave única para la cola de mensajes
  key = ftok("chatColaMsg.c", 'B');
  if (key == -1) {
    perror("Error al generar la clave");
    exit(1);
  }

  // Crear o obtener la cola de mensajes
  msgid = msgget(key, 0666 | IPC_CREAT);
  if (msgid == -1) {
    perror("Error al crear la cola de mensajes");
    exit(1);
  }

  // Limpiar mensajes residuales en la cola antes de iniciar el chat
  struct msg_buffer message;
  while (msgrcv(msgid, &message, sizeof(message.msg_text), 0, IPC_NOWAIT) !=
         -1) {
    // Limpia la cola eliminando mensajes residuales
  }

  printf("Chat iniciado. Escriba 'bye' para salir.\n");

  if (argc < 2) {
    fprintf(stderr, "Uso: %s <proceso>\n", argv[0]);
    exit(1);
  }

  if (strcmp(argv[1], "1") == 0) {
    // Proceso 1: envía mensajes de tipo 1, recibe mensajes de tipo 2
    chat(msgid, MSG_TYPE_1, MSG_TYPE_2);
  } else if (strcmp(argv[1], "2") == 0) {
    // Proceso 2: envía mensajes de tipo 2, recibe mensajes de tipo 1
    chat(msgid, MSG_TYPE_2, MSG_TYPE_1);
  } else {
    fprintf(stderr, "Parámetro inválido. Use '1' o '2'.\n");
    exit(1);
  }

  // Eliminar la cola de mensajes cuando termine
  if (strcmp(argv[1], "2") ==
      0) { // Eliminar la cola cuando el proceso 2 finaliza
    msgctl(msgid, IPC_RMID, NULL);
  }

  return 0;
}
