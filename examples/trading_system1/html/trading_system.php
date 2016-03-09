<?php
header("Cache-Control: no-cache, must-revalidate"); 
header("Expires: Sat, 26 Jul 1997 05:00:00 GMT"); 


/*
 * Trading System parameters
 */
$date_to_run = "2010-11-04";
$sample_size = 20;      # tamanho de uma amostra do preditor em minutos
$stop_loss = -0.002;    # retorno abaixo do qual vende pelo livro
$stop_gain = 0.001;     # retorno acima do qual vende pelo livro
$start_buy = -0.001;    # retorno de disparo de compra
$time_out_to_sell = 3*60; # time out para vender em segundos
$time_out_to_buy = 12*60;

$just_collecting_data = true; # usada para operar junto com o AutoHotkey para apenas coletar dados (sempre pensa que comprou e vendeu)
$ignore_trading_time_interval = false;


/*
 * Global Variable to Interface
 */
$ui_state = "IDLE";
$ui_action = "IDLE";
$ui_stock = "";
$ui_price = "";
$ui_qty = "";
$home_broker_info = array();
$home_broker_page = "";
$full_minute = false;
$full_sample = false;
$trading_system_status = "OK";
$error_message = "-";
$test_mode = false; # usado para implementar os botoes de teste
$trading_time_interval = true;
$gain_or_loss = $stop_loss - 0.0001;

include "trading_system_state_machine.php";
include "state_and_time.php";
include "order_management.php";
include "state_management.php";
include "filter_home_broker_data.php";

/*
 * Post
 */
if (isset($_POST["time_step"]))
{
    global $home_broker_page;
    global $home_broker_info;
    global $test_mode;
    
    $test_mode = true;
    $home_broker_page = file_get_contents("home_broker_data.txt-cancelada");
    if (!extract_data($home_broker_page))
    {
        $trading_system_status = "Could not get data!";
        $ui_state = "ERROR!";
    }
    else
    {
        trading_system();

        if (file_exists("buy_order.txt"))
            system("cp buy_order.txt buy_order.txt.bak");
        if (file_exists("sell_order.txt"))
            system("cp sell_order.txt sell_order.txt.bak");
    }
    save_home_broker_data_log($home_broker_page, $home_broker_info);
    save_user_interface_transfer_files();
    update_test_time(15);
}

if (isset($_POST["gain_or_loss"]))
{
    global $home_broker_page;
    global $home_broker_info;
    global $test_mode;
    global $gain_or_loss;
    
    $test_mode = true;
    $home_broker_page = file_get_contents("home_broker_data.txt-compra-executada");
    if (!extract_data($home_broker_page))
    {
        $trading_system_status = "Could not get data!";
        $ui_state = "ERROR!";
    }
    else
    {
        $buy_order_file = fopen("buy_order.txt.bak", "r");
        $last_buy_order_fields = explode(" ", fgets($buy_order_file)); 
        fclose($buy_order_file);
        
        $home_broker_info["order_status_stock"] = $last_buy_order_fields[2];
        $home_broker_info["order_status_type"] = "C"; # C = compra, V = venda
        $home_broker_info["order_status_status"] = "Executada"; # Enviada, Executada, Cancelada, Pend. Canc.
        $home_broker_info["order_status_quantity"] = number_2_php($last_buy_order_fields[4]);
        $home_broker_info["order_status_price"] = number_2_php($last_buy_order_fields[3]);
        $home_broker_info["order_status_total"] = $home_broker_info["order_status_quantity"] * $home_broker_info["order_status_price"] ; # só é diferente de zero se a ordem for executada

        $current_stock_price_and_volume = explode(" ", $home_broker_info[$home_broker_info["order_status_stock"]]);  
        #print $current_stock_price_and_volume[0] . " ";
        $current_stock_price_and_volume[0] = number_2_php($last_buy_order_fields[3]) + number_2_php($last_buy_order_fields[3]) * $gain_or_loss;
        #print $current_stock_price_and_volume[0] . " ";
        $home_broker_info[$home_broker_info["order_status_stock"]] = $current_stock_price_and_volume[0] . " " . $current_stock_price_and_volume[1];
        
        $home_broker_info["book_buy_quantity0"] = $home_broker_info["order_status_quantity"];
        $home_broker_info["book_buy_value0"] = $current_stock_price_and_volume[0];
        
        trading_system();
        if (file_exists("buy_order.txt"))
            system("cp buy_order.txt buy_order.txt.bak");
        if (file_exists("sell_order.txt"))
            system("cp sell_order.txt sell_order.txt.bak");
    }
    save_home_broker_data_log($home_broker_page, $home_broker_info);
    save_user_interface_transfer_files();
    update_test_time(15);
}

