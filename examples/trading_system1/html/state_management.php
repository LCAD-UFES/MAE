<?php


function clear_user_interface()
{
    global $ui_action;    
    global $ui_stock;
    global $ui_price;
    global $ui_qty;

    $ui_action = "IDLE";
    $ui_stock = "";
    $ui_price = "";
    $ui_qty = "";
}


function exec_sell_and_predict_state($predicted)
{
    if ($predicted == false)
    {
        if (!file_exists("predicting.txt"))
        {
            make_preditions_and_train_with_new_data();
            $predicting_file = fopen("predicting.txt", "w");
            fwrite($predicting_file, "predicting...");
            fclose($predicting_file);
        }
        write_log("[state_management.php] [exec_sell_and_predict_state]: predicted = false");
    }
    else
    {
        unlink("predicting.txt");
        write_log("[state_management.php] [exec_sell_and_predict_state]: predicted = true");
    }
    
    $sold = send_order_to_sell_last_stock_bought();
    if ($sold == true)
        write_log("[state_management.php] [exec_sell_and_predict_state]: sold = true");
    else
        write_log("[state_management.php] [exec_sell_and_predict_state]: sold = false");
    return $sold;
}


/*
 * Se a ultima ordem executada foi de compra e as perdas maiores que $stop_loss_return -> true
 */
function stop_loss($stop_loss_return)
{
    global $home_broker_info;

    $price_being_ofered_for_current_quantity = get_price_from_book($home_broker_info["order_status_quantity"]);
    $eventual_percentual_loss = ($price_being_ofered_for_current_quantity - $home_broker_info["order_status_price"]) / $home_broker_info["order_status_price"];
    if (($home_broker_info["order_status_status"] == "Executada") &&
        ($home_broker_info["order_status_type"] == "C") &&
        ($eventual_percentual_loss <= $stop_loss_return))
    {
        write_log("[state_management.php] [stop_loss]: Curr. loss = " . 
                        $eventual_percentual_loss . ". Stop loss = " .
                        $stop_loss_return . "  " .
                        $home_broker_info["order_status_stock"] . " prev. price = " . 
                        $home_broker_info["order_status_price"] . " curr. price = " .
                        $price_being_ofered_for_current_quantity);
        return true;
    }
    else
    {
        return false;
    }
}


/*
 * Se a ultima ordem executada foi de compra e as perdas maiores que $stop_gain_return -> true
 */
function stop_gain($stop_gain_return)
{
    global $home_broker_info;

    $price_being_ofered_for_current_quantity = get_price_from_book($home_broker_info["order_status_quantity"]);
    $eventual_percentual_gain = ($price_being_ofered_for_current_quantity - $home_broker_info["order_status_price"]) / $home_broker_info["order_status_price"];
    if (($home_broker_info["order_status_status"] == "Executada") &&
        ($home_broker_info["order_status_type"] == "C") &&
        ($eventual_percentual_gain >= $stop_gain_return))
    {
        write_log("[state_management.php] [stop_gain]: Curr. gain = " . 
                        $eventual_percentual_gain . ". Stop gain = " .
                        $stop_gain_return . "  " .
                        $home_broker_info["order_status_stock"] . " prev. price = " . 
                        $home_broker_info["order_status_price"] . " curr. price = " .
                        $price_being_ofered_for_current_quantity);
        return true;
    }
    else
    {
        return false;
    }
}


function get_sell_state_conditions(&$sold, &$time_out_to_sell, &$trading_time_interval)
{
    global $home_broker_info;

    $trading_time_interval = trading_time_interval();

    if (!file_exists("sell_order.txt"))
    {
        $ui_action = "IDLE";
        $ui_stock = "";
        $ui_price = "";
        $ui_qty = "";
        
        $sold = true;
        $time_out_to_sell = false;
        write_log("[state_management.php] [get_sell_state_conditions]: nothing to do...");
        return;
    }

    $sell_order_file = fopen("sell_order.txt", "r");
    $last_sell_order_fields = explode(" ", fgets($sell_order_file)); 
    fclose($sell_order_file);

    if (($home_broker_info["order_status_status"] == "Executada") &&
        ($home_broker_info["order_status_type"] == "V") &&
        ($home_broker_info["order_status_stock"] == $last_sell_order_fields[2]) &&
#        ($home_broker_info["order_status_price"] == number_2_php($last_sell_order_fields[3])) &&
#        ($home_broker_info["order_status_total"] >= $home_broker_info["order_status_price"] * $home_broker_info["order_status_quantity"]) &&
        ($home_broker_info["order_status_quantity"] == number_2_php($last_sell_order_fields[4])))
    {
        $sold = true;
        $time_out_to_sell = false;
        unlink("sell_order.txt");
        write_log("[state_management.php] [get_sell_state_conditions]: sold = true. " . 
                        $home_broker_info["order_status_status"] . " " .
                        $home_broker_info["order_status_type"] . " " .
                        $home_broker_info["order_status_stock"] . " " . 
                        $home_broker_info["order_status_price"] . " " .
                        $home_broker_info["order_status_quantity"] . " " .
                        $home_broker_info["order_status_total"]);
    }
    else
    {
        $sold = false;
        $time_out_to_sell = timeout_to_sell();
        write_log("[state_management.php] [get_sell_state_conditions]: sold = false. " . 
                        $home_broker_info["order_status_status"] . " " .
                        $home_broker_info["order_status_type"] . " " .
                        $home_broker_info["order_status_stock"] . " " .
                        $home_broker_info["order_status_price"] . " " .
                        $home_broker_info["order_status_quantity"] . " " .
                        $home_broker_info["order_status_total"]);
    }
}


