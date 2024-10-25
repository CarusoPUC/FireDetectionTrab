#include "functions.h"

int main(int argc, char const *argv[])
{
    pthread_mutex_init(&mutex_principal, NULL);
    srand(time(NULL));
    pthread_t exibir_thread, fogo_thread, central_thread;
    int exibir_status, fogo_status, central_status;
    inicializar_floresta(floresta);
    FILE *log_fogo;
    log_fogo = fopen("fogo.log", "w");
    fclose(log_fogo);

    exibir_status = pthread_create(&exibir_thread, NULL, exibir_floresta, NULL);
    if (exibir_status) {
        printf("Erro ao criar thread exibir_floresta. Código de Retorno %d\n", exibir_status);
        exit(-1);
    }

    fogo_status = pthread_create(&fogo_thread, NULL, gerar_fogo, NULL);
    if (fogo_status) {
        printf("Erro ao criar thread gerar_fogo. Código de Retorno %d\n", fogo_status);
        exit(-1);
    }

    central_status = pthread_create(&central_thread, NULL, monitorar_central, NULL);
    if (central_status) {
        printf("Erro ao criar thread monitorar_central. Código de Retorno %d\n", central_status);
        exit(-1);
    }

    pthread_join(fogo_thread, NULL);

    pthread_mutex_destroy(&mutex_principal);
    return 0;
}
