#include "functions.h"

char mat[TAM][TAM];
Node nodes[THR][THR];
pthread_t sensor_threads[THR][THR];
pthread_mutex_t mutex, mtx_prt; 

int convert_x(int id){
	int x;
	if (id % 10 == 0)
		id--;
	x = 3*(id/10) +1;
	return x;
}

int convert_y(int id){
	int y;
	if (id % 10 == 0){
		y = 28;
		return y;
	}
	y = 3 * (id%10) - 2;
	return y;
}

void inicializar_floresta(char mat[TAM][TAM]) {
	memset(mat, LIVRE, TAM * TAM * sizeof(mat[0][0]));
	int cont_id = 1;

	for (int linha = 1, row_index = 0; row_index < THR && linha < TAM; row_index++, linha += 3) {
		for (int coluna = 1, col_index = 0; col_index < THR && coluna < TAM; col_index++, coluna += 3) {
			mat[linha][coluna] = NO;
			nodes[row_index][col_index] = (Node){.x = linha, .y = coluna, .id = cont_id, .ativo = 1, 
                                                 .borda = (row_index == 0 || row_index == 9 || col_index == 0 || col_index == 9) ? 1 : 0};
			pthread_create(&sensor_threads[row_index][col_index], NULL, sensor, (void *)(intptr_t)cont_id);
			cont_id++;
		}
	}
}



void transmitir_msg(int pos_x, int pos_y) {
	if (nodes[pos_x][pos_y].borda) return;

	nodes[pos_x][pos_y].C = 0;

	Node *atual = &nodes[pos_x][pos_y];
	memcpy(nodes[pos_x - 1][pos_y].baixo, atual->centro, sizeof(atual->centro));
	nodes[pos_x - 1][pos_y].B = 1;

	memcpy(nodes[pos_x + 1][pos_y].cima, atual->centro, sizeof(atual->centro));
	nodes[pos_x + 1][pos_y].CM = 1;

	memcpy(nodes[pos_x][pos_y + 1].esquerda, atual->centro, sizeof(atual->centro));
	nodes[pos_x][pos_y + 1].E = 1;

	memcpy(nodes[pos_x][pos_y - 1].direita, atual->centro, sizeof(atual->centro));
	nodes[pos_x][pos_y - 1].D = 1;
}


void distribuir_msg(int pos_x, int pos_y) {
	if (nodes[pos_x][pos_y].borda) return;

	int enviado = 0;
	size_t tam_msg = sizeof(nodes[pos_x][pos_y].centro);

	if (nodes[pos_x][pos_y].CM) {
		nodes[pos_x][pos_y].CM = 0;
		memcpy(nodes[pos_x + 1][pos_y].cima, nodes[pos_x][pos_y].cima, tam_msg);
		nodes[pos_x + 1][pos_y].CM = 1;
		memcpy(nodes[pos_x][pos_y + 1].esquerda, nodes[pos_x][pos_y].cima, tam_msg);
		nodes[pos_x][pos_y + 1].E = 1;
		memcpy(nodes[pos_x][pos_y - 1].direita, nodes[pos_x][pos_y].cima, tam_msg);
		nodes[pos_x][pos_y - 1].D = 1;
		enviado = 1;
	} else if (nodes[pos_x][pos_y].B) {
		nodes[pos_x][pos_y].B = 0;
		memcpy(nodes[pos_x - 1][pos_y].baixo, nodes[pos_x][pos_y].baixo, tam_msg);
		nodes[pos_x - 1][pos_y].B = 1;
		memcpy(nodes[pos_x][pos_y + 1].esquerda, nodes[pos_x][pos_y].baixo, tam_msg);
		nodes[pos_x][pos_y + 1].E = 1;
		memcpy(nodes[pos_x][pos_y - 1].direita, nodes[pos_x][pos_y].baixo, tam_msg);
		nodes[pos_x][pos_y - 1].D = 1;
		enviado = 1;
	} else if (nodes[pos_x][pos_y].E) {
		nodes[pos_x][pos_y].E = 0;
		memcpy(nodes[pos_x - 1][pos_y].baixo, nodes[pos_x][pos_y].esquerda, tam_msg);
		nodes[pos_x - 1][pos_y].B = 1;
		memcpy(nodes[pos_x + 1][pos_y].cima, nodes[pos_x][pos_y].esquerda, tam_msg);
		nodes[pos_x + 1][pos_y].CM = 1;
		memcpy(nodes[pos_x][pos_y + 1].esquerda, nodes[pos_x][pos_y].esquerda, tam_msg);
		nodes[pos_x][pos_y + 1].E = 1;
		enviado = 1;
	} else if (nodes[pos_x][pos_y].D) {
		nodes[pos_x][pos_y].D = 0;
		memcpy(nodes[pos_x - 1][pos_y].baixo, nodes[pos_x][pos_y].direita, tam_msg);
		nodes[pos_x - 1][pos_y].B = 1;
		memcpy(nodes[pos_x + 1][pos_y].cima, nodes[pos_x][pos_y].direita, tam_msg);
		nodes[pos_x + 1][pos_y].CM = 1;
		memcpy(nodes[pos_x][pos_y - 1].direita, nodes[pos_x][pos_y].direita, tam_msg);
		nodes[pos_x][pos_y - 1].D = 1;
		enviado = 1;
	}

	if (enviado) sleep(1);
}

