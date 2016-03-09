Apos ter compilado o inflection_point

Para marcar os pontos
* Manualmente
  - Executar o c�digo (sem passar como argumento o nome de um arquivo cml). Ex.: 
  
  % cd MAE/examples/inflection_point
  % ./inflection_point
  
  - A marcacao ee feita na janela inflection_point.
  - Para marcar um ponto de m�ximo, clique 1 vez com o bot�o esquerdo do mouse. Para marcar um ponto de m�nimo, clique 2 vezes com o bot�o esquerdo do mouse.
  - Para desmarcar um ponto de m�ximo, clique 2 vezes com o bot�o esquerdo do mouse. Para desmarcar um ponto de m�nimo, clique 1 vez com o bot�o esquerdo do mouse.
  - Para visualizar o pr�ximo dia, clique com o bot�o direito do mouse no lado direito da janela (fora do gr�fico) inflection_point. Para visualizar o dia anterio, clique com o bot�o direito do mouse no lado esquerdo da janela (fora do gr�fico) inflection_point.
* Automaticamente
  - Executar sem o cml (% ./inflection_point) ou com o arquivo cml (% ./inflection_point *.cml)
  - A marca�ao autom�tica � feita toda vez que o diret�rio /MAX_MIN/ estiver vazio, ou seja, o c�digo procura dentro desse diretorio por um arquivo contendo as marcac�es dos pontos de m�ximo e de m�nimo; se n�o encontrar, o c�digo marca esses pontos automaticamente.

Para treinar e validar
  * Executar o c�digo passando como argumento o nome do arquivo cml de validac�o. Ex.: 
  
  % cd MAE/examples/inflection_point
  % ./inflection_point inflection_point_validation.cml
   
Para executar o teste de sanidade
  * Executar o c�digo com o arquivo cml de teste de sanidade. Ex.: 
    
  % cd MAE/examples/inflection_point
  % ./inflection_point inflection_point_sanity_test.cml

Para treinar e testar
  * Executar o c�digo com o arquivo cml de teste. Ex.: 
  
  % cd MAE/examples/inflection_point
  % ./inflection_point inflection_point.cml
   
Para executar v�rias configurac�es (por exemplo, variando n�mero de neur�nios e sinapses por neur�nio) do c�digo em paralelo no cluster:
  - Executar os scripts abaixo para criar um diret�rio para cada configurac�o diferente, e compilar e executar o c�digo em cada diret�rio:

  % cd MAE/examples/inflection_point_validation
  % cp_dirs.bat
  % make_dirs.bat
  % run_dirs.bat

  - Depois que os experimentos no cluster finalizarem:

  % result_dirs.bat
  
  - Para apagar os v�rios diret�rios gerados por cp_dirs.bat:
  
  % rm_dirs.bat
  
  
