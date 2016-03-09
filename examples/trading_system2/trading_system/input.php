<?php
if(isset($_POST["input"])) {
   $filename = "data/".time().".txt";
   $handle = fopen($filename, "w+");
   $numbytes = fwrite($handle, $_POST["input"]);
   fclose($handle);
}
?>

<html>
<body>
<form name="inputform" method="post" action="input.php">
<textarea style="height:200px; width:400px" name="input">
</textarea>
<br>
<button style="height:50px; width:400px">send</button>
</form>
</body>
</html>
