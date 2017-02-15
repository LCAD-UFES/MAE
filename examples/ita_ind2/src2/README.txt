Este codigo ita_ind2/src2 esta de acordo com o Relatorio 2 do estudo dirigido.

Ele usa maquina de estados para treinar e testar, ou seja, o retorno de treino considera a estrategia de investimento, assim como na hora de "testar" ou fazer a operacao com o valor predito.

O stop gain eh igual a 1 x custo e o stop loss eh igual a -1 x custo.

Considera R$ 125000,00 de capital e opera os 4 indices alavancado. Quantidades operadas sao constantes no codigo ita_ind_user_functions.c.

A entrada da rede eh a variacao em pontos mas considerando uma "ancora" no valor mais antigo do frame de entrada.

Com esse mesmo codigo da pra reproduzir o que esta no Relatorio 1 do estudo dirigido, ou seja, treinar sem considerar a estrategia de investimento e testar considerando.
Porem isso nao faz muito sentido!!!
