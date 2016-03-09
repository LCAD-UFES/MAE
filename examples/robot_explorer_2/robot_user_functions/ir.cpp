#include  "ir.hpp"
#include "common.hpp"

// Global variables
TModuleStatus g_eIRStatus = UNINITIALIZED;

int nSinalFrente[MAX_N_SINAL];
int nSinalEsquerda[MAX_N_SINAL];
int nSinalDireita[MAX_N_SINAL];

// ***** Variaveis para InfraRed *****
int fd;	// File descriptor
int status;
int nPin4_SET, nPin4_RESET;
float fTempo, fTempoDelay;
int nIntervalo;

// ************************************************************************
// Funcao - PortlInitialize : Inicializa a permissao para acesso a porta
//
// Entradas: nAdr - Endereco da porta.
//           nNum - Numero de enderecos a partir de nAdr cuja permissao
//                   de acesso sera finalizada. 1 <= nNum <= 3.
//
// Saida:0 se OK, -1 caso contrario.
// ************************************************************************
int PortInitialize(int nAdr, int nNum)
{
	if ((nNum < 1) || (nNum > 3))
		return (-1);

	return (ioperm(nAdr, nNum, 1) == 0 ? 0 : -1);
}

// ************************************************************************
// Funcao - PortTerminate : Finaliza a permissao para acesso a porta.
//
// Entradas: nAdr - Endereco da porta.
//			 nNum - Numero de enderecos a partir de nAdr cuja permissao
//					de acesso sera finalizada. 1 <= nNum <= 3.
//
// Saida: 0 se OK, -1 caso contrario.
// ************************************************************************
int PortTerminate(int nAdr, int nNum)
{
	if ((nNum < 1) || (nNum > 3))
		return (-1);

	return (ioperm(nAdr, nNum, 0) == 0 ? 0 : -1);
}

// ************************************************************************
// Funcao - ParallelOut : Envia um byte para a porta paralela
//
// Entradas: nAdr - Endereco da porta paralela
//			 cByte - Byte que sera enviado para a porta paralela
//
// Saida: Nenhuma.
// ************************************************************************
void ParallelOut(int nAdr, char cByte)
{
	outb(cByte, nAdr);
}

// ************************************************************************
// Funcao - ParallelTerm : Le um byte da porta paralela
//
// Entradas: nAdr - Endereco da porta paralela
//
// Saida: int - Byte lido da porta paralela cujo endereco e nAdr.
// ************************************************************************
int ParallelIn(int nAdr)
{
	return (inb(nAdr));
}

// ************************************************************************
// Funcao - GetBit: Retorna o bit numero nBit de nValue.
//                  Ex: GetBit(11, 3) -> 1
//                      GetBit(11, 2) -> 0
//                      GetBit(11, 1) -> 1
//                      GetBit(11, 0) -> 1
//
// Entradas: nValue: Valor que sera verificado o bit
// 	     nBit: Indice do bit de nValue
//
// Saida: int - Bit de indice nBit de nValue.
// ************************************************************************
int GetBit(int nValue, int nBit)
{
	nBit = nBit % 32;

	return ((nValue >> nBit) % 2);
}

// ************************************************************************
// Funcao - BusyDelay - Realiza um delay (ocupado) de uSec microsegundos.
//						OBS: A porta nAdr deve ter permiss�o previa de
//						leitura.
//
// Entradas: nAdr - Endereco da porta que sera utilizada para gerar o delay
//			 uSec - Microsegundos do delay
//
// Saida: Nenhuma
// ************************************************************************
void BusyDelay(int nAdr, int uSec)
{
	while (uSec--)
		inb(nAdr);
}

// ************************************************************************
// Funcao - IntToBin - Converte um valor inteiro para uma string contendo
//					   a representa��o bin�ria deste inteiro.
//
// Entradas: chrBin - String em que sera retornado a representacao binaria
//					  de nInt
//			 nInt - Inteiro que sera convertido
//			 nWidth - Tamnho, em bits, da conversao
//
// Saida: Nenhuma
// ************************************************************************
void IntToBin(char *chrBin, int nInt, int nWidth)
{
	int nBitTest;
	int i;

	nBitTest = (1 << (nWidth - 1));
	strcpy(chrBin,"");

	for (i = nWidth; i > 0; i--)
	{
		if (nBitTest & nInt)
			strcat(chrBin,"1");
		else
			strcat(chrBin,"0");

		nBitTest >>= 1;
	}
}

