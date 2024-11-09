#ifndef PROTOCOLO_H
#define PROTOCOLO_H
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>


struct cliente_mensaje {
  long tipo;
  int pid;
  int num_registro;
  char descripcion[100];
};

struct servidor_mensaje {
  long tipo;
  int estado;
  int num_registro;
  char descripcion[100];
};

void enviar_mensaje_cliente(int cola,struct cliente_mensaje *msg);
void recibir_mensaje_servidor(int cola, struct servidor_mensaje *msg, int pid);
void manejar_cliente(int cola);
void manejar_servidor(int cola, const char *archivo);

#endif