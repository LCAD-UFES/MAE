
>> PROGRAMAS:

- parser: dado um arquivo de dados gera as probabilidades para uso da formula de bayes.
- use_statistics: dado um arquivo de dados e um arquivo de probabilidades, gera a probabilidade de cada linha conter uma placa. a saida do programa eh uma probabilidade por linha do arquivo de dados.

>> COMO COMPILAR E RODAR OS PROGRAMAS:

- compilar: veja a opcao "make all" no makefile
- rodar: veja a opcao "make run" no makefile


>> FORMATO DO ARQUIVO DE SAIDA DO PROGRAMA "PARSER":

linha 1: altura e largura da imagem
linha 2: numero de bin na largura e altura da imagem, numero de bins no zoom, e numero de bins na confidence
linha 3: phit // = P(D)
linha 4: pmiss // = P(!D)
linhas (...): P(X[i] | D)
linhas (...): P(X[i] | !D)
linhas (...): P(Y[i] | D)
linhas (...): P(Y[i] | !D)
linhas (...): P(Z[i] | D) 	// o primeiro numero da linha, logo apos o caracter 'Z' eh o zooom e o segundo eh a probabilidade
linhas (...): P(Z[i] | !D)	// idem
linhas (...): P(C[i] | D)	// o primeiro e segundo numeros da linha, logo apos o caracter 'C' sao os limites daquele bin de confianca e o terceiro valor eh a probabilidade
linhas (...): P(C[i] | !D)	// idem
