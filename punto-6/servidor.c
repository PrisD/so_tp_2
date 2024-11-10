#include "protocolo.h"

int cola_msg;
FILE *file_pointer;

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
  struct servidor_mensaje msg_servidor;
  msg_queue = queue;

  file_pointer = fopen(filename, "r+b");
  if (!file_pointer) {
    file_pointer = fopen(filename, "w+b");
    for (int i = 0; i < 1000; i++) {
      msg_servidor.estado = 0;
      memset(msg_servidor.descripcion, 0, sizeof(msg_servidor.descripcion));
      fwrite(&msg_servidor, sizeof(struct servidor_mensaje), 1, file_pointer);
    }
  }

  signal(SIGINT, signal_handler);

  printf("Servidor levantado y listo para recibir mensajes.\n");

  while (1) {
    msgrcv(queue, &client_msg, sizeof(struct cliente_mensaje) - sizeof(long), 1,
           0);

    fseek(file_pointer,
          client_msg.num_registro * sizeof(struct servidor_mensaje), SEEK_SET);
    fread(&msg_servidor, sizeof(struct servidor_mensaje), 1, file_pointer);

    if (strcmp(client_msg.descripcion, "lock") == 0) {
      if (msg_servidor.pid_lock == 0) {
        msg_servidor.pid_lock = client_msg.pid;
        msg_servidor.estado = 1;
        snprintf(msg_servidor.descripcion, 100,
                 "registro %d bloqueado con éxito", client_msg.num_registro);

        fseek(file_pointer,
              client_msg.num_registro * sizeof(struct servidor_mensaje) +
                  offsetof(struct servidor_mensaje, pid_lock),
              SEEK_SET);
        fwrite(&msg_servidor.pid_lock, sizeof(int), 1, file_pointer);
        fwrite(&msg_servidor.estado, sizeof(int), 1, file_pointer);
        fflush(file_pointer);
      } else {
        msg_servidor.estado = 0;
        snprintf(msg_servidor.descripcion, 100,
                 "registro %d ya reservado a pid %d", client_msg.num_registro,
                 msg_servidor.pid_lock);
      }
    } else if (strcmp(client_msg.descripcion, "unlock") == 0) {
      if (msg_servidor.pid_lock == client_msg.pid) {
        msg_servidor.pid_lock = 0;
        msg_servidor.estado = 1;
        snprintf(msg_servidor.descripcion, 100,
                 "registro %d desbloqueado", client_msg.num_registro);

        fseek(file_pointer,
              client_msg.num_registro * sizeof(struct servidor_mensaje) +
                  offsetof(struct servidor_mensaje, pid_lock),
              SEEK_SET);
        fwrite(&msg_servidor.pid_lock, sizeof(int), 1, file_pointer);
        fwrite(&msg_servidor.estado, sizeof(int), 1, file_pointer);
        fflush(file_pointer);
      } else {
        msg_servidor.estado = 0;
      }
    } else if (msg_servidor.pid_lock == client_msg.pid) {
      if (strcmp(client_msg.descripcion, "leer") == 0) {
        msg_servidor.tipo = client_msg.pid;
        if (msg_servidor.estado == 1 && strlen(msg_servidor.descripcion) > 0) {
          // Mensaje leído exitosamente
        } else {
          snprintf(msg_servidor.descripcion, 100, "registro %d vacio",
                   client_msg.num_registro);
        }
      } else if (strcmp(client_msg.descripcion, "borrar") == 0) {
        if (msg_servidor.estado == 1) {
          msg_servidor.estado = 0;
          memset(msg_servidor.descripcion, 0, 100);
          snprintf(msg_servidor.descripcion, 100, "registro %d borrado",
                   client_msg.num_registro);

          fseek(file_pointer,
                client_msg.num_registro * sizeof(struct servidor_mensaje),
                SEEK_SET);
          fwrite(&msg_servidor, sizeof(struct servidor_mensaje), 1,
                 file_pointer);
          fflush(file_pointer);
        } else {
          msg_servidor.estado = 0;
          snprintf(msg_servidor.descripcion, 100,
                   "registro %d vacio",
                   client_msg.num_registro);
        }
      } else {
        msg_servidor.estado = 1;
        msg_servidor.num_registro = client_msg.num_registro;
        strncpy(msg_servidor.descripcion, client_msg.descripcion, 100);

        fseek(file_pointer,
              client_msg.num_registro * sizeof(struct servidor_mensaje),
              SEEK_SET);
        fwrite(&msg_servidor, sizeof(struct servidor_mensaje), 1, file_pointer);
        fflush(file_pointer);
      }
    } else {
      msg_servidor.estado = 0;
      snprintf(msg_servidor.descripcion, 100, "el registro  %d esta reservado ",
               client_msg.num_registro);
    }

    msg_servidor.tipo = client_msg.pid;
    msgsnd(msg_queue, &msg_servidor,
           sizeof(struct servidor_mensaje) - sizeof(long), 0);
  }
}

int main(int argc, char *argv[]) {
  key_t key = 0xA;
  int queue = msgget(key, 0666 | IPC_CREAT);

  printf("Servidor levantado y en espera de solicitudes...\n");
  run_server(queue, argv[1]);

  return 0;
}
