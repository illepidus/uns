<?php
	header ("Content-Type:text/xml");

	$scandir = scandir('./');
	$files = array();
	foreach($scandir as $file) {
		if ((preg_match('/^.*\.xml$/', $file, $matches)) || ($file == "uns_log.xsl"))
		array_push ($files, array($file, filemtime($file)));
	}
?>
<?xml version="1.0" encoding="UTF-8" ?>
<INDEX>
	<FILES>
<?php
	foreach ($files as $file) {
		echo "\t\t<FILE mtime=\"" . $file[1] . "\">" . $file[0] . "</FILE>\n";
	}
?>
	</FILES>
</INDEX>