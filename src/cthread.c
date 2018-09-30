#define _XOPEN_SOURCE 600

//para funcionar uso de contextos no mac
#ifdef _APPLE_
    pthread_threadid_np(thread,&tcb.id)
#else
#ifdef unix

#endif
#endif

#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/support.h"

#include <stdlib.h>

#define ERROR -1

//prioridades das threads
#define LOW 2
#define MEDIUM 1
#define HIGH 0

//filas e contextos globais
int currentThreadsId;
ucontext_t dispatcher_cntx;
PFILA2 apt_low, apt_medium, apt_high, blocked;
TCB_t *executing;

typedef struct block {
    TCB_t *thread;
    int tidBlocker;
    struct block *prox;
} block;

TCB_t * getThread(int tid) {

    if (FirstFila2(apt_high) == 0) {
        while (GetAtIteratorFila2(apt_high) != NULL) {
            TCB_t *temp = (TCB_t *) GetAtIteratorFila2(apt_high);
            if (temp->tid == tid) return temp;
            NextFila2(apt_high);
        }
    }
    if (FirstFila2(apt_medium) == 0) {
        while (GetAtIteratorFila2(apt_medium) != NULL) {
            TCB_t *temp = (TCB_t *) GetAtIteratorFila2(apt_medium);
            if (temp->tid == tid) return temp;
            NextFila2(apt_medium);
        }
    }
    if (FirstFila2(apt_low) == 0) {
        while (GetAtIteratorFila2(apt_low) != NULL) {
            TCB_t *temp = (TCB_t *) GetAtIteratorFila2(apt_low);
            if (temp->tid == tid) return temp;
            NextFila2(apt_low);
        }
    }
    if (FirstFila2(blocked) == 0) {
        while (GetAtIteratorFila2(blocked) != NULL) {
            block *blocked_thread = (block *) GetAtIteratorFila2(blocked);
            if (blocked_thread->thread->tid == tid) return blocked_thread->thread;
            NextFila2(blocked);
        }
    }
    return NULL;
}

void appendToRightQueue(TCB_t *thread){

  if (thread->prio == HIGH){
    AppendFila2(apt_high, (void *) thread);
  }
  else if(thread->prio == MEDIUM){
    AppendFila2(apt_medium, (void *) thread);
  }
  else{
    AppendFila2(apt_low, (void *) thread);
  }
  return;
}
void removeBlock(int tid) {

    FirstFila2(blocked);
    block *blocked_thread = (block *) GetAtIteratorFila2(blocked);
    while (blocked_thread != NULL) {
      if (blocked_thread->tidBlocker == tid){
        TCB_t *thread = blocked_thread->thread;
        thread->state = PROCST_APTO;
        appendToRightQueue(thread);
        DeleteAtIteratorFila2(blocked);
      }
      NextFila2(blocked);
      blocked_thread = (block *) GetAtIteratorFila2(blocked);
    }
}

int isBlocking(int tid) {

    FirstFila2(blocked);
    block *blocked_thread = (block *) GetAtIteratorFila2(blocked);
    while (blocked_thread != NULL) {
        if (blocked_thread->tidBlocker == tid) {
            return 1;
        }
        NextFila2(blocked);
        blocked_thread = (block *) GetAtIteratorFila2(blocked);
    }
    return 0;

}

void* dispatcher(void* arg){

  while(1){

    if (executing != NULL){
      //remove proceso que ja acabou de executando
      TCB_t *last_executing = executing;
      last_executing->state = PROCST_TERMINO;
      executing = NULL;

      //lIBERA PROCESSOS BLOQUEADOS PELO ULTIMO EXECUTANDO
      FirstFila2(blocked);
      if(isBlocking(last_executing->tid)){
        removeBlock(last_executing->tid);
      }
      free(last_executing);
    }
    //Seleciona thread apta com a de maior prioridade
    TCB_t* newThread;
    if (FirstFila2(apt_high) == 0){
      newThread = (TCB_t *) GetAtIteratorFila2(apt_high);
      DeleteAtIteratorFila2(apt_high);
    }
    else if (FirstFila2(apt_medium) == 0) {
      newThread = (TCB_t *) GetAtIteratorFila2(apt_medium);
      DeleteAtIteratorFila2(apt_medium);
    }
    else if (FirstFila2(apt_low) == 0) {
      newThread = (TCB_t *) GetAtIteratorFila2(apt_low);
      DeleteAtIteratorFila2(apt_low);
    }
    else{
      return NULL;
    }
    newThread->state = PROCST_EXEC;
    executing = newThread;
    swapcontext(&dispatcher_cntx, &newThread->context);
  }
  return NULL;
}

