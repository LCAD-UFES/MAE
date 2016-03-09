/* ********************************************************
* PROJETO: Trading System
*	
* ARQUIVO: readme.txt		
*
* DATA: 22/02/2011
*
* REFERENCIA: 
*
* RESUMO 
*	      Descreve resumidamente os arquivos
*	ipc-client1.c / ipc-client2.c / messages.h
*             
*
***********************************************************/

***********************************************************************************
			 	   GERAL
***********************************************************************************

O desenvolvimento foi baseado nos programas de teste do ipc.
Foi utilizado algumas estruras desses programas.

O Makefile foi utilizado o do testes com apenas algumas mudancas
que sao a adicao dos arquivos para compilacao com o ipc-client1.c,
 o ipc-client2.c e o messages.h.

O programa foi testado durante toda semana de 14 a 18 de fev, se 
demonstrou estavel nao dando nenhum erro ou qualquer falha de segmen-
tacao.

Fique claro que alguns dados tiveram que ser simulados devido a ausencia
da Rede Neural no sistema. E que foi tudo testado em apenas uma maquina com
virtual box em Fedora 14 com uma pasta compartilhada com o Windows XP. O programa
utilizado para a criacao e manipulacao dos arquivos no Windows foi feito 
atraves de Visual basic e os dados foram prospectados atraves de um canal DDE.

O Makefile foi feito apartir do Makefile utilizado no ipc/teste.

Obs: Caso as funcoes, estruturas nao estejam alocadas ou declaradas da melhor forma
peço que me avise. Para que sejam feitos os ajustes.

***********************************************************************************
				MESSAGES.h
***********************************************************************************

Desenvolvido com as estruturas de dados e as funções utilizadas
pelos modulos dos clientes.

Contem tambem o caminho do diretorio compartilhado via virtual
box.

As funçoes contidas nesse modulo variam de manipulação de arqui-
vos, alocacao de estruturas, liberacao da memoria da estrutura
(free) e funcoes de montagem das estruturas que sera enviadas
com os devidos dados.

***********************************************************************************
				IPC-CLIENT1.c
***********************************************************************************

Desenvolvido para ficar na maquina onde estara localizada a 
Rede Neural esta subscrita as cotacoes, controle e ordens 
executadas.

O recebimento das mensagens foi tratado e esta de acordo. Com os
testes realizados percebeu-se que os dados estao vindo da forma 
correta.

Tera que ser ajustada a medida que a 'casca' entre a Rede Neural
e o IPC estiver desenvolvido pois os testes realizado foi apenas
uma simulacao gerada por um txt.

***********************************************************************************
				IPC-CLIENT2.c
***********************************************************************************

Responsavel pela publicacao das cotacoes, ordem executadas e o
controle.

Foi adicionado tambem um modulo de alarme linkado a plataforma de geracao
de cotas em VB que caso nao esteja funcionando da forma correta gera um 
txt reportando - para que algo seja feito.

O sistema de handshaking foi feito apartir de uma funcao ja existente 
na biblioteca do ipc.h que verifica a conexao entre um cliente e outro
dentro do mesmo timer da publicao das cotacoes e da publicacao das ordens
executadas.

Como decidido foi utilizado um timer de 1 em 1 segundo que verifica as 
cotacoes caso exista txt com os dados publica. 
E no mesmo timer verifica se existe ordem executada caso exista publica. 
Ambas publicacoes sao feitas ao IPC CLIENT1, que contem a ligacao com a 
Rede Neural.

Subscrito a publicao de ordens do IPC CLIENT 1, IPC CLIENT 2 gera um txt que sera
consultado pelo sistema em VB e rotiado para bolsa. Apos rotiado se a ordem tiver
sofrido mudanca de status sera criado um txt com as informacoes de quantidade executada preco e etc.
Gerado esse txt ele sera lido pelo IPC CLIENT 2 e publicado ao IPC CLIENT 1 informando assim 
ao sistema da Rede Neural a atual situcao da ordem.

***********************************************************************************
				OBSERVACOES
***********************************************************************************

Alguns comentarios ao longo do codigo foram feitos para debugar o codigo
e para simular alguns dados que deveriam vir da Rede Neural.

Qualquer duvida
andre.costa@peixepiloto.com / acostaleal@gmail.com - 3225 3059 / 9981 8153


***********************************************************************************
***********************************************************************************
***********************************************************************************

Para rodar a nova versao que substitui IPC CLIENT 1 por ipc_trading_system e
IPC CLIENT 2 por quotes_server_order_router:

- Abra tres terminais

- No primeiro, digite
/usr/local/ipc/bin/Linux-2.6/central

- No segundo, digite
/home/alberto/MAE/examples/trading_system_peixe_piloto_week/IPC2/quotes_server_order_router

- No terceiro, digite
/home/alberto/MAE/examples/trading_system_peixe_piloto_week/IPC2/ipc_trading_system