function get_buy_state_conditions(&$bought, &$time_out_to_buy)
{
    global $home_broker_info;

    $buy_order_file = fopen("buy_order.txt", "r");
    $last_buy_order_fields = explode(" ", fgets($buy_order_file)); 
    fclose($buy_order_file);

    if (($home_broker_info["order_status_status"] == "Executada") &&
        ($home_broker_info["order_status_type"] == "C") &&
        ($home_broker_info["order_status_stock"] == $last_buy_order_fields[2]) &&
#        ($home_broker_info["order_status_price"] == number_2_php($last_buy_order_fields[3])) &&
#        ($home_broker_info["order_status_total"] <= $home_broker_info["order_status_price"] * $home_broker_info["order_status_quantity"]) &&
        ($home_broker_info["order_status_quantity"] == number_2_php($last_buy_order_fields[4])))
    {
        $bought = true;
        $time_out_to_buy = false;
        write_log("[state_management.php] [get_buy_state_conditions]: bought = true. " . 
                        $home_broker_info["order_status_status"] . " " .
                        $home_broker_info["order_status_type"] . " " .
                        $home_broker_info["order_status_stock"] . " " .
                        $home_broker_info["order_status_price"] . " " .
                        $home_broker_info["order_status_quantity"] . " " .
                        $home_broker_info["order_status_total"]);
    }
    else
    {
        $bought = false;
        $time_out_to_buy = timeout_to_buy();
        write_log("[state_management.php] [get_buy_state_conditions]: bought = false. " . 
                        $home_broker_info["order_status_status"] . " " .
                        $home_broker_info["order_status_type"] . " " .
                        $home_broker_info["order_status_stock"] . " " .
                        $home_broker_info["order_status_price"] . " " .
                        $home_broker_info["order_status_quantity"] . " " .
                        $home_broker_info["order_status_total"]);
    }
}


function add_data_to_database()
{
    global $date_to_run;
    global $home_broker_info;

    $stocks_file = fopen("acoes_de_interesse.txt","r");
    $stock_name = trim(fgets($stocks_file));
    while (!feof($stocks_file))
    {       
        $stock_file_name = "TXT_INTRADAY_XML/".$date_to_run."/".$stock_name.".SA.xml.txt";
        $stock_file = fopen($stock_file_name, "a+");
        $stock_price_and_volume = $home_broker_info[$stock_name];
        $line_of_data = date("H:i", $home_broker_info["system_time"]) . " " . $stock_price_and_volume . "\n";
        fwrite($stock_file, $line_of_data);
        fclose($stock_file);
        $stock_name = trim(fgets($stocks_file));
    }
    fclose($stocks_file);
}


function make_preditions_and_train_with_new_data()
{
    $str_command = './run_pred_trad.sh "2010-?(10-29|11-01|11-03|11-04)" TT_new wnn_pred_predict.cml';
    system($str_command);
}



/*
 * Add one quote to each stock file. 
 * It should use a quote from the home broker for each stock only if it is from a full minute. 
 * But, for now, it uses a reference file for getting quotes. 
 *
function add_data_to_database($stock_list_file_name, $date) 
{
    $stock_list_file = fopen($stock_list_file_name, "r");
    $stock_name = fgets($stock_list_file);
    while(!feof($stock_list_file)) 
   {
        $stock_name = str_replace("\n", '', $stock_name);

        $stock_file_name = "TXT_INTRADAY_XML/" . $date . "/" . $stock_name . ".SA.xml.txt";
        $stock_file = fopen($stock_file_name, "c+");

        $stock_reference_file_name = "TXT_INTRADAY_XML/2010-09-17-base/" . $stock_name . ".SA.xml.txt";
        $stock_reference_file = fopen($stock_reference_file_name, "r");
	
        $quote = get_quote_from_reference_file($stock_file, $stock_reference_file);
        #print $stock_reference_file_name . " " . $quote . "<br>";
        fwrite($stock_file, $quote);

        fclose($stock_file);
        fclose($stock_reference_file);

        $stock_name = fgets($stock_list_file);
    }
    fclose($stock_list_file);
}
 */


/*
 * Get a quote from a reference file.
 * Read both files until the end of $stock_file. Then, read one more line from $stock_reference_file and return it.
 */
function get_quote_from_reference_file($stock_file, $stock_reference_file)
{
    while(!feof($stock_file)) 
    {
        $quote = fgets($stock_file);
        $quote = fgets($stock_reference_file);
    }
    return $quote;
}

/*
 * Read Line by position
 * Test
 */

function readline_by_pos($file, $pos)
{
    $i = 0;
    while($i <= $pos)
    {
        $line = fgets($file);
        $i++;
    }
    return $line;
}


/*
 * Log function
 *
 */
function write_log($stringlog)
{
    global $home_broker_info;

    if(file_exists("log.txt"))
    {
        $log_file = fopen("log.txt", "a");
        fwrite($log_file, date("G:i:s", $home_broker_info["now_time"]) . " " . $stringlog . "\n");
        fclose($log_file);
    }
}

?>
