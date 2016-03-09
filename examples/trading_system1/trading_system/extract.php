<?php


$last_prices = create_dictonary("stockCodeList.txt");
print_r($last_prices);
print timestamp_to_hourdate(1286021991)."\n";

/*
 * Main loop
 */
/*
while(true) {
    $file = check_new_file('data');  
     if($file != null) {
        process_file($file_data);
        check_stop_loss($last_prices);
        check_stop_gain($last_prices);
        check_new_prediction($last_prices);
        
    }
    sleep(2);
}
*/

/*
 * Process file
 */
function process_file($file) {
    $file_data = file_get_contents("data/".$file);
    /*
     * Inserir lógica de processamento
     * (1) Preencher a dicionário com o preço e minuto
     * (2) Verificar se as ordem de venda ou compra foi executada, 
     *     será que vai dar para fazer pelo arquivo?
     * (Criei a função timestamp_to_datehour() que pega o minuto)
     * Podemos usar o nome do arquivo para pegar a hora
     */
    unlink("data/".$file);
}


/*
 * Create dictonary of stocks
 */
function create_dictonary($file) {
    $file = fopen("stockCodeList.txt", "r");
    while(!feof($file)) {
        $name_stock = fgets($file);        
        $name_stock = explode('.', $name_stock);
        $last_prices [''.$name_stock[0].''] = array(null,null);
    }
    fclose($file);
    return $last_prices;
}

/*
 * Check new file
 */
function check_new_file($dir) {
    if ($handle = opendir($dir)) {
        while (false !== ($file = readdir($handle))) {
            if ($file != "." && $file != "..") {
                return $file;
            }
        }
        closedir($handle);
    }
    return null;
}

/*
 * Timestmap to date
 */
function timestamp_to_hourdate($timestamp) {
    return date("G:i:s", $timestamp);
}

?>
