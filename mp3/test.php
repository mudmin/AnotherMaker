<?php
$mp3 = htmlentities($_GET['sound']); //obviously not right, but dangerous?
$player = 'cmdmp3.exe';

if(file_exists("sounds/$mp3.mp3")){
    shell_exec($player.' sounds/'.$mp3.'.mp3');
    die();
}else{
  die();
}
