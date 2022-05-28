#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define N_ESTUDANTES 50

sem_t salaMonitor;
sem_t monitor, estudante;
sem_t cadeirasVazias;

pthread_mutex_t mutex_thread;

int estudantesEsperando = 0;
int numeroCadeiras = 0;
int flagMonitorDorme = 0;
int idEstudanteAtendido = 0;

void *estudanteBody(void *arg){
  //id do estudante
  int *id = (int *)arg;
  printf("Estudante %d está indo buscar ajuda\n",*id);

  while(1){
    // requisita acesso exclusivo ao buffer
    pthread_mutex_lock(&mutex_thread);
    // verifica se tem mais estudantes esperando que cadeiras vazias
    if(estudantesEsperando < numeroCadeiras){
      //Estudante ocupa uma cadeira no corredor
      sem_wait(&cadeirasVazias);
      estudantesEsperando++;
      printf("Estudante %d está aguardando no corredor. Estudantes esperando = %d \n",*id,estudantesEsperando);
      // libera o buffer
      pthread_mutex_unlock(&mutex_thread);
      // aguarda espaço na sala
      sem_wait(&salaMonitor);
      idEstudanteAtendido = *id;
      printf("%d entrou na sala \n",*id);
      // libera cadeira no corredor
      sem_post(&cadeirasVazias);

      // acorda o monitor e é atendido
      sem_post(&estudante);
      sem_wait(&monitor);
      sleep(60);
      
      
      
    } else{
      pthread_mutex_unlock(&mutex_thread);
      printf("Estudante %d vai retornar em outro horário\n",*id);
      sleep(30);
    }
  }  
  //Fim da thread
  pthread_exit(NULL);
}

void *monitorBody(void *arg){
  while(1){
    if(estudantesEsperando>0){
      flagMonitorDorme = 0;
      sem_wait(&estudante);
      //requisita acesso exclusivo ao buffer
      pthread_mutex_lock( &mutex_thread );
      //ajuda o estudante
      int tempoDeAjuda = rand()%4;
      estudantesEsperando--;
      printf("Atendendo estudante %d por %d segundos. %d estudantes esperando \n",idEstudanteAtendido,tempoDeAjuda,estudantesEsperando);

      sleep(tempoDeAjuda);
      //libera o buffer
      pthread_mutex_unlock( &mutex_thread );
      //libera espaço para ajudar mais um estudante
      sem_post(&monitor);
      sem_post(&salaMonitor);
      
    } else{
      if(flagMonitorDorme == 0){
        printf("Sem estudantes esperando. Monitor vai dormir\n");
        flagMonitorDorme = 1;
      }
    }
  }

  //Fim da thread
  pthread_exit(NULL);
}

int main()
{

  int numEstudantes = N_ESTUDANTES;
  int* id_estudantes;
  //armazena temporariamente o id da thread
  pthread_t *thread_estudantes;
  pthread_t thread_mon;
  //armazena id das threads
  //long unsigned int thread_estudante[N_ESTUDANTES], thread_monitor;

  thread_estudantes = (pthread_t*)malloc(sizeof(pthread_t)*numEstudantes);
  
  id_estudantes = (int*)malloc(sizeof(int)*numEstudantes);
  
  while(numeroCadeiras <= 2){
    printf("Defina o numero de cadeiras fora da sala\n");
    if(scanf("%i",&numeroCadeiras)!=1){
      printf("apenas inteiros são válidos!\n");
    }
    //verificação se a quantidade cadeiras é menor ou igual a 2
    if(numeroCadeiras<=2){
        printf("Numero de cadeiras deve ser maior que 2\n");
    }
  }

  
  //Semaforo que indica ocupação máxima do numero de Cadeiras
  sem_init(&cadeirasVazias,0,numeroCadeiras);

  //Iniciamos com um espaço na sala do monitor
  sem_init(&salaMonitor,0,1);
  sem_init(&estudante,0,0);
  sem_init(&monitor,0,1);

  //inicia mutex
  pthread_mutex_init( &mutex_thread, NULL );

  //Thread do monitor
  if(pthread_create(&thread_mon,NULL,monitorBody,NULL)){
    perror("erro em pthread_create() monitor");
    exit(1);
  }

  //Threads (50) dos estudantes
  for(int i = 0; i < numEstudantes; i++){
   id_estudantes[i] = i+1;
    if(pthread_create(&thread_estudantes[i],NULL,estudanteBody,(void *)&id_estudantes[i])){
      perror("erro em pthread_create() estudante");
      exit(1);
    }
  }

  //Join na thread do monitor
  if(pthread_join(thread_mon,NULL)){
    perror("erro em pthread_join() monitor");
    exit(1);
  }
  //Join (50) nas threads dos estudantes
  for(int i = 0; i < numEstudantes; i++){
    if(pthread_join(thread_estudantes[i],NULL)){
      perror("erro em pthread_join() estudante");
      exit(1);
    }
  }


    return 0;
}
