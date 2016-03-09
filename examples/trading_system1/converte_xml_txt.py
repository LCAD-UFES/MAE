import sys
import os
from xml.etree.ElementTree import ElementTree


# Cria um dicionario com todos os hora-minutos do dia (das 10:00 aas 16:59). Cada elemento do dicionario contera um preco e um volume.
def init_cotacoes(hora_inicio_pregao, hora_fim_pregao):
    cotacoes = {}
    # hora inicio pregao (fora do horario de verao) = 10
    # hora fim pregao (fora do horario de verao) = 17
    for h in range(hora_inicio_pregao, hora_fim_pregao):
       for m in range(0,60):
           if h != hora_fim_pregao:
                if m < 10: 
                    cotacoes[str(h)+":0"+str(m)] = [-1.0, 0]
                else:
                    cotacoes[str(h)+":"+str(m)] = [-1.0, 0]                         
           else: 
               cotacoes[str(h)+":00"] = [-1.0, 0]
               break
    return cotacoes


def converte(arquivo_xml, hora_inicio_pregao, hora_fim_pregao):
    if os.path.exists(arquivo_xml):
        tree = ElementTree()
        tree.parse(arquivo_xml)     # Cria uma arvore com o conteudo do arquivo xml
        root = tree.getroot()       # Pega a raiz da arvore
        quotes = tree.findall("quote")  # Acha todos os quotes dentro da arvore criada
        
        # Cria um dicionario com todos os hora-minutos do dia (das 10:00 aas 16:59)
        cotacoes = init_cotacoes(hora_inicio_pregao, hora_fim_pregao)
        # Le a array de quotes em ordem reversa
        for quote in reversed(quotes):
#            data = quote.attrib["timestamp"]
            hora_minuto = quote.attrib["timestamp"][(len(quote.attrib["timestamp"])-5):(len(quote.attrib["timestamp"]))]      
            preco = eval(quote.find("price").text)
            volume = eval(quote.find("volume").text)
            try:
                # Em alguns minutos nao ha contacoes: a linha abaixo gera um erro que ee descartado no except
                temp = cotacoes[hora_minuto]
                cotacoes[hora_minuto] = [preco, volume]
                temp = cotacoes[hora_minuto]
                #print hora_minuto, preco, temp
            except:
                pass
        
        # Conta quantas cotacoes estao faltando no inicio do dia
        num_faltando_iniciais = 0
        for hora_minuto in sorted(cotacoes.keys()):
            if cotacoes[hora_minuto][0] == -1.0:
                num_faltando_iniciais = num_faltando_iniciais + 1
            else:
                primeiro_preco = cotacoes[hora_minuto][0]
                break

        # Preenche as cotacoes faltando com a ultima cotacao valida
        i  = 0
        volume_anterior = 0
        for hora_minuto in sorted(cotacoes.keys()):
            if i < num_faltando_iniciais:
                cotacoes[hora_minuto][0] = primeiro_preco
                cotacoes[hora_minuto][1] = 0
                i = i + 1
            
            if cotacoes[hora_minuto][0] == -1.0:
                cotacoes[hora_minuto][0] = preco_anterior
                cotacoes[hora_minuto][1] = volume_anterior
                
            print hora_minuto, cotacoes[hora_minuto][0], cotacoes[hora_minuto][1] - volume_anterior

            preco_anterior = cotacoes[hora_minuto][0]
            volume_anterior = cotacoes[hora_minuto][1]


if __name__ == '__main__':

    if len(sys.argv) != 4:
        print "Numero de paramentros errado"
	print "  Uso: python converte_xml_txt.py <arquivo.xml> <hora inicio pregao, ex.: 10> <hora inicio pregao, ex.: 17>"
	sys.exit(1)
    
    try:
        converte (sys.argv[1], int(sys.argv[2]), int(sys.argv[3]))
    except:
        print "Erro em ", sys.argv[1], sys.argv[2], sys.argv[3]
