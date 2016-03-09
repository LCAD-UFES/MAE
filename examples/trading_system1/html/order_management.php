<?php

/*
 * File: sell_order.txt
 * Format file: stock;price;quantity;timestamp
 * Log of all sell orders
 */
$sell_order_file_name = "sell_order.txt";

/*
 * File: buy_order.txt
 * Format file: stock;price;quantity;timestamp
 * Log of all buy orders
 */
$buy_order_file_name = "buy_order.txt";

/*
 * File: stock_suggested.txt
 * Format file: stock;return;hourtime
 * This file always has one line.
 */
$stock_suggested_file = "stock_suggested.txt";

/*
 * Retorna true se o arquivo buy_order.txt não existir e, caso contrário, fase
 */
function send_order_to_sell_last_stock_bought()
{
    global $home_broker_info;
    global $sell_order_file_name;
    global $buy_order_file_name;

    global $ui_action;    
    global $ui_stock;
    global $ui_price;
    global $ui_qty;

    if (!file_exists($buy_order_file_name))
    {
        $ui_action = "IDLE";
        $ui_stock = "";
        $ui_price = "";
        $ui_qty = "";
        
        write_log("[order_management.php] [send_order_to_sell_last_stock_bought]: nothing to do...");
        return;
    }

    $buy_order_file = fopen($buy_order_file_name, "r");
    $last_buy_order_fields = explode(" ", fgets($buy_order_file)); 
    fclose($buy_order_file);
    unlink($buy_order_file_name);
    
    /* TODO: Checar se a ordem em home_broker_info bate com o arquivo */
    $ui_action = "SELL";
    $ui_stock = $last_buy_order_fields[2];
    $ui_qty = $last_buy_order_fields[4];
    $stock_price_and_volume = explode(" ", $home_broker_info[$ui_stock]);
    $ui_price = number_2_interface($stock_price_and_volume[0]); # vende pelo preco atual!

    $order_description = $home_broker_info["system_time"] . " " . $home_broker_info["now_time"] . " " . $ui_stock . " " . $ui_price . " " . $ui_qty;
    $sell_order_file = fopen($sell_order_file_name, "w");
    fwrite($sell_order_file, $order_description);
    fclose($sell_order_file);

    $order_description = date("G:i:s", $home_broker_info["system_time"]) . " " . 
                                     date("G:i:s", $home_broker_info["now_time"]) . " " . $ui_stock . " " . $ui_price . " " . $ui_qty;
  
    write_log("[order_management.php] [send_order_to_sell_last_stock_bought]: sell order - " . $order_description);  
}


function send_buy_order_suggested_by_trading_system($just_collecting_data)
{
    global $home_broker_info;
    global $buy_order_file_name;

    global $ui_action;    
    global $ui_stock;
    global $ui_price;
    global $ui_qty;
    
    global $start_buy;

    $order_suggested = get_order();

    $stock = trim($order_suggested[0]);  
    $stock_price_and_volume = explode(" ", $home_broker_info[$stock]);  
    $price = $stock_price_and_volume[0]; # preco atual
    $price = $price + $price * $start_buy; # preco de compra ee igual ao preco atual mais um retorno negativo ou positivo
    if ($start_buy < 0)
        $price = round($price - 0.005, 2);
    else
        $price = round($price, 2);
        
    $num_stocks_possible = intval($home_broker_info["capital"] / $price);
    $num_stocks_possible = $num_stocks_possible - $num_stocks_possible % 100; # lote sempre de 100 acoes
    if (($num_stocks_possible != 0) || $just_collecting_data)
    {
        $ui_action = "BUY";
        $ui_stock = $stock;  
        $ui_price = number_2_interface($price); 
        $ui_qty = number_2_interface($num_stocks_possible);
        
        $order_description = $home_broker_info["system_time"] . " " . $home_broker_info["now_time"] . " " . $ui_stock . " " . $ui_price . " " . $ui_qty;
        $buy_order_file = fopen($buy_order_file_name,"w+");
        fwrite($buy_order_file, $order_description);
        fclose($buy_order_file);

        $order_description = date("G:i:s", $home_broker_info["system_time"]) . " " . 
                                         date("G:i:s", $home_broker_info["now_time"]) . " " . $ui_stock . " " . $ui_price . " " . $ui_qty;
        write_log("[order_management.php] [send_buy_order_suggested_by_trading_system]: buy order - ". $order_description);
        return true;
    }
    else # nao da para comprar nem 1 lote
    {
        write_log("[order_management.php] [send_buy_order_suggested_by_trading_system]: cannot buy a lote (not enough capital)");
        return false;
    }
}


