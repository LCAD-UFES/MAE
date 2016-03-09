import os

for nome in os.listdir('.'):
 nome_split = nome.split(".")
 novo_nome = nome_split[0] + '.' + "jpg" + '.' + "pnm"
 var = "cp " + nome + ' ' + novo_nome
 os.system(var)
print 'OK'
