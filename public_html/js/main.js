function getStateName(s)
{
	if (s ==  0) return "ВЫКЛ";
	if (s ==  1) return "ВКЛ";
	if (s ==  2) return "НЕИЗВЕСТНО";
	if (s ==  3) return "ОШИБКА";
	if (s ==  4) return "ЗАВИС";
	if (s ==  5) return "ОЖИДАНИЕ";
	if (s ==  6) return "ВЕРХ";
	if (s ==  7) return "НИЗ";
	if (s ==  8) return "ПУСТО";
	if (s ==  9) return "ИДЕТ РАБОТА";
	if (s == 10) return "СНАРЯЖЕН";
	if (s == 11) return "ГОТОВ";
	if (s == 12) return "ПЕРЕВЕС";
	if (s == 13) return "ОСТАНОВЛЕН";
	return "UNDEFINED_STATE";
}

function loadSettings() {
	$.getJSON('ajax.php?do=get_uns_settings',
	function(data) {
		updateData();
		setInterval('updateData();', 1000);
	});
}

function updateData() {
	$.getJSON('ajax.php?do=get_uns_data',
	function(data) {
		if (data['SERVER_NOT_RUNNING']) {
			$('.ajax_data').addClass('server_down');
			$('.ajax_data').removeAttr('data-state');
			$('.ajax_data').html('НЕИЗВЕСТНО');
			$('#station_state').html(getStateName(0)); $('#station_state').attr('data-state', 0);
			return;
		}
		else {
			$('.ajax_data').removeClass('server_down');
			try {
				$('#station_state').html(getStateName(data['UNS']['state']));
				$('#station_state').attr('data-state', data['UNS']['state']);
			} catch(err){};
			try {
				$('#active_pump').html(data['UNS']['active_pump']);
			} catch(err){};

			try {
				$('#point_1_state').html(getStateName(data['POINT_1']['state']));
				$('#point_1_state').attr('data-state', data['POINT_1']['state']);
			} catch(err){};
			try {
				$('#point_2_state').html(getStateName(data['POINT_2']['state']));
				$('#point_2_state').attr('data-state', data['POINT_2']['state']);
			} catch(err){};

			try {
				$('#point_1_mass').html((data['UNS_UNIT_1']['mass'] - data['UNS_UNIT_1']['tare']).toFixed(2) + " [кг]");
			} catch(err){};
			try {
				$('#point_2_mass').html((data['UNS_UNIT_2']['mass'] - data['UNS_UNIT_2']['tare']).toFixed(2) + " [кг]");
			} catch(err){};

			try {
				$('#point_1_tank_type').html(data['POINT_1']['tank_type']);
			} catch(err){};
			try {
				$('#point_2_tank_type').html(data['POINT_2']['tank_type']);
			} catch(err){};
			
			try {
				$('#point_1_bal_code').html('<span class="bal_code">' + data['POINT_1']['bal_code'] + '</span>');
			} catch(err){};
			try {
				$('#point_2_bal_code').html('<span class="bal_code">' + data['POINT_2']['bal_code'] + '</span>');
			} catch(err){};

			try {
				$('#pump_1_state').html(getStateName(data['PUMP_1']['state']));
				$('#pump_1_state').attr('data-state', data['PUMP_1']['state']);
			} catch(err){};
			try {
				$('#pump_2_state').html(getStateName(data['PUMP_2']['state']));
				$('#pump_2_state').attr('data-state', data['PUMP_2']['state']);
			} catch(err){};

			/*
			try {
				$('#sb_1_state').html(getStateName(data['SB_UNIT_1']['state']));
				$('#sb_1_state').attr('data-state', data['SB_UNIT_1']['state']);
			} catch(err){};
			try {
				$('#sb_2_state').html(getStateName(data['SB_UNIT_2']['state']));
				$('#sb_2_state').attr('data-state', data['SB_UNIT_2']['state']);
			} catch(err){};
			*/

			try {
				$('#tank_1_gas_mass').html((data['SB_UNIT_1']['mass'] - data['SB_UNIT_1']['tare']).toFixed(2) + " [кг]");
			} catch(err){};
			try {
				$('#tank_2_gas_mass').html((data['SB_UNIT_2']['mass'] - data['SB_UNIT_2']['tare']).toFixed(2) + " [кг]");
			} catch(err){};
}
});
}

$(document).ready(function () {
	loadSettings();
	$('#point_1_bal_code_input').keypress(function (e) {
		if (e.which == 13) {
			$('#point_1_bal_code_result').html(".....................");
			$.ajax({
				url: 'ajax.php?do=rpc&function=setBalCode&args=' + JSON.stringify({POST: 1, CHAN: 'www', CODE: $(this).val()})
			})
			.done(function(result) {
				if (result == 0)
					$('#point_1_bal_code_result').html('ОК');
				else
					$('#point_1_bal_code_result').html('Ошибка ' + result);
				updateData();
			});
		return false;
	  }
	});
	$('#point_2_bal_code_input').keypress(function (e) {
		if (e.which == 13) {
			$('#point_2_bal_code_result').html(".....................");
			$.ajax({
				url: 'ajax.php?do=rpc&function=setBalCode&args=' + JSON.stringify({POST: 2, CHAN: 'www', CODE: $(this).val()})
			})
			.done(function(result) {
				if (result == 0)
					$('#point_2_bal_code_result').html('ОК');
				else
					$('#point_2_bal_code_result').html('Ошибка ' + result);
				updateData();
			});
		return false;
	  }
	});
});
