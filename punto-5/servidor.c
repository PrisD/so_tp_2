#include "protocolo.h"

int msg_queue;
FILE *file_pointer;

void signal_handler(int signo) {
  if (file_pointer)
    fclose(file_pointer);
  msgctl(msg_queue, IPC_RMID, NULL);
  exit(0);
}

void run_server(int queue, const char *filename) {
  struct cliente_mensaje client_msg;
  struct servidor_mensaje server_msg;
  msg_queue = queue;

  file_pointer = fopen(filename, "r+b");
  if (!file_pointer) {
    file_pointer = fopen(filename, "w+b");
    for (int i = 0; i < 1000; i++) {
      server_msg.estado = 0;
      memset(server_msg.descripcion, 0, sizeof(server_msg.descripcion));
      fwrite(&server_msg, sizeof(struct servidor_mensaje), 1, file_pointer);
    }
  }

  signal(SIGINT, signal_handler);

  printf("Servidor levantado y listo para recibir mensajes.\n");

  while (1) {
    msgrcv(queue, &client_msg, sizeof(struct cliente_mensaje) - sizeof(long), 1,
           0);

    fseek(file_pointer,
          client_msg.num_registro * sizeof(struct servidor_mensaje), SEEK_SET);
    fread(&server_msg, sizeof(struct servidor_mensaje), 1, file_pointer);

    if (strcmp(client_msg.descripcion, "leer") == 0) {
      if (server_msg.estado == 1) {
        server_msg.tipo = client_msg.pid;
        server_msg.estado = 1;
      } else {
        server_msg.estado = 0;
        snprintf(server_msg.descripcion, sizeof(server_msg.descripcion),
                 "Registro %d está vacío", client_msg.num_registro);
      }
    } else if (strcmp(client_msg.descripcion, "borrar") == 0) {
      if (server_msg.estado == 1) {
        server_msg.estado = 2;
        snprintf(server_msg.descripcion, sizeof(server_msg.descripcion),
                 "Registro %d borrado", client_msg.num_registro);

        fseek(file_pointer,
              client_msg.num_registro * sizeof(struct servidor_mensaje),
              SEEK_SET);
        fwrite(&server_msg, sizeof(struct servidor_mensaje), 1, file_pointer);
      } else {
        server_msg.estado = 0;
        snprintf(server_msg.descripcion, sizeof(server_msg.descripcion),
                 "Registro %d ya está vacío o borrado",
                 client_msg.num_registro);
      }
    } else {
      server_msg.estado = 1;
      server_msg.num_registro = client_msg.num_registro;
      strncpy(server_msg.descripcion, client_msg.descripcion,
              sizeof(server_msg.descripcion));
      fseek(file_pointer,
            client_msg.num_registro * sizeof(struct servidor_mensaje),
            SEEK_SET);
      fwrite(&server_msg, sizeof(struct servidor_mensaje), 1, file_pointer);
    }

    server_msg.tipo = client_msg.pid;
    msgsnd(queue, &server_msg, sizeof(struct servidor_mensaje) - sizeof(long),
           0);
  }
}

int main(int argc, char *argv[]) {
  key_t key = 0xA;
  int queue = msgget(key, 0666 | IPC_CREAT);

  printf("Servidor levantado y en espera de solicitudes...\n");
  run_server(queue, argv[1]);

  return 0;
}