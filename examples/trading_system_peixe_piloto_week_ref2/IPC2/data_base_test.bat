#!/bin/csh

echo === DADOS INTRADAY ===
foreach STOCK ( `cat ../acoes_de_interesse.txt` )
	echo -n ${STOCK} " "
	tail -n 1 DADOS_PEIXE_PILOTO/COM_VOLUME/${STOCK}*
end

echo === DADOS DIARIOS ===
foreach STOCK ( `cat ../acoes_de_interesse.txt` )
	echo -n ${STOCK} " "
	tail -n 1 DATA/${STOCK}/*.clo
end



