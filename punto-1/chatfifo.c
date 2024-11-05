/* Realizar un chat entre procesos independientes usando fifo’s. Cada proceso
independiente recibe por línea de comandos 2 parámetros: el nombre del fifo de
escritura y el nombre del fifo de lectura. Puede resolver todo el problema
programando un único programa (chatfifo.c) y creando dos procesos a partir de un
mismo programa, ejecutándolo desde dos terminales distintas con distintos
argumentos. Establezca un protocolo para salir del chat, por ejemplo, cuando se
tipea la palabra “bye” el proceso deja de enviar y recibir mensajes a través de
los fifo’s. */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int fd_escritura, fd_lectura;
  /* dependiendo los argumentos lee o escribe el proceso */
  char *fifo_escritor = argv[1];
  char *fifo_lector = argv[2];
  char buffer[1000];

  fd_escritura = open(fifo_escritor, O_RDWR);
  fd_lectura = open(fifo_lector, O_RDONLY);

  printf("Chat iniciado. Escriba 'bye' para salir.\n");

  pid_t pid = fork();

  if (pid == 0) {
    while (1) {
      memset(buffer, 0, sizeof(buffer));
      if (read(fd_lectura, buffer, sizeof(buffer))) {
        if (!strcmp(buffer, "bye\n")) {
          printf("El chat ha finalizado\n");
          break;
        } else {
          printf("%s", buffer);
        }
      }
    }
  } else {
    while (1) {
      fgets(buffer, sizeof(buffer), stdin);
      write(fd_escritura, buffer, sizeof(buffer));
      if (!strcmp(buffer, "bye\n")) {
        printf("Has finalizado el chat\n");
        break;
      }
    }
    wait(0);
  }

  close(fd_escritura);
  close(fd_lectura);

  return 0;
}