// ************************************************************************
// Funcao - SerialPortlInitialize : Inicializa a porta serial.
//
// Entradas: dev: Dispositivo da porta serial. Pode ser uma das constantes:
//					- DEV_SERIAL_PORT_1
//					- DEV_SERIAL_PORT_2
//
// Saida: int: File Descriptor da porta serial. Caso seja < 0, ocorreu um
//			   erro.
// ************************************************************************
int SerialPortInitialize(char *dev)
{
	int fd;

	// Oppening the serial port as a file
	fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd != -1)
		fcntl(fd, F_SETFL, 0);

	return fd;
}

// ************************************************************************
// Funcao - SerialPortlTerminate : Fecha a porta serial.
//
// Entradas: fd: File Descriptor da porta serial.
//
// Saida: Nenhuma
// ************************************************************************
void SerialPortTerminate(int fd)
{
	close(fd);
}

void InicializaSerialIR(int fd)
{
	int status;

	// Le o status da serial
	ioctl(fd, TIOCMGET, &status);
	status &= ~PIN4_SERIAL;		// Reseta o pino do emissor
	status |= PIN7_SERIAL;		// Seta o pino de alimentacao
	ioctl(fd, TIOCMSET, &status);
}

void FinalizaSerialIR(int fd)
{
	int status;

	// Le o status da serial
	ioctl(fd, TIOCMGET, &status);
	status &= ~PIN7_SERIAL;		// Reseta o pino do emissor
	ioctl(fd, TIOCMSET, &status);
}

// =============================================================================
// FUNCOES PARA A MOVIMENTACAO DO ROBO VIA INFRA-VERMELHO
// =============================================================================


// ----------------------------------------------------
// PreparaSinal - Calcula o tamanho dos loops de cada
//				  parte do sinal
//
// Entrada: strDado - Sinal
//			nTempoMs - Tempo em milesegundos do sinal
//			nFreq - Frequencia da portadora
//			nDado - Saida. Tamanho dos loops
//
// Saida: Nenhuma.
// ----------------------------------------------------
void PreparaSinal(char* strDado, int nTempoMs, int nFreq, int *nDado)
{
	int i, nPosIni, nPosFim;
	int nIndice;
	float fAux;

	i = 0;
	nIndice = 0;
	while (i < (int) strlen(strDado))
	{
		// Marca a posicao inicial
		nPosIni = i;

		if (strDado[i] == CHAR_SIG_POS)
		{
			// Indica que o intervalo �positivo
			nDado[nIndice++] = 1;

			// Percorre o sinal enquanto for positivo
			for (; strDado[i] == CHAR_SIG_POS; i++);
		}
		else
		{
			// Indica que o intervalo �negativo
			nDado[nIndice++] = 0;

			// Percorre o sinal enquanto for negativo
			for (; strDado[i] == CHAR_SIG_NEG; i++);
		}

		// Marca a posicao final
		nPosFim = i;

		// Calcula o "numero mágico" :)
		fAux = (float)nFreq * nTempoMs * (nPosFim - nPosIni) / strlen(strDado) / 1000.0;
		nDado[nIndice++] = (int)fAux;
	}

	nDado[nIndice] = -1;
}

