== Para colocar o trading system para rodar ==

Voce esta no diretoiro MAE/examples/trading_system_peixe_piloto_week

- Instale os dados da Enfoque:
> rm -rf DATA
> tar xzvf dados_fabio.tgz

- Instale os dados da Peixe Piloto (pegar da pasta shered_docs e colocar 
na pasta DADOS_PEIXE_PILOTO)
> rm -rf DADOS_PEIXE_PILOTO
> (Andre vai colocar aqui os comandos)

- Copie o diretorio IPC2 para a maquina peixepiloto e, neste diretorio na 
maquina peixe_piloto, digite:
> make clean
> make

- Abra um terminal na maquina alberto e digite:
> /usr/local/ipc/bin/Linux-2.6/central -lx
- Na maquina peixepiloto, digite:
> ./quotes_server_order_router

- Faca as predicoes:
> make clean
> make -f Makefile.no_interface
> shopt -s extglob
> cp wnn_pred.cml.last_week wnn_pred.cml
> ./run_all.sh DATA TT_new

- Rode o trading system:
> IPC2/trading_system TT_new/*

Voce pode interromper e reiniciar o trading system. Uma saida graciosa ee
digitar 'q' e 'enter'. Mas pode digitar crtl+c tambem.

- Observe o log:
Abra um outro terminal e va para a pasta MAE/examples/trading_system_peixe_piloto_week
> cat log.txt