void createQueues(){

  apt_high = malloc(sizeof(PFILA2));
  CreateFila2(apt_high);

  apt_medium = malloc(sizeof(PFILA2));
  CreateFila2(apt_medium);

  apt_low = malloc(sizeof(PFILA2));
  CreateFila2(apt_low);

  blocked = malloc(sizeof(PFILA2));
  CreateFila2(blocked);

  TCB_t *main_thread = malloc(sizeof(TCB_t));
  main_thread->tid = 0;
  main_thread->state = PROCST_CRIACAO;
  main_thread->prio = LOW;
  getcontext(&(main_thread->context));

  main_thread->state = PROCST_EXEC;
  executing = main_thread;

  currentThreadsId = 0;

  dispatcher_cntx.uc_link = 0;
  dispatcher_cntx.uc_stack.ss_sp = malloc (8*1024);
  dispatcher_cntx.uc_stack.ss_size = 8*1024;
  dispatcher_cntx.uc_stack.ss_flags = 0;

  makecontext(&dispatcher_cntx, (void(*)(void)) dispatcher, 1, NULL);

}

int ccreate (void* (*start)(void*), void *arg, int prio){

// caso seja inicio do programa e as filas nao tenham sido inicializadas
  if (apt_high == NULL){
    createQueues();
  }

  if(prio < HIGH || prio > LOW ){
    printf("Prioridade invalida. Escolha um valor entre 0 e 2");
    return ERROR;
  }

  ucontext_t childcontext, dispatcher_cnt;

  TCB_t *thread = malloc(sizeof(TCB_t));
  thread->tid = ++currentThreadsId;
  thread->prio = prio;
  thread->state = PROCST_CRIACAO;

  getcontext(&childcontext);

  childcontext.uc_link=&dispatcher_cntx;
  childcontext.uc_stack.ss_sp = malloc (8*1024);
  childcontext.uc_stack.ss_size = 8*1024;
  childcontext.uc_stack.ss_flags = 0;

  makecontext(&childcontext, (void(*)(void)) start, 1, arg);

  appendToRightQueue(thread);
  thread->state = PROCST_APTO;
  thread->context=childcontext;
  if (executing != NULL){
    if (executing->prio < prio){
      TCB_t *last_executing = executing;
      last_executing->state = PROCST_APTO;
      appendToRightQueue(last_executing);
      executing = NULL;
      swapcontext(&(last_executing->context), &dispatcher_cntx);
    }
  }
  return currentThreadsId;
}

int cyield(void){

  TCB_t *last_executing = executing;
  executing = NULL;

  appendToRightQueue(last_executing);
  last_executing->state = PROCST_APTO;
  swapcontext(&(last_executing->context), &dispatcher_cntx);

  return 0;
}

int csetprio(int tid, int prio){

  if (executing->tid != tid){
    printf("Proibido alterar priodidade de outra thread ou de tid inexistente");
    return ERROR;
  }
  TCB_t *last_executing = executing;
  executing = NULL;

  last_executing->state = PROCST_APTO;
  last_executing->prio = prio;

  switch (prio) {
    case HIGH:
      if (FirstFila2(apt_high)){
        InsertBeforeIteratorFila2(last_executing);
      }
      else{
        appendToRightQueue(last_executing);
      }
      break;
    case MEDIUM:
      if (FirstFila2(apt_medium)){
        InsertBeforeIteratorFila2(last_executing);
      }
      else{
        appendToRightQueue(last_executing);
      }
      break;
    case LOW:
      if (FirstFila2(apt_low)){
        InsertBeforeIteratorFila2(last_executing);
      }
      else{
        appendToRightQueue(last_executing);
      }
      break;
  }
  swapcontext(&(last_executing->context), &dispatcher_cntx);
  return 0;
}

int cjoin(int tid){
  if (getThread(tid) == NULL) {
       return ERROR;
   } else if (isBlocking(tid)) {
       return ERROR;
   }

   TCB_t *last_executing = executing;

   executing = NULL;

   last_executing->state = 3;

   block *blocked_thread = malloc(sizeof(block));
   blocked_thread->tidBlocker = tid;
   blocked_thread->thread = last_executing;

   AppendFila2(blocked, (void *) blocked_thread);

   swapcontext(&(last_executing->context), &dispatcher_cntx);

   return 0;
}

int csem_init(csem_t *sem, int count){return ERROR;}

int cwait(csem_t *sem){return ERROR;}

int csignal(csem_t *sem){return ERROR;}

int cidentify (char *name, int size) {
	strncpy (name, " Jady Feijo - 00230210 \n Lucio Franco - 00252867;", size);
	return 0;
}
