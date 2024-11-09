/* Desarrolle un programa de chat usando archivos. Elija las estructuras de
datos y organización de archivos que mejor se adapte a la resolución de este
problema. Realice el diseño de la aplicación y luego su implementación */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

struct mensaje {
  int usuario;
  char texto[256];
};

void enviar(int usuario, const char *texto) {
  FILE *archivo = fopen("archivo.txt", "a");
  struct mensaje msg;
  msg.usuario = usuario;
  strncpy(msg.texto, texto, 256);
  fprintf(archivo, "%d: %s\n", msg.usuario, msg.texto);
  fclose(archivo);
}

void notificacion(int *last_message_pos) {
  char line[300];

  FILE *archivo = fopen("archivo.txt", "r");
  fseek(archivo, *last_message_pos, SEEK_SET);
  while (fgets(line, sizeof(line), archivo) != NULL) {
    printf("%s", line);
  }
  *last_message_pos = ftell(archivo);
  fclose(archivo);
}

int main(int argc, char *argv[]) {

  int usuario = atoi(argv[1]);
  char buffer[256];
  int last_message_pos = 0;

  pid_t pid = fork();

  if (pid == 0) {
    while (1) {
      notificacion(&last_message_pos);
    }
    exit(0);
  } else if (pid > 0) {
    while (1) {
      fgets(buffer, 256, stdin);
      buffer[strcspn(buffer, "\n")] = '\0';
      if (strcmp(buffer, "bye") == 0) {
        printf("Cerrando chat.\n");
        break;
      }
      enviar(usuario, buffer);
    }
    kill(pid, SIGTERM);
    wait(NULL);
  }
  return 0;
}