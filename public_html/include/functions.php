<?php
	require_once 'include/global.php';
	
	function raw_rpc($f, $a) {
		$request = xmlrpc_encode_request($f, $a);
		$header[] = "Content-type: text/xml";
		$header[] = "Content-length: " . strlen($request);

		$curl = curl_init(PNS_XMLRPC_SERVER_ADRESS); 
			$headers = array();
			curl_setopt($curl, CURLOPT_PORT, PNS_XMLRPC_SERVER_PORT); 
			curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
			curl_setopt($curl, CURLOPT_TIMEOUT, 1);
			curl_setopt($curl, CURLOPT_HTTPHEADER, $header);
			curl_setopt($curl, CURLOPT_POSTFIELDS, $request);
		$result = trim(curl_exec($curl));

		return $result;
	}

	function rpc($f, $a) {
		$r = xmlrpc_decode(raw_rpc($f, $a));
		return json_encode($r);
	}

?>