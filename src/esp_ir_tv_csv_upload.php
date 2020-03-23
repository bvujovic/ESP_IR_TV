<?php
//* PHP skripta koja se nalazi na kingtrader.info/php koja prihvata i cuva .csv fajlove poslate sa ESP-a

// POST: fileName=tags.csv&txt=azuki\r\nbleee
$fileName = filter_input(INPUT_POST, 'fileName', FILTER_SANITIZE_STRING);
$txt = filter_input(INPUT_POST, 'txt', FILTER_SANITIZE_STRING);

//T file_put_contents("test.txt", $txt);
if (strlen($txt) > 10000000)
    file_put_contents("upload_csv.log", "Sadrzaj .csv fajla je predugacak.");
elseif (strlen($fileName) > 20)
    file_put_contents("upload_csv.log", "Naziv .csv fajla je predugacak.");
elseif (strpos($fileName, ' ') !== false)
    file_put_contents("upload_csv.log", "Naziv .csv fajla sadrzi space.");
elseif (strpos($fileName, "\t") !== false)
    file_put_contents("upload_csv.log", "Naziv .csv fajla sadrzi tab.");
elseif (strpos($fileName, '.') === false)
    file_put_contents("upload_csv.log", "Naziv .csv fajla ne sadrzi tacku.");
else
    file_put_contents($fileName, urldecode($txt));
