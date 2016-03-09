<?php


/*
 * Change $system_state
 */
function change_state($new_state)
{
    global $ui_state;
    
    $current_state = get_current_state();
    $system_state_file = fopen("system_state.txt", "w");
    fwrite($system_state_file, $new_state);
    fclose($system_state_file);
    
    write_log("[state_and_time.php] [change_state]: " . $current_state . " -> " . $new_state);  
    
    $ui_state = $new_state;
    return $new_state;
}

/*
 * Test if test_state == $system_state
 */
function current_state($test_state)
{
    $system_state = get_current_state();
    if (trim($test_state) == trim($system_state))
        return true;
    else
        return false;
}

/*
 * Get $system_state
 */
function get_current_state()
{
    global $ui_state;
    
    $system_state_file = fopen("system_state.txt", "r");
    $system_state = fgets($system_state_file);
    fclose($system_state_file);

    $ui_state = $system_state;
    return trim($system_state);
}

/*
 * Ckeck trading interval
 */
function trading_time_interval()
{
    global $home_broker_info;
    global $trading_time_interval;

    $today = date("Y-m-d");
    $start_time = strtotime($today.", 10:59:00");
    $end_time = strtotime($today.", 17:55:00");
    if($home_broker_info["now_time"] > $start_time && $home_broker_info["now_time"] < $end_time)
    {
        $trading_time_interval = true;
        return true;
    }    
    else
    {
        $trading_time_interval = false;
        return false;    
    }
}

/*
 * Get control time from file
 *
 * File: system_time.txt
 */
function get_system_time($test = false)
{
    if (file_exists("system_time.txt"))
    {
        $system_time_file = fopen("system_time.txt", "r");
        $system_time = fgets($system_time_file);
        fclose($system_time_file);
    }
    else
    {
        $system_time = 0;
    } 
    return $system_time;
}


/*
 * Write system time to file
 */
function set_system_time($new_time)
{
    $system_time_file = fopen("system_time.txt","w+");
    fwrite($system_time_file, $new_time);
    fclose($system_time_file);
}



function get_test_time()
{
    if(file_exists("system_test_time.txt"))
    {
        $system_test_time_file = fopen("system_test_time.txt", "r");
        $system_test_time = trim(fgets($system_test_time_file));
        fclose($system_test_time_file);
    }
    else
    {
        $system_test_time_file = fopen("system_test_time.txt","w");
        $system_test_time = strtotime(date("Y-m-d").", 10:55:01");
        fwrite($system_test_time_file, $system_test_time);
        fclose($system_test_time_file);
    } 
    return $system_test_time;
}

function update_test_time($add_time_for_each_post)
{
    $itime = intval(get_test_time()) + intval($add_time_for_each_post);
    $system_test_time_file = fopen("system_test_time.txt","w");
    fwrite($system_test_time_file, $itime);
    fclose($system_test_time_file);
}


/*
 * Check if $time surpases the last full minute $system_time
 * O trading system possui um tempo próprio, $home_broker_info["system_time"].
 * Um full minute é detectado sempre que o trading system for invocado e o tempo corrente 
 * tiver ultrapassado o minuto do tempo mantido pelo trading system, $home_broker_info["system_time"].
 * Quando isso ocorrer, e apenas quando isso ocorrer, o tempo do trading system é atualizado com o
 * tempo atual.
 */
function full_minute() 
{
    global $home_broker_info;
    global $full_minute;

    $now_minute = $home_broker_info["now_time"]  - ($home_broker_info["now_time"] % 60);
    $system_time_minute = $home_broker_info["system_time"] - ($home_broker_info["system_time"] % 60);
    if ($now_minute > $system_time_minute)
    {
        $home_broker_info["system_time"] = $now_minute;
        set_system_time($home_broker_info["system_time"]);
        
        write_log("[state_and_time.php] [full_minute]: true ".date('G:i:s',$home_broker_info["system_time"]));
        
        $full_minute = true;
        return true;
    }
    else
    {
        $full_minute = false;
        return false;
    }
}


/*
 * Check if $sample_size system minutes have elapsed
 */
function full_sample() 
{
    global $home_broker_info;
    global $full_sample;
    global $sample_size;
    
    $system_minute = ($home_broker_info["now_time"] + 60) / 60;
    if ($system_minute % $sample_size == 0)
    {
        write_log("[state_and_time.php] [full_sample]: true " .  date('G:i:s', $system_minute * 60));
        $full_sample = true;
        return true;
    }
    else
    {
        write_log("[state_and_time.php] [full_sample]: false " .  date('G:i:s', $system_minute * 60));
        $full_sample = false;
        return false;
    }
}


/*
 * Check if $sample_size system minutes have elapsed
 */
function even_full_sample() 
{
    global $home_broker_info;
    global $sample_size;
    
    $system_minute = ($home_broker_info["now_time"]) / 60;
    if ($system_minute % $sample_size == 0)
    {
        write_log("[state_and_time.php] [even_full_sample]: true " .  date('G:i:s', $system_minute * 60));
        return true;
    }
    else
    {
        write_log("[state_and_time.php] [even_full_sample]: false " .  date('G:i:s', $system_minute * 60));
        return false;
    }
}


function timeout_to_buy()
{
    global $home_broker_info;
    global $buy_order_file_name;
    global $time_out_to_buy;

    $buy_order_file = fopen($buy_order_file_name, "r");
    $last_buy_order = fgets($buy_order_file);
    fclose($buy_order_file);

    $last_buy_order_fields = explode(" ", $last_buy_order);
    if ($home_broker_info["now_time"] - $last_buy_order_fields[1] > $time_out_to_buy)
    {
        write_log("[state_and_time.php] [timeout_to_buy]: true. " . 
                         date("G:i:s", $home_broker_info["now_time"]) . " - " .
                         date("G:i:s", $last_buy_order_fields[1]) . " > " .
                         $time_out_to_buy . "s");
        unlink("buy_order.txt");
        return true;
    }
    else
        return false;  
}

function timeout_to_sell()
{
    global $sell_order_file_name;
    global $home_broker_info;
    global $time_out_to_sell;

    $sell_order_file = fopen($sell_order_file_name, "r");
    $last_sell_order_fields = explode(" ", fgets($sell_order_file));
    fclose($sell_order_file);
    
    if (($home_broker_info["now_time"] - $last_sell_order_fields[1]) > $time_out_to_sell)
    {
        write_log("[state_and_time.php] [timeout_to_sell]: true. " . 
                         date("G:i:s", $home_broker_info["now_time"]) . " - " .
                         date("G:i:s", $last_sell_order_fields[1]) . " > " .
                         $time_out_to_sell . "s");
        return true;
    }
    else
        return false; 
}

?>
