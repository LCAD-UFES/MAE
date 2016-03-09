<?php

/*
 * Trading System State Machine
 */
function trading_system()
{
    global $home_broker_page;
    global $home_broker_info;
    global $full_minute;
    global $full_sample;
    global $just_collecting_data;
    global $trading_time_interval;
    global $stop_loss;
    global $stop_gain;
    global $error_message;
    global $ignore_trading_time_interval;

    $home_broker_file = fopen("home_broker_data.txt", "w");
    fwrite($home_broker_file, $home_broker_page);
    fclose($home_broker_file);


    if (current_state("SYNCHRONIZE"))
    {
        if (even_full_sample() && (trading_time_interval() || $ignore_trading_time_interval))
            change_state("SYNCHRONIZE2");
    }
    
    if (current_state("SYNCHRONIZE2"))
    {
        if (full_minute())
            change_state("SAVE_DATA0");
    }
 
    if (current_state("IDLE"))
    {
        if (full_minute())
            change_state("SAVE_DATA");
        else
            change_state("STOP_GAIN_OR_LOSS");
    }

    if (current_state("SAVE_DATA0"))
    {
        add_data_to_database();
        
        if (full_sample())
            change_state("IDLE");
        else
            change_state("SYNCHRONIZE2");
    }
 
    if (current_state("SAVE_DATA"))
    {
        add_data_to_database();
        
        if (full_sample())
            change_state("SELL_AND_PREDICT");
        else
            change_state("STOP_GAIN_OR_LOSS");
    }

    if (current_state("STOP_GAIN_OR_LOSS"))
    {
        if (stop_loss($stop_loss) || stop_gain($stop_gain))
            change_state("SELL_BY_BOOK");
        else
            change_state("IDLE");
    }
    
    if (current_state("WAIT_PREDICTION"))
    {
        if (predict_step_finished())
            change_state("WAIT_SELL");
    }

    if (current_state("SELL_AND_PREDICT"))
    {
        send_order_to_sell_last_stock_bought();
        make_preditions_and_train_with_new_data();
        change_state("WAIT_PREDICTION");
    }

    if (current_state("WAIT_SELL"))
    {
        $sold = false;
        $time_out_to_sell = false;

        get_sell_state_conditions($sold, $time_out_to_sell, $trading_time_interval);
        if (($sold || $just_collecting_data) && ($trading_time_interval || $ignore_trading_time_interval))
            change_state("BUY");
        else if (!$sold && !$time_out_to_sell && full_minute())
            change_state("SAVE_DATA2");
        else if (!$sold && $time_out_to_sell)
            change_state("CANCEL_ORDER");
        else if (($sold || $just_collecting_data) && !$trading_time_interval)
            change_state("SYNCHRONIZE");
    }

    if (current_state("SAVE_DATA2"))
    {
        add_data_to_database();
        change_state("WAIT_SELL");
    }

    if (current_state("WAIT_BUY"))
    {
        $bought = false;
        $time_out_to_buy = false;
        
        get_buy_state_conditions($bought, $time_out_to_buy);
        if ($bought || $just_collecting_data)
            change_state("IDLE");
        else if (!$bought && !$time_out_to_buy && full_minute())
            change_state("SAVE_DATA4");
        else if (!$bought && $time_out_to_buy)
            change_state("CANCEL_ORDER2");
    }

    if (current_state("SAVE_DATA4"))
    {
        add_data_to_database();
        change_state("WAIT_BUY");
    }
    
    if (current_state("WAIT_CANCEL_ORDER2"))
    {
        $canceled = cancel_order_finished();
        if (!$canceled && full_minute())
            change_state("SAVE_DATA5");
        else if ($canceled)
            change_state("IDLE");
    }

    if (current_state("SAVE_DATA5"))
    {
        add_data_to_database();
        change_state("WAIT_CANCEL_ORDER2");
    }
    
    if (current_state("CANCEL_ORDER2"))
    {
        send_cancel_order();
        change_state("WAIT_CANCEL_ORDER2");
    }

    if (current_state("BUY"))
    {
        if (send_buy_order_suggested_by_trading_system($just_collecting_data))
            change_state("WAIT_BUY");
        else # not enough capital to buy the suggested stock
            change_state("IDLE");
    }
    
    if (current_state("WAIT_CANCEL_ORDER"))
    {
        $canceled = cancel_order_finished();
        if (!$canceled && full_minute())
            change_state("SAVE_DATA3");
        else if ($canceled)
            change_state("SELL_BY_BOOK");
    }

    if (current_state("SAVE_DATA3"))
    {
        add_data_to_database();
        change_state("WAIT_CANCEL_ORDER");
    }
    
    if (current_state("WAIT_SELL_BY_BOOK"))
    {
        $sold = false;
        $time_out_to_sell = false;
        get_sell_state_conditions($sold, $time_out_to_sell, $trading_time_interval);
        if (($sold || $just_collecting_date) && ($trading_time_interval || $ignore_trading_time_interval))
            change_state("IDLE");
        else if (($sold || $just_collecting_data) && !$trading_time_interval)
            change_state("SYNCHRONIZE");
        else if (!$sold && $time_out_to_sell)
            change_state("CANCEL_ORDER");
    }

    if (current_state("CANCEL_ORDER"))
    {
        send_cancel_order();
        change_state("WAIT_CANCEL_ORDER");
    }

    if (current_state("SELL_BY_BOOK"))
    {
        send_order_to_sell_by_book();
        change_state("WAIT_SELL_BY_BOOK");
    }
}
?>