// ----------------------------------------------------
// InitInfraRed - Inicializao para utilizar o modulo
//				  Infra-Vermelho
//
// Entrada: Nenhuma
//
// Saida: Nenhuma.
// ----------------------------------------------------
void InitInfraRed (void)
{
	FILE *arq;
	char strSinal[N_MAX_AMOSTRAS];
	int nTempoMs;

	// Inicializa a porta paralela (utilizada para fornecer o delay)
	if (PortInitialize(LPT1_BASE, 3))
		ERRO_ACESSO_PARALELA

	// Inicializa a porta serial
	fd = SerialPortInitialize(DEV_SERIAL_PORT_1);
	if (fd < 0)
		ERRO_ACESSO_SERIAL

	// Inicializa a serial para o IR (Seta o pino de alimentação e Reseta o pino do emissor)
	InicializaSerialIR(fd);

	// Inicializa as constantes de SET e RESET do pino 4 da porta serial
	ioctl(fd, TIOCMGET, &status);
	nPin4_RESET = status & ~PIN4_SERIAL;
	nPin4_SET = status | PIN4_SERIAL;

	// Le o parametro de configuração do delay
	arq = fopen(ARQ_CONFIG,"r");
	if (!arq)
		ERRO_ARQUIVO_CONFIG
	fscanf(arq,"%f",&fTempoDelay);	// A primeira leitura é descartada (config de envio)
	fscanf(arq,"%f",&fTempoDelay);
	fclose(arq);

	// Calculando os parametros para o BusyDelay
	nIntervalo = (int)((float) 1000000.0 / fTempoDelay / FREQ_PORTADORA / 2.0);
	printf("fTempoDelay: %.3fus - nIntervalo: %d\n",fTempoDelay,nIntervalo);

	// Carrega o arquivo com o sinal para ir para frente
	arq = fopen(ARQ_SINAL_FRENTE,"r");
	if (!arq)
		ERRO_ARQ_IR(ARQ_SINAL_FRENTE)
	fscanf(arq,"%d",&nTempoMs);
	fscanf(arq,"%s",strSinal);
	fclose(arq);
	PreparaSinal(strSinal, nTempoMs, FREQ_PORTADORA, nSinalFrente);
	printf("Arquivo '%s' carregado com sucesso.\n",ARQ_SINAL_FRENTE);

	// Carrega o arquivo com o sinal para ir para esquerda
	arq = fopen(ARQ_SINAL_ESQUERDA,"r");
	if (!arq)
		ERRO_ARQ_IR(ARQ_SINAL_ESQUERDA)
	fscanf(arq,"%d",&nTempoMs);
	fscanf(arq,"%s",strSinal);
	fclose(arq);
	PreparaSinal(strSinal, nTempoMs, FREQ_PORTADORA, nSinalEsquerda);
	printf("Arquivo '%s' carregado com sucesso.\n",ARQ_SINAL_ESQUERDA);

	// Carrega o arquivo com o sinal para ir para direita
	arq = fopen(ARQ_SINAL_DIREITA,"r");
	if (!arq)
		ERRO_ARQ_IR(ARQ_SINAL_DIREITA)
	fscanf(arq,"%d",&nTempoMs);
	fscanf(arq,"%s",strSinal);
	fclose(arq);
	PreparaSinal(strSinal, nTempoMs, FREQ_PORTADORA, nSinalDireita);
	printf("Arquivo '%s' carregado com sucesso.\n",ARQ_SINAL_DIREITA);
}

// ----------------------------------------------------
// SendIR - Envia um sinal Infra-Vermelho
//
// Entrada: nSinal - Sinal Infra-Vermelho
//			nRepeticoes - Quantidade de repetições
//
// Saida: Nenhuma.
// ----------------------------------------------------
void SendIR (int *nSinal, int nRepeticoes)
{
	int i, j;
	int nIndice;

	// Aumenta a prioridade do processo
	nice(-20);

	// Envia o sinal
	for (j = 0; j < nRepeticoes; j++)
	{
		nIndice = 0;
		while (nSinal[nIndice] != -1)
		{
			if (nSinal[nIndice] == 1)
			{
				for (i = 0; i < nSinal[nIndice + 1]; i++)
				{
					ioctl(fd, TIOCMSET, &nPin4_SET);
					BusyDelay(LPT1_BASE + 1, nIntervalo);
					ioctl(fd, TIOCMSET, &nPin4_RESET);
					BusyDelay(LPT1_BASE + 1, nIntervalo);
				}
			}
			else
			{
				for (i = 0; i < nSinal[nIndice + 1]; i++)
				{
					ioctl(fd, TIOCMSET, &nPin4_RESET);
					BusyDelay(LPT1_BASE + 1, nIntervalo);
					ioctl(fd, TIOCMSET, &nPin4_RESET);
					BusyDelay(LPT1_BASE + 1, nIntervalo);
				}
			}
			nIndice += 2;
		}
	}

	// Volta a prioridade do processo para normal
	nice(20);
}



// ----------------------------------------------------
// IRInitialize - initializes the IR module
//
// Inputs: none  
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------

int IRInitialize (void)
{
	if (g_eIRStatus == INITIALIZED)
                return (-1);

	InitInfraRed ();

	g_eIRStatus = INITIALIZED;
	
	return (0);
}



// ----------------------------------------------------
// IRQuit - exits the IR module
//
// Inputs: none  
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------

int IRQuit (void)
{
	if (g_eIRStatus == UNINITIALIZED)
                return (-1);

	g_eIRStatus = UNINITIALIZED;

	return (0);
}



// ----------------------------------------------------
// IRSendSignal - send a signal
//
// Inputs: p_nSignal - the signal
//	   nRepetitions - the repetitions  
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------

int IRSendSignal (int *p_nSignal, int nRepetitions)
{
	if (g_eIRStatus == UNINITIALIZED)
                return (-1);
		
	SendIR (p_nSignal, nRepetitions);

	return (0);
}
