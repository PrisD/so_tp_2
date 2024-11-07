/* Desarrolle un programa de chat usando Memoria Compartida. Repita el mismo
esquema de procesos que en el punto anterior (2 procesos independientes con un
hijo cada uno). Puede resolverse con una única área de memoria con espacio para
dos mensajes (digamos, de 256 bytes cada una, ésta será la limitación del
programa, no acepta mensajes de más de 256 caracteres). Se requerirá de dos
semáforos para que, cuando un proceso escriba un mensaje no permita que otro
proceso lo lea hasta que la escritura no se haya completado y cuando un proceso
lea, impedir que otro proceso escriba el mensaje que se está leyendo. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

struct mensaje {
  char msg[256];
  int listo[2];
};

void P(int semid, int sem) {
  struct sembuf buf;
  buf.sem_num = sem;
  buf.sem_op = -1;
  buf.sem_flg = 0;
  semop(semid, &buf, 1);
}
void V(int semid, int sem) {
  struct sembuf buf;
  buf.sem_num = sem;
  buf.sem_op = 1;
  buf.sem_flg = 0;
  semop(semid, &buf, 1);
}

int main(int argc, char *argv[]) {
  key_t key;

  key = ftok("chatSharedM.c", 'B');


  int proc_num = atoi(argv[1]);
  if (proc_num != 0 && proc_num != 1) {
    fprintf(stderr, "El número de proceso debe ser 0 o 1.\n");
    exit(EXIT_FAILURE);
  }

  int shmid = shmget(key, sizeof(struct mensaje), 0666 | IPC_CREAT);
  struct mensaje *mem = (struct mensaje *)shmat(shmid, NULL, 0);

  // Crear semáforos
  int semid = semget(key, 2, 0666 | IPC_CREAT);
  semctl(semid, 0, SETVAL, 1); // Semáforo de escritura
  semctl(semid, 1, SETVAL, 1); // Semáforo de lectura

  mem->listo[0] = 0;
  mem->listo[1] = 0;

  pid_t pid = fork();

  if (pid == 0) { // Proceso hijo para recibir mensajes
    while (1) {
      P(semid, 1); // Bloquear lectura
      if (mem->listo[1 - proc_num] == 1) {
        printf("Recibido: %s\n", mem->msg);
        if (strcmp(mem->msg, "bye") == 0) {
          printf("El otro usuario ha salido del chat.\n");
          break;
        }
        mem->listo[1 - proc_num] = 0; // Marcar mensaje como leído
        fflush(stdout);
      }
      V(semid, 1);    // Liberar lectura
      usleep(100000); // Reducir consumo de CPU
    }
  } else { // Proceso padre para enviar mensajes
    while (1) {
      char buffer[256];
      printf("Tú: ");
      fgets(buffer, 256, stdin);
      buffer[strcspn(buffer, "\n")] = '\0'; // Eliminar salto de línea

      P(semid, 0); // Bloquear escritura
      strncpy(mem->msg, buffer, 256);
      mem->listo[proc_num] = 1; // Marcar mensaje como listo

      V(semid, 0); // Liberar escritura

      if (strcmp(buffer, "bye") == 0) {
        printf("Saliendo del chat...\n");
        break;
      }
    }
    wait(NULL);
  }

  shmdt(mem);
  if (pid != 0) {
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
  }

  return 0;
}
