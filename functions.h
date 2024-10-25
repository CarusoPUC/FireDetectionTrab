#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_
#include <time.h>
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#define ARVORE 'T'
#define FOGO '@'
#define ESPACO_VAZIO '-'
#define QUEIMADO '/'
#define DESATIVADO '!'
#define TAMANHO 30
#define NUM_THREADS 10

extern char floresta[TAMANHO][TAMANHO];
typedef struct Sensor {
    int pos_x, pos_y;
    int borda;
    int acima[6], abaixo[6], a_esquerda[6], a_direita[6], centro[6];
    int flag_acima, flag_abaixo, flag_esquerda, flag_direita, flag_centro;
    int identificador;
    int ativo;
} Sensor;
extern Sensor sensores[NUM_THREADS][NUM_THREADS];
extern pthread_t threads_sensores[NUM_THREADS][NUM_THREADS];
extern pthread_mutex_t mutex_principal, mutex_impressao;


void inicializar_floresta(char floresta[TAMANHO][TAMANHO]);
void *exibir_floresta(void *args);
void *gerar_fogo(void *args);
void *executar_sensor(void *args);
void *monitorar_central(void *args);
void enviar_mensagem(int pos_x, int pos_y);
void propagar_mensagem(int pos_x, int pos_y);
void limpar_mensagem(int mensagem[6]);
void apagar_fogo(int pos_x, int pos_y);
int converter_pos_x(int identificador);
int converter_pos_y(int identificador);

#endif
