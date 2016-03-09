<?php

/* 
 * Codigo de teste
 *
$home_broker_info = array();
$home_broker_file = fopen("home_broker_data.txt", "r");
$home_broker_page = fread($home_broker_file, 1000000);
extract_data($home_broker_page);

print_stocks_prices_and_volumes("\n");
print_book("\n");
print_stocks_prices_and_volumes("\n");
print_book("\n");
print "capital = " . $home_broker_info["capital"] . "\n";
print_order_status();

print "Error: ".$error_message."\n";

 *
 * Fim do codigo de teste
 */


function extract_data($home_broker_page)
{
    global $home_broker_info;
    global $test_mode;
    global $error_message;
    
    $home_broker_info["system_time"] = get_system_time();
    if ($test_mode == false)
        $home_broker_info["now_time"] = time();
    else
        $home_broker_info["now_time"] = get_test_time();    

    $home_broker_file_lines = explode("\r\n", $home_broker_page);
    /*
     * Os dados completos do home broker tem cerca de 700 linhas
     * Se existir menos que 350 linhas consideramos o arquivo incompleto
     */
    if (count($home_broker_file_lines) < 350)
    {
        $error_message = "home broker page incomplete";
        return false;
    }
    
    if (!get_stocks_prices_and_volumes($home_broker_file_lines))
    {
        $error_message = "could not get all stock prices and volumes";
        return false;
    }
    
    if (!get_current_stock_book_info($home_broker_file_lines))
    {
        $error_message = "could not get current book information";
        return false;
    }
    
    if (!get_capital($home_broker_file_lines))
    {
        $error_message = "could not get current capital";
        return false;
    }
    
    if (!get_order_status($home_broker_file_lines))
    {
        $error_message = "could not get current order status";
        return false;
    }

    return true;
}

function extract_data_test()
{
    global $date_to_run;
    global $home_broker_info;

    update_test_time(20);
    $times = get_time(true);

    $home_broker_info["id_time"] = $times[0];
    $home_broker_info["last_full_minute_time"] = $times[1];    
    $home_broker_info["system_time"] = $times[2];

    $home_broker_info["capital"] = 20000.00;
    $home_broker_info["sell_order_pending"] = false;
    $home_broker_info["buy_order_pending"] = false;


    if(full_minute(false))
    {
        $stocks_file = fopen("acoes_de_interesse.txt","r");
        $stock_name = trim(fgets($stocks_file));
        while(!feof($stocks_file))
        {
            if($home_broker_info["id_time"] == "-1")
                $pos = 0;
            else
                $pos = intval($home_broker_info["id_time"]) + 1;

            $stock_reference_file_name = "TXT_INTRADAY_XML/".$date_to_run."-base/" . $stock_name . ".SA.xml.txt";
            $stock_reference_file = fopen($stock_reference_file_name, "r");
            $line = explode(" ",readline_by_pos($stock_reference_file, $pos));
            $home_broker_info[$stock_name] = $line[0].";".$line[1].";".$line[2];
            $stock_name = trim(fgets($stocks_file));
            fclose($stock_reference_file);
        }
        fclose($stocks_file);
    }
}


function print_stocks_prices_and_volumes($line_feed)
{
    global $home_broker_info;
    
    $stocks_file = fopen("acoes_de_interesse.txt", "r");
    
    $stock = trim(fgets($stocks_file));
    while (!feof($stocks_file))
    {
        print $stock . "  " . $home_broker_info[$stock] . $line_feed;

        $stock = trim(fgets($stocks_file));
    }
}



function print_book($line_feed)
{
    global $home_broker_info;

    for ($i = 0; $i < 15; $i++)
    {
        print "Compra " . $home_broker_info["book_buy_quantity" . $i]  . "\t" .
                                    $home_broker_info["book_buy_value" . $i]  . "\t" .
                                    $home_broker_info["book_sell_value" . $i]  . "\t" .
                                    $home_broker_info["book_sell_quantity" . $i] . "\tVenda" . $line_feed;
    }
}



function get_current_stock_book_info($home_broker_file_lines)
{
    global $home_broker_info;
    
    $line_number = 0;
    do 
    {
        $line = $home_broker_file_lines[$line_number];
        $line_number = $line_number + 1;
    } while (($line != "Qtde.	\$Compra	\$Venda	Qtde.") && 
                  ($line_number < count($home_broker_file_lines)));
    
    /*
     * Verifica se o home_broker_file_lines possui a quantidade
     * minima para rodar o loop abaixo.
     */
    if(count($home_broker_file_lines) < ($line_number + 15 * 7))
        return false;
    
    for ($i = 0; $i < 15; $i++)
    {
        $line = $home_broker_file_lines[$line_number];
        $home_broker_info["book_buy_quantity" . $i] = number_2_php(str_replace("\n", "", $line));

        $line_number = $line_number + 2;
        $line = $home_broker_file_lines[$line_number];
        $home_broker_info["book_buy_value" . $i] = number_2_php(str_replace("\n", "", $line));

        $line_number = $line_number + 2;
        $line = $home_broker_file_lines[$line_number];
        $home_broker_info["book_sell_value" . $i] =  number_2_php(str_replace("\n", "", $line));

        $line_number = $line_number + 2;
        $line = $home_broker_file_lines[$line_number];
        $home_broker_info["book_sell_quantity" . $i] =  number_2_php(str_replace("\n", "", $line));
        
        $line_number = $line_number + 1;
    }
    return true;
}


