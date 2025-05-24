#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_THREADS 5
#define NUM_ITERATIONS 3
#define INITIAL_RESOURCES 10

int available_resources = INITIAL_RESOURCES;
sem_t semaphore;
FILE *logfile;

void *thread_function(void *thread_id) {
    long tid = (long)thread_id;
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // intenta tomar un recurso
        sem_wait(&semaphore);
        fprintf(logfile, "Iniciando thread %ld - (Iteración %d) Intentando tomar recurso\n", tid, i+1);
        
        if (available_resources > 0) {
            available_resources--;
            fprintf(logfile, " %ld - Semaforo abierto con éxito\n", tid);
            fprintf(logfile, " %ld - (!) Recurso tomado. Recursos restantes: %d\n", tid, available_resources);
            sem_post(&semaphore);
            
            // simulación de uso del recurso
            int sleep_time = rand() % 3 + 1; // tiempo aleatorio entre 1 y 3 segundos
            fprintf(logfile, " %ld - Usando recurso por %d segundos\n", tid, sleep_time);
            sleep(sleep_time);
            
            // devolviendo recurso
            sem_wait(&semaphore);
            available_resources++;
            fprintf(logfile, " %ld - Recurso devuelto. Recursos totales: %d\n", tid, available_resources);
            sem_post(&semaphore);
        } else {
            fprintf(logfile, " %ld - No hay recursos disponibles. Esperando...\n", tid);
            sem_post(&semaphore);
            sleep(1); // esperar antes de reintentar
            i--;
        }
    }
    
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    srand(time(NULL));
    
    logfile = fopen("semaphore_log.txt", "w");
    if (logfile == NULL) {
        perror("Error al abrir archivo de bitácora");
        return 1;
    }
    
    fprintf(logfile, "Iniciando programa con semáforos\n");
    fprintf(logfile, "Recursos iniciales: %d\n", INITIAL_RESOURCES);\
    fprintf(logfile, "Creando threads: %d\n\n", NUM_THREADS);
    
    sem_init(&semaphore, 0, 1); // semáforo
    
    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_function, (void *)i);
        fprintf(logfile, "Thread %ld creado\n", i);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    fprintf(logfile, "Todos los threads han terminado. Recursos finales: %d\n", available_resources);
    
    sem_destroy(&semaphore);
    fclose(logfile);
    return 0;
}