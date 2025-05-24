#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_THREADS 5
#define NUM_ITERATIONS 3
#define INITIAL_RESOURCES 10

// estructura del monitor
typedef struct {
    int available_resources;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    FILE *logfile;
} Monitor;

Monitor monitor = {
    .available_resources = INITIAL_RESOURCES,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
    .logfile = NULL
};

// disminuir recursos disponibles
int decrease_count(int count, long thread_id) {
    pthread_mutex_lock(&monitor.mutex);
    fprintf(monitor.logfile, "Iniciando thread %ld - Intentando tomar %d recursos\n", thread_id, count);
    
    while (monitor.available_resources < count) {
        fprintf(monitor.logfile, " %ld - NELSON! Recursos actuales: %d (se necesita %d)\n", 
                thread_id, monitor.available_resources, count);
        pthread_cond_wait(&monitor.cond, &monitor.mutex); // espera a que haya recursos disponibles
    }
    
    monitor.available_resources -= count;
    fprintf(monitor.logfile, " %ld - Mutex adquirido, entrando al monitor\n", thread_id);
    fprintf(monitor.logfile, " %ld - (!) %d recursos tomados. Recursos restantes: %d\n", 
            thread_id, count, monitor.available_resources);
    
    pthread_mutex_unlock(&monitor.mutex);
    return 0;
}

// aumentar recursos disponibles
int increase_count(int count, long thread_id) {
    pthread_mutex_lock(&monitor.mutex);
    
    monitor.available_resources += count;
    fprintf(monitor.logfile, " %ld - %d recursos devueltos. Recursos totales: %d\n", 
            thread_id, count, monitor.available_resources);
    
    pthread_cond_broadcast(&monitor.cond); // despierta a todos los threads esperando
    pthread_mutex_unlock(&monitor.mutex);
    return 0;
}

void *thread_function(void *thread_id) {
    long tid = (long)thread_id;
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        int resources_needed = rand() % 3 + 1; // asignar recursos a usar, entre 1-3 recursos
        
        fprintf(monitor.logfile, "Iniciando thread %ld - (Iteración %d) Se consumirán %d recursos\n", tid, i+1, resources_needed);
        
        if (decrease_count(resources_needed, tid) == 0) {
            // simulando el uso de recursos
            int sleep_time = rand() % 3 + 1; // tiempo aleatorio entre 1 y 3 segundos
            fprintf(monitor.logfile, " %ld - Usando %d recursos por %d segundos\n", tid, resources_needed, sleep_time);
            sleep(sleep_time);
            
            increase_count(resources_needed, tid);
        }
    }
    
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    srand(time(NULL));
    
    monitor.logfile = fopen("monitor_log.txt", "w");
    if (monitor.logfile == NULL) {
        perror("Error al abrir archivo de bitácora");
        return 1;
    }
    
    fprintf(monitor.logfile, "Iniciando programa con monitor\n");
    fprintf(monitor.logfile, "Recursos iniciales: %d\n", INITIAL_RESOURCES);
    fprintf(monitor.logfile, "Creando threads: %d\n\n", NUM_THREADS);
    
    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_function, (void *)i);
        fprintf(monitor.logfile, "Thread %ld creado\n", i);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    fprintf(monitor.logfile, "\nTodos los threads han terminado. Recursos finales: %d\n", monitor.available_resources);
    
    fclose(monitor.logfile);
    return 0;
}