Apos ter compilado o inflection_point

Para marcar os pontos
* Manualmente
  - Executar o código (sem passar como argumento o nome de um arquivo cml). Ex.: 
  
  % cd MAE/examples/inflection_point
  % ./inflection_point
  
  - A marcacao ee feita na janela inflection_point.
  - Para marcar um ponto de máximo, clique 1 vez com o botão esquerdo do mouse. Para marcar um ponto de mínimo, clique 2 vezes com o botão esquerdo do mouse.
  - Para desmarcar um ponto de máximo, clique 2 vezes com o botão esquerdo do mouse. Para desmarcar um ponto de mínimo, clique 1 vez com o botão esquerdo do mouse.
  - Para visualizar o próximo dia, clique com o botão direito do mouse no lado direito da janela (fora do gráfico) inflection_point. Para visualizar o dia anterio, clique com o botão direito do mouse no lado esquerdo da janela (fora do gráfico) inflection_point.
* Automaticamente
  - Executar sem o cml (% ./inflection_point) ou com o arquivo cml (% ./inflection_point *.cml)
  - A marcaçao automática é feita toda vez que o diretório /MAX_MIN/ estiver vazio, ou seja, o código procura dentro desse diretorio por um arquivo contendo as marcacões dos pontos de máximo e de mínimo; se não encontrar, o código marca esses pontos automaticamente.

Para treinar e validar
  * Executar o código passando como argumento o nome do arquivo cml de validacão. Ex.: 
  
  % cd MAE/examples/inflection_point
  % ./inflection_point inflection_point_validation.cml
   
Para executar o teste de sanidade
  * Executar o código com o arquivo cml de teste de sanidade. Ex.: 
    
  % cd MAE/examples/inflection_point
  % ./inflection_point inflection_point_sanity_test.cml

Para treinar e testar
  * Executar o código com o arquivo cml de teste. Ex.: 
  
  % cd MAE/examples/inflection_point
  % ./inflection_point inflection_point.cml
   
Para executar várias configuracões (por exemplo, variando número de neurônios e sinapses por neurônio) do código em paralelo no cluster:
  - Executar os scripts abaixo para criar um diretório para cada configuracão diferente, e compilar e executar o código em cada diretório:

  % cd MAE/examples/inflection_point_validation
  % cp_dirs.bat
  % make_dirs.bat
  % run_dirs.bat

  - Depois que os experimentos no cluster finalizarem:

  % result_dirs.bat
  
  - Para apagar os vários diretórios gerados por cp_dirs.bat:
  
  % rm_dirs.bat
  
  