function get_stocks_prices_and_volumes($home_broker_file_lines)
{
    global $home_broker_info;
    
    $stocks_file = fopen("acoes_de_interesse.txt", "r");
    
    $stock = trim(fgets($stocks_file));
    $line = get_first_stock_line_number($home_broker_file_lines);

    if ($line == false)
        return false;

    while (!feof($stocks_file))
    {
        while ($line < count($home_broker_file_lines))
        {
            $home_broker_file_line = explode("\t", $home_broker_file_lines[$line]);
            if ($stock == $home_broker_file_line[0])
                break;
            $line = $line + 1;
        }
        $home_broker_info[$stock] = get_stock_price_and_volume($home_broker_file_lines[$line]);

        $stock = trim(fgets($stocks_file));
        $line = $line + 1;
    }

    return true;
}


function get_first_stock_line_number($home_broker_file_lines)
{
    $line_number = 0;
    $line = $home_broker_file_lines[$line_number];
    $line_fields = explode("\t", $line);
    while ($line_fields[0] != "IBOV") # o primeiro stock tem que ser o IBOV
    {
        $line_number = $line_number + 1;
        $line = $home_broker_file_lines[$line_number];
        $line_fields = explode("\t", $line);
    }
    $line_number = $line_number + 1;
    $line = $home_broker_file_lines[$line_number];
    $line_fields = explode("\t", $line);
    if ($line_fields[0] != "DOLCM")
        return false; # o segundo stock tem que ser DOLCM
    else
        return $line_number + 1;
}


function get_stock_price_and_volume($home_broker_file_line)
{
    $line_fields = explode("\t", $home_broker_file_line);
    $price = number_2_php($line_fields[1]); # o preco tem que estar na posicao 1
    $volume = number_2_php(adjust_volume_format($line_fields[9])); # o volume tem que estar na posicao 9
    
    $price_and_volume = $price . " " . $volume;
    return $price_and_volume;
}


function adjust_volume_format($raw_volume)
{
    $volume = str_replace("M", "000000", $raw_volume); # tem que trocar M por milhão, etc
    
    return $volume;
}


function get_capital($home_broker_file_lines)
{
    global $home_broker_info; 
    $line_number = 0;
    do 
    {
        $line = $home_broker_file_lines[$line_number];
        $line_fields = explode("\t", $line);
        $line_number = $line_number + 1;
    } while (($line_fields[0] != "Saldo em Conta ") && ($line_number < count($home_broker_file_lines)));
    if ($line_number >= count($home_broker_file_lines))
        return false; 
    else
        $home_broker_info["capital"] = number_2_php($line_fields[5]);

    return true;
}


function get_order_status($home_broker_file_lines)
{
    global $home_broker_info;
    
    $line_number = 0;
    do 
    {
        $line = $home_broker_file_lines[$line_number];
        $line_number = $line_number + 1;
    } while (($line != "QtdeExec") && 
                  ($line_number < count($home_broker_file_lines)));

    if ($line_number >= count($home_broker_file_lines))
        return false; 
    
    $line = $home_broker_file_lines[$line_number];
    $line_fields = explode("\t", $line);
    if (count($line_fields) < 8)
    {
        $home_broker_info["order_status_order_number"] = "";
        $home_broker_info["order_status_stock"] = "";
        $home_broker_info["order_status_type"] = ""; # C = compra, V = venda
        $home_broker_info["order_status_status"] = ""; # Enviada, Executada, Cancelada, Pend. Canc.
        $home_broker_info["order_status_quantity"] = "";
        $home_broker_info["order_status_price"] = "";
        $home_broker_info["order_status_total"] = ""; # só é diferente de zero se a ordem for executada
    }
    else
    {
        $home_broker_info["order_status_order_number"] = $line_fields[1];
        $home_broker_info["order_status_stock"] = $line_fields[2];
        $home_broker_info["order_status_type"] = $line_fields[3]; # C = compra, V = venda
        $home_broker_info["order_status_status"] = $line_fields[5]; # Enviada, Executada, Cancelada, Pend. Canc.
        $home_broker_info["order_status_quantity"] = number_2_php($line_fields[6]);
        $home_broker_info["order_status_price"] = number_2_php($line_fields[7]);
        $home_broker_info["order_status_total"] = number_2_php($line_fields[8]); # só é diferente de zero se a ordem for executada
    }

    return true;
}


function print_order_status()
{
    global $home_broker_info;
    
    print "order number = " . $home_broker_info["order_status_order_number"] . ", ";
    print "stock = " . $home_broker_info["order_status_stock"] . ", ";
    print "type = " . $home_broker_info["order_status_type"] . ", "; # C = compra, V = venda
    print "status = " . $home_broker_info["order_status_status"] . ", "; # Enviada, Executada, Cancelada, Pend. Canc.
    print "quantity = " . $home_broker_info["order_status_quantity"] . ", ";
    print "price = " . $home_broker_info["order_status_price"] . ", ";
    print "total = " . $home_broker_info["order_status_total"] . "\n";  # só é diferente de zero se a ordem for executada
}


function number_2_php($number_from_interface)
{
    $new_number = str_replace(".", "", $number_from_interface);
    $new_number = str_replace(",", ".", $new_number);
    
    return $new_number;
}


function number_2_interface($number_from_php)
{
    $new_number = str_replace(".", ",", $number_from_php);
    
    return $new_number;
}


function save_home_broker_data_log($home_broker_page, $home_broker_info)
{
    $file = fopen("home_broker_data_log/".date("Y-m-d-G-i-s")."-home_broker_page.txt", "w+");
    fwrite($file, $home_broker_page);
    fclose($file);

    $file = fopen("home_broker_data_log/".date("Y-m-d-G-i-s")."-home_broker_info.txt", "w+");
    foreach($home_broker_info as $key => $value)
    {
        fwrite($file, $key.": ".$value."\n");
    }
    fclose($file);
}

?>
