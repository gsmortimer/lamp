<?php
$lamp = $_GET["lamp"];
readfile("lamp.txt");
if ($lamp == "on") {
  $file = fopen("lamp.txt", "w") or die("Unable to open file!");
  fwrite($file, "on\n");
  fclose($file);
}
if ($lamp == "off") {
  $file = fopen("lamp.txt", "w") or die("Unable to open file!");
  fwrite($file, "off\n");
  fclose($file);
}
?>
