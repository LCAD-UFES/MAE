;
; AutoHotkey Version: 1.x
; Language:       English
; Platform:       Win9x/NT
; Author:         Alberto
;

#NoEnv  ; Recommended for performance and compatibility with future AutoHotkey releases.
SendMode Input  ; Recommended for new scripts due to its superior speed and reliability.

SetWorkingDir, Z:\

; Substitui um caracter no clipboard
; caracter := "x"
; StringReplace, clipboard, clipboard, %caracter%, +, All

; Cria uma janela do Notepad para colocar os valores das ações ou usa uma existente
; Run Notepad
; WinWait Untitled - Notepad
; WinActivate
; WinMaximize	

; envia a hora, o minuto e o segundo do relógio do sistema
; Send %A_Hour%-%A_Min%-%A_Sec%

Loop
{
	UpdateTradingSystemData()
	action := GetActionFromTradingSystem()
	; MsgBox, %action%
	if (action = "BUY")
	{
		BuyAction()
	}
	else if (action = "SELL")
	{
		SellAction()
	}
	else if (action = "CANCELORDER")
	{
		CancelAction()
	}
}


GetActionFromTradingSystem()
{
	GetTradingSystemPage()

	action_file_name := "Z:\action.txt"
	action := GetTextFromFile(action_file_name)
	return action
}


BuyAction()
{
	stock := ""
	price := 0
	quantity := 0
	GetStockPriceAndQuantityFromTradingSystem(stock, price, quantity)
	GetHomeBrokerPage()
	; Clica na aba "Compra"
	Click 1167, 152
	Sleep, 500

	FillInStockQuantityAndPriceAndSendOrder(stock, price, quantity)
}


SellAction()
{
	stock := ""
	price := 0
	quantity := 0
	GetStockPriceAndQuantityFromTradingSystem(stock, price, quantity)
	GetHomeBrokerPage()
	; Clica na aba "Venda"
	Click 1229, 152
	Sleep, 500

	FillInStockQuantityAndPriceAndSendOrder(stock, price, quantity)
}



FillInStockQuantityAndPriceAndSendOrder(stock, price, quantity)
{
	; Clica no campo usado para inserir o nome da acao
	Click 1115, 156
	Sleep, 300
	Send {Backspace 11}
	Sleep, 200
	Send ^a
	Sleep, 300
	Send %stock%
	Sleep, 500
	Send {Enter}
	Sleep, 2000
	
	; Clica no campo Quantidade
	Click 1170, 210
	Sleep, 200
	Click 2
	Sleep, 200
	Send %quantity%
	Sleep, 200
	
	; Clica no campo Preco
	Click 1100, 256
	Sleep, 300
	Click 2
	Sleep, 200
	Send %price%
	Sleep, 200
	
	; Clica no botao Enviar
	; Click 1224, 400

	sleep, 1000
}


CancelAction()
{
	GetHomeBrokerPage()

	; Clica na ordem corrente
	Click 150, 550
	Sleep, 500
	; Clica na barra de rolagem descendo
	Click 1275, 580
	Sleep, 500
	; Clica no botão Cancelar Ordem
	Click 915, 595
	Sleep, 500
	; Clica no botão de confirmação (OK)
	Click 604, 350
	Sleep, 500
	; Clica na barra de rolagem subindo
	Click 1275, 110
	Sleep, 500
}



GetStockPriceAndQuantityFromTradingSystem(ByRef stock, ByRef price, ByRef quantity)
{
	GetTradingSystemPage()

	stock_file_name := "Z:\stock.txt"
	stock := GetTextFromFile(stock_file_name)
	
	price_file_name := "Z:\price.txt"
	price := GetTextFromFile(price_file_name)
	
	quantity_file_name := "Z:\quantity.txt"
	quantity := GetTextFromFile(quantity_file_name)
}



GetTextFromFile(file_name)
{
	FileRead, Contents, %file_name%
	if not ErrorLevel  ; Successfully loaded.
	{
	    ; FileDelete, file_name
	    if Contents = 
	    {
		    MsgBox O arquivo %file_name% está vazio
		    return ""
	    }
	    return %Contents%
	}
	else
	{
	    MsgBox Não consegui abrir o arquivo %file_name%
	    return ""
	}
}



UpdateTradingSystemData()
{
	GetHomeBrokerPage()

	; Clica em uma região em branco da janela do Home Broker
	Click 4, 75
	Sleep, 500

	; Clica na barra de rolagem subindo
	Click 1275, 110
	Sleep, 200

	; Clica na aba FINACEIRO
	Click 250, 510
	Sleep, 2000

	; Clica na aba SUAS ORDENS
	Click 70, 510
	Sleep, 4000


	; Copia toda a pagina do Home Broker
	Send ^a
	Sleep, 500
	clipboard = 
	Send ^c
	ClipWait
	Sleep, 500
	clipboard = %clipboard%
	Sleep, 500

	; Clica em uma região em branco da janela do Home Broker
	Click 4, 75
	Sleep, 500

	GetTradingSystemPage()

	; Clica no campo de texto do Trading System
	Click 215, 200
	; Envia o texto copiado da janela do Homebroker
	Sleep, 500
	Send ^v
	Sleep, 500

	; Clica no botao "send" do Trading System
	Click 216, 382
	Sleep, 1000
}



GetHomeBrokerPage()
{
	; Acha a janela do Home Broker
	IfWinExist MyCAP Homebroker
	{
	    WinActivate
	}
	else
	{
	    MsgBox Não achei a janela do Home Broker.
	    return
	}
	Sleep, 500
}



GetTradingSystemPage()
{
	; Acha a janela do Trading System
	IfWinExist Trading System
	{
	    WinActivate
	}
	else
	{
	    MsgBox Não achei a janela do Trading System.
	    return
	}
	Sleep, 500
}

GetTextViaDoubleClick(quem_chamou)
{
	Sleep, 1000
	Click 3
	clipboard = 
	Sleep, 300
	Send ^c
	ClipWait, 3
	clipboard = %clipboard%
	Sleep, 200
	if clipboard =
	{
		MsgBox, Nao consegui obter texto via GetTextViaDoubleClick() em %quem_chamou%
		return ""
	}
	return %clipboard%
}

;Parar o programa a qualquer momento, basta aperta F2
F2::ExitApp
