<?php
// Sample MP3 player API by AnotherMaker
// Here's a video demo of the mp3 player https://www.youtube.com/watch?v=PJ2Ag5xLjIU
// Sound effects obtained from https://www.zapsplat.com
// cdmp3.exe is an open source mp3 player for windows, written in C.
// Git Repo: https://github.com/jimlawless/cmdmp3

//grab the $_GET variable and do some basic sanitization
$secret = htmlentities($_GET['secret']);

//figure out the OS so this will work on all 3 major operating systems
$os = strtoupper(substr(PHP_OS, 0, 3));

//Set your MP3 player based on your operating system. Note in linux you will need to install mpg123
// debian/ubuntu - sudo apt-get install mpg123
// centos/rhel - yum install mpg123

if ($os  === 'WIN') {
  $player = 'cmdmp3.exe';
}elseif($os === 'DAR') { //MacOs
  $player = 'afplay';
}else{
  $player = 'mpg123';
}

//IMPORTANT NOTE: By far, by far, by far the BEST way to do this is to either use a switch statement or if/eles statements as below
//to explicitly link a GET request to a specific mp3 file
//in this example, if you hit the secret button then 24,25,26,27 you get the coin 1 sound. Otherwise, you get coin 2
if($secret == "24252627"){
    shell_exec($player.' sounds/coin1.mp3');
    die();
}else{
  shell_exec($player.' sounds/coin2.mp3');
  die();
}

//If you are on a closed network and you really want to just grab the file name from GET, that's fine, but please consider using the above
//option.

// //Or let 'er rip with checking if the file exists
// if(file_exists("sounds/$secret.mp3")){
//   $escaped = substr(escapeshellarg(' sounds/'.$secret.'.mp3'),1,-1);
//     shell_exec($player.$escaped);
//     die();
// }else{
//   die();
// }
