# Script que concatena os tres resultados da seguinte forma:
#	 Se as tres camadas votaram igual, esta é a classe escolhida;
#	 Se duas votaram iguais, a classe votada pelas duas é a escolhida;
#	 Caso contrario, a classe escolhida é a de maior confianca.

arq_output = open("resultadoFinal_1.csv", 'w')

r = open("/home/car02/MAE/examples/traffic_sign_3.6/traffic_sign_3.6_RED/hit_statistics_minchinton.csv")
g = open("/home/car02/MAE/examples/traffic_sign_3.6/traffic_sign_3.6_GREEN/hit_statistics_minchinton.csv")
b = open("/home/car02/MAE/examples/traffic_sign_3.6/traffic_sign_3.6_BLUE/hit_statistics_minchinton.csv")

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

output = ""
for i in lines_red:
	ls_red = i.split(";")
	arquivo_red.append(ls_red[0])
	classe_red.append(int(ls_red[1]))
	confianca_red.append(ls_red[2])

for i in lines_green:
	ls_green = i.split(";")
	arquivo_green.append(ls_green[0])
	classe_green.append(int(ls_green[1]))
	confianca_green.append(ls_green[2])

for i in lines_blue:
	ls_blue = i.split(";")
	arquivo_blue.append(ls_blue[0])
	classe_blue.append(int(ls_blue[1]))
	confianca_blue.append(ls_blue[2])

for i in range(0, len(classe_red)):
	if ((classe_red[i] == classe_green[i]) and (classe_red[i] == classe_blue[i])):
		print "Todos iguais"
		output = output + str(arquivo_red[i]) + ";" + str(classe_red[i]) + "\n"
	else:
		if (classe_red[i] == classe_green[i]):
			print "RED = GREEN"
			output = output + str(arquivo_red[i]) + ";" + str(classe_red[i]) + "\n"
		else:
			if (classe_red[i] == classe_blue[i]):
				print "RED = BLUE"
				output = output + str(arquivo_red[i]) + ";" + str(classe_red[i]) + "\n"
			else:
				if (classe_green[i] == classe_blue[i]):
					print "GREEN = BLUE"
					output = output + str(arquivo_green[i]) + ";" + str(classe_green[i]) + "\n"
				else:
					if ((confianca_red[i] > confianca_green[i]) and (confianca_red[i] > confianca_blue[i])):
						print "Red maior"
						output = output + str(arquivo_red[i]) + ";" + str(classe_red[i]) + "\n"
					else:
						if ((confianca_green[i] > confianca_red[i]) and (confianca_green[i] > confianca_blue[i])):
							print "Green maior"
							output = output + str(arquivo_green[i]) + ";" + str(classe_green[i]) + "\n"
						else:
							print "Blue maior"
							output = output + str(arquivo_blue[i]) + ";" + str(classe_blue[i]) + "\n"

r.close()
g.close()
b.close()
arq_output.write(output)
arq_output.close()
