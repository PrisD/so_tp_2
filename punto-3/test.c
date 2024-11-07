#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

int main() {
  // ruta definida para la key
  const char *path = "/tmp/chat_shm_key";

  // crear el archivo si no existe
  int fd = open(path, O_CREAT | O_EXCL, 0666);
  if (fd == -1) {
    perror("ERRO AL CREAR LA CLAVE");
  } else {
    close(fd);
  }

  key_t key = ftok(path, 'A');
  if (key == -1) {
    perror("ERROR AL GENERAR CLAVE DE LA COLA DE MENSAJES");
    exit(EXIT_FAILURE);
  }

  printf("CLAVE CREADA CON EXITO, key = %d\n", key);
  return 0;
}