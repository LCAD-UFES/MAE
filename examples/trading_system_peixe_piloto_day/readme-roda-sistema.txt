== Para colocar o trading system diario para rodar ==

Va para o diretorio MAE/examples/trading_system_peixe_piloto_day:
> cd MAE/examples/trading_system_peixe_piloto_day

- Instale os dados da Enfoque:
> rm DATA  (Note que nao ee rm DATA/   Voce esta apagando o link simbolico)
> cd ../dados
> rm -rf DATA
> tar xzvf dados_fabio.tgz
> cd MAE/examples/trading_system_peixe_piloto_day
> ln -s ../dados/DATA .

- Instale os dados da Peixe Piloto (pegar da pasta shered_docs e colocar na pasta DADOS_PEIXE_PILOTO):
> cd MAE/examples/trading_system_peixe_piloto_day
> rm DADOS_PEIXE_PILOTO  (Note que nao ee rm DADOS_PEIXE_PILOTO/   Voce esta apagando o link simbolico)
> cd ../dados
> rm -rf DADOS_PEIXE_PILOTO
> (Andre vai colocar aqui comandos equivalentes ao tar xzvf dados_fabio.tgz acima)
> cd MAE/examples/trading_system_peixe_piloto_day
> ln -s ../dados/DADOS_PEIXE_PILOTO .

- Paracao de DATA_BUY_SELL
Va para o diretorio cd MAE/examples/trading_system_peixe_piloto_day:
> cd MAE/examples/trading_system_peixe_piloto_day
> make clean
> make -f Makefile.no_interface
> cd IPC2
Edite o Makefile descomentando a linha "IPC_FLAGS = -DBUILD_DAILY_BUY_SELL_FILES". Todas as outras comecando com IPC_FLAGS devem estar comentadas.
> make clean
> make
> cd ..
> ./run_all.sh DATA TT_new  (podem ocorrer varios erros mas nao tem problema)
> ./compute_buy_sell_prices.sh  (este comando demora bastante tempo para terminar)

- Restauracao do trading system ao seu estado de operacao normal:
Va para o diretorio IPC2:
> cd MAE/examples/trading_system_peixe_piloto_day/IPC2
Edite o Makefile descomentando a linha "IPC_FLAGS = -DUSE_IPC -DIPC_ON_LINE". Todas as outras comecando com IPC_FLAGS devem estar comentadas.
> make clean
> make

- Faca as predicoes:
> cd MAE/examples/trading_system_peixe_piloto_day
> make clean
> make -f Makefile.no_interface
> shopt -s extglob
> cp wnn_pred.cml.last_week wnn_pred.cml
> ./run_all.sh DATA TT_new

- Abra um terminal na maquina alberto e digite:
> /usr/local/ipc/bin/Linux-2.6/central -lx

- Na maquina peixepiloto, digite:
> ./quotes_server_order_router

- Rode o trading system:
> IPC2/trading_system TT_new/*

Voce pode interromper e reiniciar o trading system. Uma saida graciosa ee
digitar 'q' e 'enter'. Mas pode digitar crtl+c tambem.

- Observe o log:
Abra um outro terminal e va para a pasta MAE/examples/trading_system_peixe_piloto_week
> cat log.txt
