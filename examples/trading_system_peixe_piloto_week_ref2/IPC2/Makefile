# Makefile trading_system + ipc

# Quando IPC_FLAGS inclui USE_IPC, o trading_system usa o IPC para receber quotes, e enviar ordens e receber confirma��o de ordens.
# Quando IPC_FLAGS inclui IPC_TEST (deve ser incluido em IPC_FLAGS junto com o USE_IPC), o quotes_server_order_router 
# reenvia as ordens recebidas do trading_system para simular um roteamento de ordens perfeito.
# Quando IPC_FLAGS inclui IPC_ON_LINE (deve ser incluido em IPC_FLAGS junto com o USE_IPC), o quotes_server_order_router 
# faz o roteamento normal de ordens, i.e., junto com o roteador on-line Peixe Piloto.
IPC_FLAGS =
#IPC_FLAGS = -DUSE_IPC -DIPC_TEST
#IPC_FLAGS = -DUSE_IPC -DIPC_ON_LINE
CFLAGS = -Wall -Wnested-externs -g
INC_DIRS = -I. -I/usr/local/ipc/include -I/usr/include/bsd  -I/usr/local/ipc/src -I${MAEHOME}/include
CFLAGS2 = -DREDHAT_52 -DREDHAT_6 -DREDHAT_71 -DTEST_TRADING_SYSTEM ${IPC_FLAGS}
LIB_DIRS = -L/usr/local/ipc/lib/Linux-3.0 -L/usr/local/ipc/lib/Linux-2.6 -L/lib -L$(MAEHOME)/lib
ifeq (USE_IPC, $(findstring USE_IPC, $(IPC_FLAGS)))
LIBS = -lipc  -lc -lpthread -lm -lforms
else
LIBS = -lc -lpthread -lm -lforms
endif

ifeq (USE_IPC, $(findstring USE_IPC, $(IPC_FLAGS)))
build: quotes_server_order_router trading_system state_view trading_system_control
	cp state.bin state.bin.old
	cp log.txt log.txt.old
	rm -f state.bin GRAFICOS/* GRAFICO_ONLINE/* log.txt /mnt/sharedocs/quotes/* /mnt/sharedocs/order/* /mnt/sharedocs/orderExecution/*
	rm -f /mnt/sharedocs/multi_order/* /mnt/sharedocs/multi_orderExecution/*

trading_system: trading_system.o trading_system_state_machine.o trade.o data_set_management.o trading_system_globals.o timeutil.o ipc_trading_system.o messages.o ipc_globals.o
	gcc ${CFLAGS} ${LIB_DIRS} -o trading_system trading_system.o trading_system_state_machine.o trade.o data_set_management.o trading_system_globals.o timeutil.o ipc_trading_system.o messages.o ipc_globals.o ${LIBS}

state_view: state_view.o trade.o data_set_management.o trading_system_globals.o timeutil.o messages.o ipc_globals.o trading_system_state_machine.o 
	gcc ${CFLAGS} ${LIB_DIRS} -o state_view state_view.o trade.o data_set_management.o trading_system_globals.o timeutil.o messages.o ipc_globals.o trading_system_state_machine.o ${LIBS}

trading_system_control: trading_system_control_main_true.o trading_system_control.o trading_system_control_cb_true.o trading_system_state_machine.o trade.o data_set_management.o trading_system_globals.o timeutil.o 
	gcc ${CFLAGS} ${LIB_DIRS} -o trading_system_control trading_system_control_main_true.o trading_system_control.o trading_system_control_cb_true.o trading_system_state_machine.o trade.o data_set_management.o trading_system_globals.o timeutil.o messages.o ipc_globals.o ${LIBS}
else
build: trading_system state_view
	cp state.bin state.bin.old
	cp log.txt log.txt.old
	rm -f state.bin GRAFICOS/* GRAFICO_ONLINE/* log.txt

trading_system: trading_system.o trading_system_state_machine.o trade.o data_set_management.o trading_system_globals.o timeutil.o
	gcc ${CFLAGS} ${LIB_DIRS} -o trading_system trading_system.o trading_system_state_machine.o trade.o data_set_management.o trading_system_globals.o timeutil.o ${LIBS}

state_view: state_view.o trade.o data_set_management.o trading_system_globals.o timeutil.o trading_system_state_machine.o 
	gcc ${CFLAGS} ${LIB_DIRS} -o state_view state_view.o trade.o data_set_management.o trading_system_globals.o timeutil.o trading_system_state_machine.o ${LIBS}
endif


trading_system.o: trading_system.c trading_system.h timeutil.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c trading_system.c 

state_view.o: state_view.c trading_system.h 
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c state_view.c 

trading_system_state_machine.o: trading_system_state_machine.c trading_system.h timeutil.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c trading_system_state_machine.c 

trade.o: trade.c trading_system.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c trade.c 

data_set_management.o: data_set_management.c trading_system.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c data_set_management.c 

trading_system_globals.o: trading_system_globals.c trading_system.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c trading_system_globals.c 

timeutil.o: timeutil.c timeutil.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c timeutil.c 

ifeq (USE_IPC, $(findstring USE_IPC, $(IPC_FLAGS)))
ipc_trading_system: ipc_trading_system.o messages.o ipc_globals.o trading_system_globals.o
	gcc ${CFLAGS} ${LIB_DIRS} -o ipc_trading_system ipc_trading_system.o messages.o ipc_globals.o trading_system_globals.o ${LIBS}

ipc_trading_system.o: ipc_trading_system.c trading_system.h messages.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c ipc_trading_system.c 

trading_system_control_main_true.o: trading_system_control_main_true.c trading_system.h messages.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c trading_system_control_main_true.c 

trading_system_control.o: trading_system_control.c trading_system.h messages.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c trading_system_control.c 

trading_system_control_cb_true.o: trading_system_control_cb_true.c trading_system.h messages.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c trading_system_control_cb_true.c 
endif

quotes_server_order_router: quotes_server_order_router.o messages.o ipc_globals.o
	gcc ${CFLAGS} ${LIB_DIRS} -o quotes_server_order_router quotes_server_order_router.o messages.o ipc_globals.o ${LIBS}

quotes_server_order_router.o: quotes_server_order_router.c messages.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c quotes_server_order_router.c 

messages.o: messages.c messages.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c messages.c 

ipc_globals.o: ipc_globals.c messages.h
	gcc ${CFLAGS} ${INC_DIRS} ${CFLAGS2} -c ipc_globals.c 

clean:
	rm -f *.o trading_system_control ipc_trading_system trading_system quotes_server_order_router state_view /mnt/sharedocs/quotes/*
	rm -f state.bin GRAFICOS/* GRAFICO_ONLINE/* log.txt /mnt/sharedocs/quotes/* /mnt/sharedocs/order/* /mnt/sharedocs/orderExecution/*
	rm -f /mnt/sharedocs/multi_order/* /mnt/sharedocs/multi_orderExecution/*
