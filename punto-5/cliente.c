#include "protocolo.h"

void enviar_mensaje_cliente(int queue, struct cliente_mensaje *msg) {
  msgsnd(queue, msg, sizeof(struct cliente_mensaje) - sizeof(long), 0);
}

void recibir_mensaje_servidor(int queue, struct servidor_mensaje *msg,
                              int pid) {
  msgrcv(queue, msg, sizeof(struct servidor_mensaje) - sizeof(long), pid, 0);
}

void mensaje_flujo(int queue) {
  struct cliente_mensaje client_msg;
  struct servidor_mensaje server_msg;
  char input[200];
  int pid, record_num;
  char description[100];

  client_msg.tipo = 1;
  client_msg.pid = getpid();

  printf("Pid proceso: %d\n", client_msg.pid);
  printf("Ingrese comando en el formato: pid,num_registro,descripcion o "
         "escriba 'bye' para salir.\n");

  while (1) {
    fgets(input, sizeof(input), stdin);

    // Si el usuario ingresa "bye", termina el programa
    if (strncmp(input, "bye", 3) == 0) {
      printf("Terminando el cliente...\n");
      break;
    }

    if (sscanf(input, "%d,%d,%99[^\n]", &pid, &record_num, description) != 3) {
      printf("Formato incorrecto. Intente de nuevo.\n");
      continue;
    }

    client_msg.pid = pid;
    client_msg.num_registro = record_num;
    strncpy(client_msg.descripcion, description,
            sizeof(client_msg.descripcion));
    enviar_mensaje_cliente(queue, &client_msg);
    recibir_mensaje_servidor(queue, &server_msg, client_msg.pid);

    printf("servidor- %s\n", server_msg.descripcion);
  }
}

int main() {
  key_t key = 0xA;
  int queue = msgget(key, 0666 | IPC_CREAT);
  mensaje_flujo(queue);
  return 0;
}