function send_order_to_sell_by_book()
{
    global $home_broker_info;
    global $sell_order_file_name;
    global $buy_order_file_name;

    global $ui_action;    
    global $ui_stock;
    global $ui_price;
    global $ui_qty;

    
    $ui_action = "SELL";
    $ui_stock = $home_broker_info["order_status_stock"];
    $ui_qty = number_2_interface($home_broker_info["order_status_quantity"]);
    $ui_price = number_2_interface(get_price_from_book($ui_qty));

    unlink($buy_order_file_name);

    $order_description = $home_broker_info["system_time"] . " " . $home_broker_info["now_time"] . " " . $ui_stock . " " . $ui_price . " " . $ui_qty;
    $sell_order_file = fopen($sell_order_file_name, "w");
    fwrite($sell_order_file, $order_description);
    fclose($sell_order_file);

    $order_description = date("G:i:s", $home_broker_info["system_time"]) . " " . 
                                     date("G:i:s", $home_broker_info["now_time"]) . " " . $ui_stock . " " . $ui_price . " " . $ui_qty;
    write_log("[order_management.php] [send_order_to_sell_by_book]: sell by book order - " . $order_description);
}


function get_price_from_book($quantity)
{
    global $home_broker_info;

    # write_log("[order_management.php] [get_price_from_book]: quantity = " . $quantity);
    $book_quantity = 0;
    for ($i = 0; $i < 15; $i++)
    {
        # write_log("[order_management.php] [get_price_from_book]: book_buy_quantity = " . $home_broker_info["book_buy_quantity" . $i]);
        $book_quantity = $book_quantity + $home_broker_info["book_buy_quantity" . $i];
        if ($book_quantity >= $quantity)
            break;
    }
    # write_log("[order_management.php] [get_price_from_book]: i = " . $i . " book_buy_value = " . $home_broker_info["book_buy_value" . $i]);
    return $home_broker_info["book_buy_value" . $i];
}


function send_cancel_order()
{
    global $ui_action;    
    global $ui_stock;
    global $ui_price;
    global $ui_qty;

    $ui_action = "CANCELORDER";
    $ui_stock = "";
    $ui_price = "";
    $ui_qty = "";

    write_log("[order_management.php] [send_cancel_order]: Cancel order.");
}


function cancel_order_finished()
{
    global $home_broker_info;

    if ($home_broker_info["order_status_status"] == "Cancelada")
        return true;
    else
        return false;
}


function predict_step_finished()
{
    global $stock_suggested_file;
    
    if (file_exists($stock_suggested_file)) 
        return true;
    else 
        return false;
}


function get_order()
{
    global $stock_suggested_file;
    
    if (predict_step_finished())
    {
        $file = fopen($stock_suggested_file, "r");
        $data = explode(" ", fgets($file));
        fclose($file);
        unlink($stock_suggested_file);
        
        return $data;
    }
    return null;
}


function write_order_log($order, $type)
{
    global $home_broker_info;

    if($type == "buy" && file_exists("buy_order_log.txt"))
    {
        $log_file = fopen("buy_order_log.txt", "a");
        fwrite($log_file, date("G:i:s", $home_broker_info["now_time"]) . " " . $stringlog . "\n");
        fclose($log_file);
    }
    
    if($type == "sell" && file_exists("sell_order_log.txt"))
    {
        $log_file = fopen("sell_order_log.txt", "a");
        fwrite($log_file, date("G:i:s", $home_broker_info["now_time"]) . " " . $stringlog . "\n");
        fclose($log_file);
    }
}



?>
