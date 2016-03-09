== Para instalar o IPC no Linux (Fedora) ==
Baixe a versão IPC 3.8.6 de http://www.cs.cmu.edu/~ipc/
Como usuário root:
 mv ipc-3.8.6.tar.gz /usr/local
 cd /usr/local
 tar xzvf ipc-3.8.6.tar.gz
 cd ipc-3.8.6
 gedit GNUmakefile

Altere a linha
 SUBDIRS = etc src lisp java test doc xdrgen
Para
 SUBDIRS = etc src test doc

 gmake install
 cd test
 gmake
 cd /usr/local
 ln -s ipc-3.8.6 ipc
 
== Para testar o IPC ==
Como usuário normal (não root) abra três terminais e, no primeiro, digite:
 /usr/local/ipc/bin/Linux-2.6/central

Você deve ver as mensagens:
____
Task Control Server 3.8.6 (Dec-17-10)
Expecting 1 on port 1381
> 
____

No segundo, digite:
 /usr/local/ipc/test/bin/Linux-2.6/module1

Você deve ver as mensagens:
____
 
  IPC_connect(module1)
NMP IPC Version 3.8.6 (Dec-17-10)
Warning: CENTRALHOST environment variable not set.
Warning: trying to connect to local host cachoeiro.localdomain.
... IPC Connected on port 1381

IPC_defineFormat(T1, {int, {enum : 3}, [double:2,3], double})

IPC_defineFormat(T2, {string, int, <T1:2>, {enum WaitVal, SendVal, ReceiveVal, ListenVal}})

IPC_defineMsg(message1, IPC_VARIABLE_LENGTH, int)

IPC_defineMsg(query1, IPC_VARIABLE_LENGTH, T1)

IPC_subscribe(message2, msg2Handler, module1)

IPC_subscribeFD(0, stdinHnd, module1)

Type 'm' to send message1; Type 'r' to send query1; Type 'q' to quit
_____


No terceiro, digite:
 /usr/local/ipc/test/bin/Linux-2.6/module2

Você deve ver as mensagens:
____
 
IPC_connect(module2)
NMP IPC Version 3.8.6 (Dec-17-10)
Warning: CENTRALHOST environment variable not set.
Warning: trying to connect to local host cachoeiro.localdomain.
... IPC Connected on port 1381

IPC_defineMsg(message2, IPC_VARIABLE_LENGTH, string)

IPC_defineMsg(response1, IPC_VARIABLE_LENGTH, T2)

IPC_subscribe(message1, msg1Handler, module2)

IPC_subscribe(query1, queryHandler, module2)

IPC_subscribeFD(0, stdinHnd, module2)

Type 'q' to quit
_____

A inicialização de module1 e module2 é detectada por central. 
Para terminar os programas digite "q" para module1 e module2 e "quit" 
para central.
 

== Para compilar código que use IPC ==
O texto abaixo é um exemplo de Makefile para compilar código que use IPC. 
Este Makefile produz dois executáveis: ipc-client1, ipc-client2. Estes executáveis
empregam mensagens definidas em messages.h, cujo código associado aparece em 
messages.c. O arquivo global.c possui declarações de variáveis globais. Para compilar
os programas ipc-client1 e ipc-client2 basta digitar:
 make clean
 make


# Makefile para executáveis que usem IPC

CFLAGS = -Wall -Wnested-externs -O2
INC_DIRS = -I. -I/usr/local/ipc/include -I/usr/include/bsd  -I/usr/local/ipc/src
CFLAGS2 = -DREDHAT_52 -DREDHAT_6 -DREDHAT_71

LIB_DIRS = -L/usr/local/ipc/lib/Linux-2.6 -L/lib
LIBS = -lipc  -lc -lpthread

build: ipc-client1 ipc-client2

ipc-client1: ipc-client1.o messages.o global.o
	gcc ${LIB_DIRS} -o ipc-client1 ipc-client1.o messages.o global.o ${LIBS}

ipc-client2: ipc-client2.o messages.o global.o
	gcc ${LIB_DIRS} -o ipc-client2 ipc-client2.o messages.o global.o ${LIBS}

ipc-client1.o: ipc-client1.c messages.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c ipc-client1.c 

ipc-client2.o: ipc-client2.c messages.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c ipc-client2.c 

messages.o: messages.c messages.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c messages.c 

global.o:
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c global.c 

clean:
	rm *.o ipc-client1 ipc-client2
