 <!DOCTYPE html>
<html>
	<head>
		<meta charset="UTF-8">
		<title>УНС</title>
		<link rel="icon" href="favicon.ico" type="image/x-icon">
		<link href="css/main.css" type="text/css" rel="stylesheet" />
		<script type="text/javascript" src="js/jquery.js"></script>
		<script type="text/javascript" src="js/main.js"></script>
	</head>
	<body>
		<div id="links">
			<a href="/log/">Логи XML</a>
			<a href="/doc/commands.txt">Список команд</a>
			<a href="/doc/">Документы</a>
<?php
		if ($_SERVER['REMOTE_USER'] == 'illepidus') echo('
			<a href="ajax.php?do=get_uns_debug_log">Отладочный лог</a>
			<a href="ajax.php?do=get_raw_uns_settings">Настройки</a>
			<a href="ajax.php?do=get_raw_uns_data">Данные</a>
		');
?>

		</div>
		<div id="devices">
			<div id="station">
				<table>
					<tr>
						<td class="device_name" colspan="2">УНС</td>
					</tr>
					<tr>
						<td>Состояние: </td>
						<td id="station_state" class="ajax_data"></td>
					</tr>
					<tr>
						<td>Активный насос: </td>
						<td id="active_pump" class="ajax_data"></td>
					</tr>
				</table>
			</div>
			<div id="filling_points">
				<div class="filling_point" id="point_1">
					<table>
						<tr>
							<td class="device_name" colspan="2">Пост #1</td>
						</tr>
						<tr>
							<td>Состояние: </td>
							<td id="point_1_state" class="ajax_data"></td>
						</tr>
						<tr>
							<td>Текущий вес: </td>
							<td id="point_1_mass" class="ajax_data"></td>
						</tr>
						<tr>
							<td>Текущая мода: </td>
							<td id="point_1_tank_type" class="ajax_data"></td>
						</tr>
						<tr>
							<td>Код баллона: </td>
							<td id="point_1_bal_code" class="ajax_data"></td>
						</tr>
						<tr>
							<td id="point_1_bal_code_result"></td>
							<td><input id="point_1_bal_code_input" class="code_input" type="text" size="22"></td>
						</tr>
					</table>
				</div>
				<div class="filling_point" id="point_2">
					<table>
						<tr>
							<td class="device_name" colspan="2">Пост #2</td>
						</tr>
						<tr>
							<td>Состояние: </td>
							<td id="point_2_state" class="ajax_data"></td>
						</tr>
						<tr>
							<td>Текущий вес: </td>
							<td id="point_2_mass" class="ajax_data"></td>
						</tr>
						<tr>
							<td>Текущая мода: </td>
							<td id="point_2_tank_type" class="ajax_data"></td>
						</tr>
						<tr>
							<td>Код баллона: </td>
							<td id="point_2_bal_code" class="ajax_data"></td>
						</tr>
						<tr>
							<td id="point_2_bal_code_result"></td>
							<td><input id="point_2_bal_code_input" class="code_input" type="text" size="22"></td>
						</tr>
					</table>
				</div>
			</div>
			<div class="spacer-div"></div>
			<div id="tanks">
				<div class="tank" id="tank_1">
					<table>
						<tr>
							<td class="device_name" colspan="2">Ёмкость 1</td>
						</tr>
						<tr>
							<td>Состояние насоса: </td>
							<td id="pump_1_state" class="ajax_data"></td>
						</tr>
						<tr>
						<tr>
							<td>Вес газа: </td>
							<td id="tank_1_gas_mass" class="ajax_data"></td>
						</tr>
					</table>
				</div>
				<div class="tank" id="tank_2">
					<table>
						<tr>
							<td class="device_name" colspan="2">Ёмкость 2</td>
						</tr>
						<tr>
							<td>Состояние насоса: </td>
							<td id="pump_2_state" class="ajax_data"></td>
						</tr>
						<tr>
							<td>Вес газа: </td>
							<td id="tank_2_gas_mass" class="ajax_data"></td>
						</tr>
					</table>
				</div>
			</div>
		</div>
	</body>
</html> 