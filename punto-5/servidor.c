#include "protocolo.h"

int cola_msg;
FILE *file;

void manejador_signal(int signo) {
  if (file)
    fclose(file);
  msgctl(cola_msg, IPC_RMID, NULL);
  exit(0);
}

void manejar_servidor(int cola, const char *archivo) {
  struct cliente_mensaje msg_cliente;
  struct servidor_mensaje msg_servidor;
  cola_msg = cola;

  file = fopen(archivo, "r+b");
  if (!file) {
    file = fopen(archivo, "w+b");
    for (int i = 0; i < 1000; i++) {
      msg_servidor.estado = 0;
      memset(msg_servidor.descripcion, 0, 100);
      fwrite(&msg_servidor, sizeof(struct servidor_mensaje), 1, file);
    }
  }

  signal(SIGINT, manejador_signal);

  while (1) {
    msgrcv(cola, &msg_cliente, sizeof(struct cliente_mensaje) - sizeof(long), 1, 0);

    fseek(file, msg_cliente.num_registro * sizeof(struct servidor_mensaje),
          SEEK_SET);
    fread(&msg_servidor, sizeof(struct servidor_mensaje), 1, file);

    if (strcmp(msg_cliente.descripcion, "leer") == 0) {
      if (msg_servidor.estado == 1) {
        msg_servidor.tipo = msg_cliente.pid;
        msg_servidor.estado = 1;
      } else {
        msg_servidor.estado = 0;
        snprintf(msg_servidor.descripcion, 100, "Registro %d está vacío",
                 msg_cliente.num_registro);
      }
    } else if (strcmp(msg_cliente.descripcion, "borrar") == 0) {
      if (msg_servidor.estado == 1) {
        msg_servidor.estado = 2;
        snprintf(msg_servidor.descripcion, 100, "Registro %d borrado",
                 msg_cliente.num_registro);

        fseek(file, msg_cliente.num_registro * sizeof(struct servidor_mensaje),
              SEEK_SET);
        fwrite(&msg_servidor, sizeof(struct servidor_mensaje), 1, file);
      } else {
        msg_servidor.estado = 0;
        snprintf(msg_servidor.descripcion, 100,
                 "Registro %d ya está vacío o borrado",
                 msg_cliente.num_registro);
      }
    } else {
      msg_servidor.estado = 1;
      msg_servidor.num_registro = msg_cliente.num_registro;
      strcpy(msg_servidor.descripcion, msg_cliente.descripcion);
      fseek(file, msg_cliente.num_registro * sizeof(struct servidor_mensaje), SEEK_SET);
      fwrite(&msg_servidor, sizeof(struct servidor_mensaje), 1, file);
    }

    // enviar respuesta al cliente
    msg_servidor.tipo = msg_cliente.pid;
    msgsnd(cola, &msg_servidor, sizeof(struct servidor_mensaje) - sizeof(long), 0);
  }
}

int main(int argc, char *argv[]) {
  key_t key = 0xA;
  int cola = msgget(key, 0666 | IPC_CREAT);
  manejar_servidor(cola, argv[1]);
  return 0;
}