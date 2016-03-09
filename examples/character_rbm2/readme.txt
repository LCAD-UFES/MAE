Introdução:

O diretório Autoencoder_Code possui o código fonte em Matlab (http://www.cs.toronto.edu/~hinton/MatlabForSciencePaper.html) utilizado nos experimentos com a base MNIST para o artigo http://www.cs.toronto.edu/~hinton/science.pdf
	Neste diretório há dois scripts principais: "mnistdeepauto.m" e "mnistclassify.m". O primeiro para gerar os resultados de autoencode e o segundo para os resultados
	de classificação apresentados no artigo citado acima. Para mais detalhes sobre essa implementação consulte http://www.cs.toronto.edu/~hinton/absps/science_som.pdf

Os demais arquivos e diretórios são específicos do exemplo character_rbm da MAE. 

1) Para rodar o exemplo primeiro baixe e descompacte a base MNIST usando os comandos abaixo:
	cd /dados
	mkdir MNIST
	cd MNIST
	
	wget http://yann.lecun.com/exdb/mnist/train-images-idx3-ubyte.gz 
	wget http://yann.lecun.com/exdb/mnist/train-labels-idx1-ubyte.gz
	wget http://yann.lecun.com/exdb/mnist/t10k-images-idx3-ubyte.gz
	wget http://yann.lecun.com/exdb/mnist/t10k-labels-idx1-ubyte.gz

	unzip train-images-idx3-ubyte.gz 
	unzip train-labels-idx1-ubyte.gz
	unzip t10k-images-idx3-ubyte.gz
	unzip t10k-labels-idx1-ubyte.gz
	
2) Verifique se o arquivo character.in referencia corretamente o caminho e o nome dos arquivos descompactados.

3) Compile o exemplo e antes de rodar o script character.cml verifique neste se a quantidade de exemplos de treino e teste
	está definida como 60000 exemplos de treino e 10000 de teste.

4) Para rodar basta executar o seguinte comando: ./character character.cml