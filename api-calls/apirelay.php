<?php
header('Content-Type: application/json');
$ch = curl_init();
$curlConfig = array(
    CURLOPT_URL            => "https://api.openweathermap.org/data/2.5/forecast?id=5128581&MyAPIKeyHere&units=imperial",
    CURLOPT_POST           => true,
    CURLOPT_RETURNTRANSFER => true,
);
curl_setopt_array($ch, $curlConfig);
$result = curl_exec($ch);
$result = json_decode($result);
$code = $result->cod;

if($code == 200){
  $msg['temp'] = number_format($result->list[0]->main->temp,2);
  $msg['humidity'] = $result->list[0]->main->humidity;
  $msg['weather'] = ucwords($result->list[0]->weather[0]->description);
  curl_close($ch);
  echo json_encode($msg);
}
