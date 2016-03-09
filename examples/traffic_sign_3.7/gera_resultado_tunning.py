# Script que gera o resultado para o tunning.

arq_output = open("resultadoTunning.csv", 'w')

# Lendo os arquivos
hit = open("resultadoFinalHit.csv")
pwd = open("diretorio.txt")

lines_hit = hit.readlines()
lines_pwd = pwd.readlines()

result_hit = float(lines_hit[0])

for i in lines_pwd:
	ls_pwd = i.split("_")
	nscale = (ls_pwd[3])
	synapses = (ls_pwd[4])
	gaussrad = (ls_pwd[5])
	logfactor = (ls_pwd[6])
	voteparam = (float(ls_pwd[7]))

output = ""
output = output + str(nscale) + ";" + str(synapses) + ";" + str(gaussrad)+ ";" + str(logfactor)+ ";" + str(voteparam)+ ";" + str(result_hit)

hit.close()
pwd.close()
arq_output.write(output)
arq_output.close()


