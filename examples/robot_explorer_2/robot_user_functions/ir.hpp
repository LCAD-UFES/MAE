#ifndef _IR_H
#define _IR_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

// Definitions

// Pinos
#define PIN4_SERIAL	0x0002
#define PIN6_SERIAL	0x0100
#define PIN7_SERIAL	0x0004

// Dispositivos
#define DEV_SERIAL_PORT_1	"/dev/ttyS0"
#define DEV_SERIAL_PORT_2	"/dev/ttyS1"

// Constantes da porta paralela
#define LPT1_BASE       0x378

// Constanste do sinal positivo e negativo
#define SIG_POS "#"
#define SIG_NEG "."
#define CHAR_SIG_POS '#'
#define CHAR_SIG_NEG '.'

// Numero maximo de amostras
#define N_MAX_AMOSTRAS	24000

// Arquivo de configuração do IR
#define ARQ_CONFIG	"ir.cfg"
#define ERRO_ARQ_IR(arq)		{printf("Nao foi possivel o arquivo '%s'. Programa abortado!\n",arq); exit(1);}

#define FREQ_PORTADORA			40000
#define MAX_N_SINAL			1024
#define ARQ_SINAL_FRENTE		"./forward.dat"
#define ARQ_SINAL_ESQUERDA		"./turn_left.dat"
#define ARQ_SINAL_DIREITA		"./turn_right.dat"

#define TURN_LEFT			0
#define TURN_RIGHT			1
#define MOVE_FORWARD			2

// Macros
#define ERRO_ACESSO_SERIAL		{printf("Nao foi possivel acessar a porta serial\n");exit(1);}
#define ERRO_ACESSO_PARALELA		{printf("Nao foi possivel acessar a porta paralela\n");exit(1);}
#define ERRO_ARQUIVO_ENTRADA		{printf("Nao foi possivel abrir o arquivo de entrada.\n\n"); exit(1);}
#define ERRO_ARQUIVO_CONFIG		{printf("Nao foi possivel abrir o arquivo de configuração.\n\n"); exit(1);}

// Types

// Exportable variables

#ifdef __cplusplus
extern "C" {
#endif
extern int nSinalFrente[MAX_N_SINAL];
extern int nSinalEsquerda[MAX_N_SINAL];
extern int nSinalDireita[MAX_N_SINAL];

// Prototypes
int PortInitialize(int nAdr, int nNum);
int PortTerminate(int nAdr, int nNum);
void ParallelOut(int nAdr, char cByte);
int ParallelIn(int nAdr);
int GetBit(int nValue, int nBit);
void BusyDelay(int nAdr, int uSec);
void IntToBin(char *chrBin, int nInt, int nWidth);
int SerialPortInitialize(char *dev);
void SerialPortTerminate(int fd);

/***********************************************
	PINAGEM DA PORTA SERIAL UTILIZADA NO IR
	=======================================
	
	SINAL	|	PINO (comp)	
	--------+----------------------------
	DTR		|	4		Emissor - Laranja
	GND		|	5		Ground - Marrom
	DCD		|	6		Receptor - Verde
	RTS		|	7		Fonte de tensao - Preto
***********************************************/

void InicializaSerialIR(int fd);
void FinalizaSerialIR(int fd);
void PreparaSinal(char* strDado, int nTempoMs, int nFreq, int *nDado);
void InitInfraRed (void);
void SendIR(int *nSinal, int nRepeticoes);

int IRInitialize (void);
int IRQuit (void);
int IRSendSignal (int *p_nSignal, int nRepetitions);
#ifdef __cplusplus
}
#endif

#endif

