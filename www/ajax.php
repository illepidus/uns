<?php
	require_once 'include/global.php';
	require_once 'include/functions.php';

	if ($_REQUEST['do'] == "get_uns_data") {
		if (exec(UNS_PROCESS_ID_COMMAND) > 0)
			echo json_encode(parse_ini_file(UNS_DATA_PATH, true));
		else echo('{"SERVER_NOT_RUNNING":true}');
	}

	if ($_REQUEST['do'] == "get_raw_uns_data") {
		header("Content-Type:text/plain");
		$data = @file_get_contents(UNS_DATA_PATH);
		if ($data) echo $data;
	}

	if ($_REQUEST['do'] == "get_uns_debug_log") {
		header("Content-Type:text/plain");
		system(UNS_DEBUG_LOG_COMMAND);
	}

	if ($_REQUEST['do'] == "get_uns_settings")
		echo json_encode(parse_ini_file(UNS_SETTINGS_PATH, true));

	if ($_REQUEST['do'] == "get_raw_uns_settings") {
		header("Content-Type:text/plain");
		$data = @file_get_contents(UNS_SETTINGS_PATH);
		if ($data) echo $data;
	}

	if ($_REQUEST['do'] == "get_uns_status")
		echo ((exec(UNS_PROCESS_ID_COMMAND) > 0)? 1 : 0);

	if ($_REQUEST['do'] == "get_today_stats") {

	}
	
	else if (($_REQUEST['do'] == "rpc") && isset($_REQUEST['function'])) {
		if (!isset($_REQUEST['args']))
			echo rpc($_REQUEST['function']);
		else
			echo rpc($_REQUEST['function'], json_decode($_REQUEST['args']));
	}
	
	if (($_REQUEST['do'] == 'test')) {
		echo raw_rpc('setBalCode', [
			'CODE' => '50A81EE1234C19C3206668', //50|T|YY|NNNNNN|P|AAA|GG|VV|MMM
			'POST' => 1,
			'CHAN' => 'www'
		]);
	}
?>
