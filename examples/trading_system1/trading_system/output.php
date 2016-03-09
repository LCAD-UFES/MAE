<?php
header("Cache-Control: no-cache, must-revalidate"); 
header("Expires: Sat, 26 Jul 1997 05:00:00 GMT"); 
?>
<html>
<head>
<script type="text/javascript" src="jquery.js"></script>
<?php
$filename = 'input.txt';
if (is_readable($filename)) {
    $status = "ok";
    $file = fopen($filename,"r");
    $data = fgets($file);
    $data = explode(';',$data);
    fclose($file);
} else {
    $status = "nok";
}
?>
</head>
<body>
<table>
<tr>
    <td>0: </td>
    <td><input type="text" style="width:100px;height:30px;" value="<?echo $status; ?>"/><td>
</tr>

<tr>
    <td>1: </td>
    <td><input type="text" style="width:100px;height:30px;" value="<?echo $data[0]; ?>"/><td>
</tr>
<tr>
    <td>2: </td>
    <td><input type="text" style="width:100px;height:30px;" value="<?echo $data[1]; ?>"/><td>
</tr>
<tr>
    <td>3: </td>
    <td><input type="text" style="width:100px;height:30px;" value="<?echo $data[2]; ?>"/><td>
</tr>
<tr>
    <td>4: </td>
    <td><input type="text" style="width:100px;height:30px;" value="<?echo $data[3]; ?>"/><td>
</tr>
<tr>
    <td>5: </td>
    <td><input type="text" style="width:100px;height:30px;" value="<?echo $data[4]; ?>"/><td>
</tr>
<tr>
    <td>6: </td>
    <td><input type="text" style="width:100px;height:30px;" value="<?echo $data[5]; ?>"/><td>
</tr>
</body>
</html>
