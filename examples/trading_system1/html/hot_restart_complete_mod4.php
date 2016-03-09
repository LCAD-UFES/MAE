<?php

$filename = $_SERVER['argv'][1];
$file_contents = explode("\n",file_get_contents($filename));

$number_lines = count($file_contents) - 1;
$number_to_complete = 4 - (($number_lines) % 4);

if($number_to_complete != 4)
{
    for ($i = 0; $i < $number_to_complete; $i++)
    {
         $file_contents[$number_lines + $i] = $file_contents[$number_lines + $i - 1]; 
    }
}
$file_output = fopen("hot_restart_php.tmp.txt", "w+");
$data_output = implode("\n",$file_contents)."\n";
fwrite($file_output, $data_output);
fclose($file_output);

?>