if (isset($_POST["bought"]))
{
    global $home_broker_page;
    global $home_broker_info;
    global $test_mode;
    
    $test_mode = true;
    $home_broker_page = file_get_contents("home_broker_data.txt-compra-executada");
    if (!extract_data($home_broker_page))
    {
        $trading_system_status = "Could not get data!";
        $ui_state = "ERROR!";
    }
    else
    {
        system("cp buy_order.txt.bak buy_order.txt");
        $buy_order_file = fopen("buy_order.txt", "r");
        $last_buy_order_fields = explode(" ", fgets($buy_order_file)); 
        fclose($buy_order_file);
        
        $home_broker_info["order_status_stock"] = $last_buy_order_fields[2];
        $home_broker_info["order_status_type"] = "C"; # C = compra, V = venda
        $home_broker_info["order_status_status"] = "Executada"; # Enviada, Executada, Cancelada, Pend. Canc.
        $home_broker_info["order_status_quantity"] = number_2_php($last_buy_order_fields[4]);
        $home_broker_info["order_status_price"] = number_2_php($last_buy_order_fields[3]);
        $home_broker_info["order_status_total"] = $home_broker_info["order_status_quantity"] * $home_broker_info["order_status_price"] ; # só é diferente de zero se a ordem for executada

        trading_system();
        if (file_exists("buy_order.txt"))
            system("cp buy_order.txt buy_order.txt.bak");
        if (file_exists("sell_order.txt"))
            system("cp sell_order.txt sell_order.txt.bak");
    }
    save_home_broker_data_log($home_broker_page, $home_broker_info);
    save_user_interface_transfer_files();
    update_test_time(15);
}

if (isset($_POST["pending_sell"]))
{
    global $home_broker_page;
    global $home_broker_info;
    global $test_mode;

    $test_mode = true;
    $home_broker_page = file_get_contents("home_broker_data.txt-venda-pendente");
    if (!extract_data($home_broker_page))
    {
        $trading_system_status = "Could not get data!";
        $ui_state = "ERROR!";
    }
    else
    {
        trading_system();
        if (file_exists("buy_order.txt"))
            system("cp buy_order.txt buy_order.txt.bak");
        if (file_exists("sell_order.txt"))
            system("cp sell_order.txt sell_order.txt.bak");
    }
    save_home_broker_data_log($home_broker_page, $home_broker_info);
    save_user_interface_transfer_files();
    update_test_time(15);
}

if (isset($_POST["sold"]))
{
    global $home_broker_page;
    global $home_broker_info;
    global $test_mode;
    
    $test_mode = true;
    $home_broker_page = file_get_contents("home_broker_data.txt-venda-executada");
    if (!extract_data($home_broker_page))
    {
        $trading_system_status = "Could not get data!";
        $ui_state = "ERROR!";
    }
    else
    {
        system("cp sell_order.txt.bak sell_order.txt");
        $sell_order_file = fopen("sell_order.txt", "r");
        $last_sell_order_fields = explode(" ", fgets($sell_order_file)); 
        fclose($sell_order_file);
        
        $home_broker_info["order_status_stock"] = $last_sell_order_fields[2];
        $home_broker_info["order_status_type"] = "V"; # C = compra, V = venda
        $home_broker_info["order_status_status"] = "Executada"; # Enviada, Executada, Cancelada, Pend. Canc.
        $home_broker_info["order_status_quantity"] = number_2_php($last_sell_order_fields[4]);
        $home_broker_info["order_status_price"] = number_2_php($last_sell_order_fields[3]);
        $home_broker_info["order_status_total"] = $home_broker_info["order_status_quantity"] * $home_broker_info["order_status_price"] ; # só é diferente de zero se a ordem for executada

        trading_system();
        if (file_exists("buy_order.txt"))
            system("cp buy_order.txt buy_order.txt.bak");
        if (file_exists("sell_order.txt"))
            system("cp sell_order.txt sell_order.txt.bak");
    }
    save_home_broker_data_log($home_broker_page, $home_broker_info);
    save_user_interface_transfer_files();
    update_test_time(15);
}


