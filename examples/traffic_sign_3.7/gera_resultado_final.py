# Script que concatena os tres resultados da seguinte forma:
#	 Eh somado todos os votos das tres camadas.
#	 A classe que obteve mais votos neste somatorio eh a escolhida.

arq_output = open("resultadoFinal_new.csv", 'w')


# Lendo os arquivos
r = open("traffic_sign_3.6_RED/hit_statistics_new.csv")
g = open("traffic_sign_3.6_GREEN/hit_statistics_new.csv")
b = open("traffic_sign_3.6_BLUE/hit_statistics_new.csv")

lines_red = r.readlines()
lines_green = g.readlines()
lines_blue = b.readlines()

arquivo_red = []
classe_red = []
confianca_red = []

arquivo_green = []
classe_green = []
confianca_green = []

arquivo_blue = []
classe_blue = []
confianca_blue = []

for i in lines_red:
	ls_red = i.split(";")
	arquivo_red.append(ls_red[0])
	classe_red.append(int(ls_red[1]))
	confianca_red.append(float(ls_red[2]))

for i in lines_green:
	ls_green = i.split(";")
	arquivo_green.append(ls_green[0])
	classe_green.append(int(ls_green[1]))
	confianca_green.append(float(ls_green[2]))

for i in lines_blue:
	ls_blue = i.split(";")
	arquivo_blue.append(ls_blue[0])
	classe_blue.append(int(ls_blue[1]))
	confianca_blue.append(float(ls_blue[2]))


# Gerando um vetor com o nome dos arquivos sem repeticoes
arquivo = []
for i in range (0, len(arquivo_red)/(43*3)):
	j = i*43*3
	arquivo.append(arquivo_red[j])


# Juntando os 3 votos de cada neuronio
classe_red_soma = []
classe_green_soma = []
classe_blue_soma = []

for i in range(0, (len(classe_red)/(43*3))):
	classe_red_aux = []
	classe_green_aux = []
	classe_blue_aux = []

	#inicializando os vetores auxiliares
	for k in range (0, 43):
		classe_red_aux.append(0.0)
		classe_green_aux.append(0.0)
		classe_blue_aux.append(0.0)

	for j in range (0, 43*3):
		# print "iteracao", (i*43*3)+j
		classeR = int(classe_red[(i*43*3)+j])
		classe_red_aux[classeR] = classe_red_aux[classeR] + confianca_red[(i*43*3)+j]
		classeG = int(classe_green[(i*43*3)+j])
		classe_green_aux[classeG] = classe_green_aux[classeG] + confianca_green[(i*43*3)+j]
		classeB = int(classe_blue[(i*43*3)+j])
		classe_blue_aux[classeB] = classe_blue_aux[classeB] + confianca_blue[(i*43*3)+j]

	classe_red_soma.append(classe_red_aux)
	classe_green_soma.append(classe_green_aux)
	classe_blue_soma.append(classe_blue_aux)


# Gerando o resultado final da soma dos votos das 3 camadas
classe_soma = []

for i in range (len(classe_red_soma)):
	classe_soma_aux = []

	#inicializando o vetor auxiliar
	for k in range (0, 43):
		classe_soma_aux.append(0.0)

	for j in range (0, 43):
		classe_soma_aux[j] = classe_red_soma[i][j] + classe_green_soma[i][j] + classe_blue_soma[i][j]
	
	classe_soma.append(classe_soma_aux)


output = ""
# Gerando o resultado final
for i in range (0, len(classe_soma)):
	aux = classe_soma[i][0]
	indice = 0
	for j in range (1, 43):
		if (aux < classe_soma[i][j]):
			aux = classe_soma[i][j]
			indice = j
	output = output + str(arquivo[i]) + ";" + str(indice) + "\n"

r.close()
g.close()
b.close()
arq_output.write(output)
arq_output.close()


