# Script que gera o recognition rate final a partir do resultado final gerado para envio do site.

arq_output = open("resultadoFinalHit_new.csv", 'w')

# Lendo os arquivos
result = open("resultadoFinal_new.csv")
desc = open("traffic_sign_3.6_BLUE/testing_set_raw/desc_testing_set")

lines_result = result.readlines()
lines_desc = desc.readlines()

arquivo_result = []
classe_result = []

arquivo_desc = []
classe_desc = []

for i in lines_result:
	ls_result = i.split(";")
	arquivo_result.append(ls_result[0])
	classe_result.append(int(ls_result[1]))

for i in lines_desc:
	ls_desc = i.split(" ")
	arquivo_desc.append(ls_desc[0])
	classe_desc.append(int(ls_desc[7]))

output = ""
hit = 0.0
num = 0.0

# Gerando o resultado final
for i in range (0, len(arquivo_result)):
		if (classe_result[i] == classe_desc[i]):
			hit = hit + 1.0;
		num = num + 1.0
		
resultado = float(hit/num)

print float(resultado)

output = str(resultado)

result.close()
desc.close()
arq_output.write(output)
arq_output.close()