if (isset($_POST["input"])) 
{
    global $home_broker_page;
    global $home_broker_info;
    
    $home_broker_page = $_POST["input"];
    if (!extract_data($home_broker_page))
    {
        $trading_system_status = "Could not get data!";
        $ui_state = "ERROR!";
    }
    else 
    {
        trading_system();
    }
    save_home_broker_data_log($home_broker_page, $home_broker_info);
    save_user_interface_transfer_files();
}

#$ui_action = "BUY";
#$ui_stock = "PETR4";
#$ui_price = "22,00";
#$ui_qty = "100";
#save_user_interface_transfer_files();

function save_user_interface_transfer_files()
{
    global $ui_action;
    global $ui_stock;
    global $ui_price;
    global $ui_qty;
    
    $user_interface_transfer_file = fopen("action.txt", "w");
    fwrite($user_interface_transfer_file, $ui_action);
    fclose($user_interface_transfer_file);
    
    $user_interface_transfer_file = fopen("stock.txt", "w");
    fwrite($user_interface_transfer_file, $ui_stock);
    fclose($user_interface_transfer_file);
    
    $user_interface_transfer_file = fopen("price.txt", "w");
    fwrite($user_interface_transfer_file, $ui_price);
    fclose($user_interface_transfer_file);
    
    $user_interface_transfer_file = fopen("quantity.txt", "w");
    fwrite($user_interface_transfer_file, $ui_qty);
    fclose($user_interface_transfer_file);
}


?>
<html>
<body>
<head>
<title>Trading System</title>
<head>
<table border="1">
<tr>
    <td>
        <form name="inputform" method="post" action="trading_system.php">
        <textarea style="height:200px; width:400px" name="input"></textarea>
        <br>
        <button style="height:50px; width:400px">send</button>
        </form>
    </td>
    <td style="vertical-align:top">
        <table border="1">
        <tr>
            <td>STATE: </td>
            <td><input type="text" style="width:200px;height:30px;" value="<?=$ui_state;?>"/></td>
        </tr>
        <tr>
            <td>ACTION: </td>
            <td><input type="text" style="width:200px;height:30px;" value="<?=$ui_action;?>"/></td>
        </tr>
        <tr>
            <td>STOCK: </td>
            <td><input type="text" style="width:200px;height:30px;" value="<?=$ui_stock;?>"/></td>
        </tr>
        <tr>
            <td>PRICE: </td>
            <td><input type="text" style="width:200px;height:30px;" value="<?=$ui_price;?>"/></td>
        </tr>
        <tr>
            <td>QTY: </td>
            <td><input type="text" style="width:200px;height:30px;" value="<?=$ui_qty;?>"/></td>
        </tr>
        </table>
    <td>
    <td style="vertical-align:top">
        <table border="1">
            <tr>
                <td>Capital: </td>
                <td><? print $home_broker_info["capital"] ?></td>
            </tr>
            <tr>
                <td>System Now: </td>
                <td><? print date('G:i:s Y-m-d', $home_broker_info["now_time"]); ?></td>
            </tr>
            <tr>
               <td>System Time: </td>
               <td><? print date('G:i:s Y-m-d', $home_broker_info["system_time"]); ?></td>
            </tr>
            <tr>
               <td>Full Minute: </td>
               <td><? if ($full_minute) print "true"; else print "false";?></td>
            </tr>
            <tr>
               <td>Full Sample: </td>
               <td><? if ($full_sample) print "true"; else print "false";?></td>
            </tr>
           <tr>
               <td>Trading System Status: </td>
               <td><? print $trading_system_status;?></td>
            </tr>
           <tr>
               <td>Error: </td>
               <td><? print $error_message;?></td>
            </tr>
            <tr>
               <td>Trading Time Interval: </td>
               <td><? if ($trading_time_interval) print "true"; else print "false"; ?></td>
            </tr>
        </table>
    </td>
</tr>
<tr>
    <td colspan="4">
    <form name="test_form" method="post" action="trading_system.php">
    <button name="time_step">time step</button>
    <button name="gain_or_loss">gain or loss</button>
    <button name="bought">bought</button>
    <button name="pending_sell">pending sell</button>
    <button name="sold">sold</button>
    </form>
    </td>
</tr>
<table>
<?

/*
$mtime = strtotime("2010-10-6, 10:00:1");
print $mtime . "<br>";
print date('Y-m-d H:i:s', $mtime);
*/

?>
</body>
</html>
