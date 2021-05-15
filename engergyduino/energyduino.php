<?php
//This file expects to be in the root of a userspice install.
//Otherwise, you will have to modify the paths and database calls for your own needs
require_once 'users/init.php';
$db = DB::getInstance();

//Grab and sanitize the data from the url
$hum = Input::get('hum');
$temp = Input::get('temp');

if(is_numeric($hum) && is_numeric($temp)){
//I'm putting a little extra load on this API in order to create the table for you.
//You're welcome :)
//try to create table if it doesn't exist
$check = $db->query("SELECT id FROM energyduino LIMIT 1")->count();
if($check < 1){
  $db->query("
  CREATE TABLE `energyduino` (
   `id` int(11) UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
    `temperature` decimal(11,2) NOT NULL,
    `humidity` decimal(11,2) NOT NULL,
    `ts` timestamp NOT NULL DEFAULT current_timestamp() ON UPDATE current_timestamp()
  ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
  ");
}


$fields = [
  'humidity'=>$hum,
  'temperature'=>$temp
];
$db->insert("energyduino",$fields);
  echo "success";
}else{
  echo "fail";
}
