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
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int fd[2];
  /* dependiendo los argumentos lee o escribe el proceso */
  char *fifo_escritor = argv[1];
  char *fifo_lector = argv[2];
  char buffer[1000];

  printf("Chat iniciado. Escriba 'bye' para salir.\n");
  while (1) {
    if (read(fd[0], buffer, 100) > 0) {
      printf("Recibido: %s\n", buffer);
      if (strncmp(buffer, "bye", 3) == 0) {
        printf("El otro usuario ha terminado el chat.\n");
        break;
      }
    }

    printf("Tú: ");
    fgets(buffer, 100, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';

    write(fd[1], buffer, strlen(buffer) + 1);

    if (strcmp(buffer, "bye") == 0) {
      printf("Terminando el chat.\n");
      break;
    }
  }

  close(fd[1]);
  close(fd[0]);

  return 0;
}