void limpar_msg(int msg[6]){
	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			if (memcmp(msg,nodes[i][j].cima,sizeof(msg[0])*6) == 0){
				nodes[i][j].CM = 0;
			}
			else if (memcmp(msg,nodes[i][j].baixo,sizeof(msg[0])*6) == 0){
				nodes[i][j].B = 0;
			}
			else if (memcmp(msg,nodes[i][j].esquerda,sizeof(msg[0])*6) == 0){
				nodes[i][j].E = 0;
			}
			else if (memcmp(msg,nodes[i][j].direita,sizeof(msg[0])*6) == 0){
				nodes[i][j].D = 0;
			}
			else if (memcmp(msg,nodes[i][j].centro,sizeof(msg[0])*6) == 0){
				nodes[i][j].C = 0;
			}
		}
	}

}

void apagar_fogo(int x, int y){
	sleep(1);
	if (mat[x][y] == QUEIMADO || mat[x][y] == FOGO)
		mat[x][y] = LIVRE;
}


void *print_floresta(void *args){
	while(1)
	{
		pthread_mutex_lock(&mtx_prt);
		system("clear");
		int idx, idy;
		int sensor = 0;
		
		for (int i = 0; i < TAM; ++i)
		{
			printf("\t");
			for (int j = 0; j < TAM; ++j)
			{
				
				if (mat[i][j] == '*') 
				{	
					idx = ((i+2)/3)-1;
					idy = ((j+2)/3)-1;
					sensor = 1;
					mat[i][j] = MORTO;
				}

				if (mat[i][j] == QUEIMADO || mat[i][j] == FOGO)
					printf("@  ");
				else if (mat[i][j] == MORTO)
					printf("%c ",MORTO);
				else
					printf("%c  ",mat[i][j]);
				
			}
			printf("\n");
		}
		sensor = 0;
		pthread_mutex_unlock(&mtx_prt);
		sleep(1);
	}
}

void *fogo(void *args){
	FILE *log_thr;
	int x,y;
	int idx, idy;
	while(1){
		sleep(3);
		x = rand()%30;
		y = rand()%30;
		pthread_mutex_lock(&mtx_prt); 
		if (mat[x][y] == NO){
			mat[x][y] = '*';
			idy = ((y+2)/3)-1;
			idx = ((x+2)/3)-1;
			nodes[idx][idy].ativo = 0;
		}
		else if (mat[x][y] == MORTO){}
		else
			mat[x][y] = FOGO;
		pthread_mutex_unlock(&mtx_prt);
	}
}

void *sensor(void *args){
	int id = (intptr_t) args;
	int idx, idy;
	int x, y;
	int sent;
	x = convert_x(id);
	y = convert_y(id);
	idx = ((x+2)/3)-1;
	idy = ((y+2)/3)-1;
	while(1){
		if (nodes[idx][idy].ativo == 1)
		{
			sent = 0;
			pthread_mutex_lock(&mutex);
			for (int i = x-1; i <= x + 1; ++i)
			{
				for (int j = y-1; j <= y + 1; ++j)
				{			
					if (i == x && j == y)
						continue;
					if (mat[i][j] == FOGO)
					{	
						mat[i][j] = QUEIMADO;
						nodes[idx][idy].centro[0] = nodes[idx][idy].id;
						nodes[idx][idy].centro[1] = i;
						nodes[idx][idy].centro[2] = j;
						nodes[idx][idy].C = 1;
						if(nodes[idx][idy].borda == 0){	
							enviar_msg(idx, idy);
							sent = 1;
						}
					}
				}
			}
			if(sent == 0)
				prop_msg(idx,idy);
			
			pthread_mutex_unlock(&mutex);
			sleep(1);
		}
	}
}

void *central(void *args){
	int msg[6];
	size_t size = sizeof(msg);
	FILE *log_fire;
	while(1){
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				if (nodes[i][j].borda == 0)
					continue;

				if (nodes[i][j].CM == 1){
					nodes[i][j].CM = 0;
					log_fire = fopen("fogo.log", "a");
					memcpy(msg, nodes[i][j].cima, size);
					fprintf(log_fire,"Thread %d apagou fogo em [%d][%d]\n",msg[0],msg[1],msg[2]);
					fclose(log_fire);
					limpar_msg(msg);				
					apagar_fogo(msg[1],msg[2]);
				}
				else if (nodes[i][j].B == 1){
					nodes[i][j].B = 0;
					log_fire = fopen("fogo.log", "a");
					memcpy(msg, nodes[i][j].baixo, size);
					fprintf(log_fire,"Thread %d apagou fogo em [%d][%d]\n",msg[0],msg[1],msg[2]);
					fclose(log_fire);					
					limpar_msg(msg);				
					apagar_fogo(msg[1],msg[2]);
				}
				else if (nodes[i][j].E == 1){
					nodes[i][j].E = 0;
					log_fire = fopen("fogo.log", "a");
					memcpy(msg, nodes[i][j].esquerda, size);
					fprintf(log_fire,"Thread %d apagou fogo em [%d][%d]\n",msg[0],msg[1],msg[2]);
					fclose(log_fire);					
					limpar_msg(msg);				
					apagar_fogo(msg[1],msg[2]);
				}
				else if (nodes[i][j].D == 1){
					nodes[i][j].D = 0;
					log_fire = fopen("fogo.log", "a");
					memcpy(msg, nodes[i][j].direita, size);
					fprintf(log_fire,"Thread %d apagou fogo em [%d][%d]\n",msg[0],msg[1],msg[2]);
					fclose(log_fire);					
					limpar_msg(msg);				
					apagar_fogo(msg[1],msg[2]);
				}
				else if (nodes[i][j].C == 1){
					nodes[i][j].C = 0;
					log_fire = fopen("fogo.log", "a");
					memcpy(msg, nodes[i][j].centro, size);
					fprintf(log_fire,"Thread %d apagou fogo em [%d][%d]\n",msg[0],msg[1],msg[2]);
					fclose(log_fire);					
					limpar_msg(msg);			
					apagar_fogo(msg[1],msg[2]);
				}
			}
		}
		sleep(1);
	}
}
