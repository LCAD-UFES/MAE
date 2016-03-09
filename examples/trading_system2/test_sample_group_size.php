<?php

$stock_quotes_file = fopen($argv[1], "r");
$num_sample_groups = 0;
$average_num_samples_per_group = 0;

$line = fgets($stock_quotes_file);
while (!feof($stock_quotes_file))
{
    $line_fields1 = explode(" ", $line);
    $line = fgets($stock_quotes_file);
    $line_fields2 = explode(" ", $line);
    $num_samples = 2;
    while (!feof($stock_quotes_file) && (percentage_variation($line_fields1[1], $line_fields2[1]) < $argv[2]))
    {
        $num_samples += 1;
        $line = fgets($stock_quotes_file);
        $line_fields2 = explode(" ", $line);        
    }
    if (!feof($stock_quotes_file))
    {
        $num_sample_groups += 1;
        $average_num_samples_per_group += $num_samples;
        $line = fgets($stock_quotes_file);
    }
}
if ($num_sample_groups > 0)
{
    $average_num_samples_per_group /= $num_sample_groups;
    print $average_num_samples_per_group . "\n";
}
else
    print 0 . "\n";

fclose($stock_quotes_file);

function percentage_variation($first_quote, $last_quote)
{
    return abs(($last_quote - $first_quote) / $first_quote);
}

?>
