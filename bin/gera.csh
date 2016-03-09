#!/bin/bash
echo "#NOME		ENTRADA		SAIDA		CONNECT		ACERTO(%)	TIME(s)		CORRELATE(%)	TIME(s)	ANN_BS(%)" > $1.dat
tail -n1 `find -name leiame* | grep $1` > tmp
grep -v "==" tmp >> $1.dat
rm tmp
