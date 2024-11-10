#include "protocolo.h"

void enviar_mensaje_cliente(int queue, struct cliente_mensaje *msg) {
  msgsnd(queue, msg, sizeof(struct cliente_mensaje) - sizeof(long), 0);
}

void recibir_mensaje_servidor(int queue, struct servidor_mensaje *msg,
                              int pid) {
  msgrcv(queue, msg, sizeof(struct servidor_mensaje) - sizeof(long), pid, 0);
}

void mensaje_flujo(int queue) {
  struct cliente_mensaje msg_cliente;
  struct servidor_mensaje msg_servidor;
  char input[200];
  int pid, record_num;
  char descripcion[100];

  msg_cliente.tipo = 1;
  msg_cliente.pid = getpid();

  printf("Pid proceso: %d\n", msg_cliente.pid);
  printf("Ingrese comando en el formato: pid,num_registro,descripcion o "
         "escriba 'bye' para salir.\n");

  while (1) {
    fgets(input, sizeof(input), stdin);

    if (strncmp(input, "bye", 3) == 0) {
      printf("Terminando el cliente...\n");
      break;
    }

    if (sscanf(input, "%d,%d,%99[^\n]", &pid, &record_num, descripcion) != 3) {
      printf("Formato incorrecto. Intente de nuevo.\n");
      continue;
    }

    msg_cliente.pid = pid;
    msg_cliente.num_registro = record_num;
    strncpy(msg_cliente.descripcion, descripcion,
            sizeof(msg_cliente.descripcion));
    enviar_mensaje_cliente(queue, &msg_cliente);
    recibir_mensaje_servidor(queue, &msg_servidor, msg_cliente.pid);

    printf("servidor- %s\n", msg_servidor.descripcion);
  }
}

int main() {
  key_t key = 0xA;
  int queue = msgget(key, 0666 | IPC_CREAT);
  mensaje_flujo(queue);
  return 0;
}