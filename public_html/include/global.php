<?php
	define('UNS_DATA_PATH', '/etc/uns_data');
	define('UNS_SETTINGS_PATH', '/etc/uns_settings');
	define('UNS_DEBUG_LOG_PATH', '/var/log/uns.log');
	define('UNS_PROCESS_ID_COMMAND', 'pgrep -x "uns"');
	define('UNS_DEBUG_LOG_COMMAND', 'tac  /var/log/uns.log  | head -n1000');
	define('PNS_XMLRPC_SERVER_ADRESS', 'localhost');
	define('PNS_XMLRPC_SERVER_PORT', '9999');
?>