<!DOCTYPE html>
<html>
	<head>
		<meta charset="UTF-8">
		<title>Логи УНС</title>
		<link rel="icon" href="favicon.ico" type="image/x-icon">
		<style type="text/css">
		.month { 
			color: #800;
			margin-top: 200px;
			font-size: 120%;
			font-weight: bold;
		}

		.empty_date {
			color: #999;
		}
		a {
			color: #00F;
			text-decoration: none;
		}

		a:visited {
			color: #00F;
		}
		</style>
	</head>
	<body style="margin: 0; padding: 5px; color: #000; background: #FFF;">
		<table id="layout" style="border: none; border-collapse: collapse; table-layout: fixed; width: 250px; margin: auto">
		<?php
			$russian_mounths = array('Январь', 'Февраль', 'Март', 'Апрель', 'Май', 'Июнь', 'Июль', 'Август', 'Сентябрь', 'Октябрь', 'Ноябрь', 'Декабрь', 'Январь');
			$scandir = scandir('./');
			$files = array();
			foreach($scandir as $file) {
				if (preg_match('/^.*(\d\d)(\d\d)(\d\d)\.xml$/', $file, $matches))
				$files[$matches[1].$matches[2].$matches[3]] = $file;
			}

			$date = new DateTime();
			$start_date = new DateTime(); $start_date->setDate(2017, 6, 1);

			while ($date >= $start_date) {
				$date->setDate($date->format('Y'), $date->format('m'), 1);
				$y  = $date->format('Y');
				$y2 = $date->format('y');
				$m  = $date->format('n');
				$m2 = $date->format('m');
				$d  = $date->format('j');
				$w  = $date->format('w'); $w = ($w > 0) ? ($w - 1) : 6;
				$dm = cal_days_in_month(CAL_GREGORIAN, $m, $y);

				echo('<tr class="month"><td align="center" colspan="7">'. $russian_mounths[$m - 1] . " $y</td></tr>");
				echo('<tr><td>Пн</td><td>Вт</td><td>Ср</td><td>Чт</td><td>Пт</td><td>Сб</td><td>Вс</td></tr>');
				$month = array();
				for ($i = 0; $i < $w; $i++) {
					$month[] = NULL;
				}
				while ($d <= $dm) {
					$month[] = ($d > 9) ? $d : ('0' . $d);
					$d++;
				}

				for ($i = 0; $i < 6; $i++) {
					echo('<tr>');
					for ($j = 0; $j < 7; $j++) {
						if (!$month[$i*7 + $j]) {
							echo('<td></td>');
						}
						else {
							if ($files[$y2 . $m2 . $month[$i*7 + $j]]) {
								echo('<td><a href="' . $files[$y2 . $m2 . $month[$i*7 + $j]] . '">' . $month[$i*7 + $j] . '</a></td>');
							}
							else {
								echo('<td class="empty_date">' . $month[$i*7 + $j] . '</td>');
							}
						}
					}
					echo('</tr>');
				}
				echo('<tr><td colspan=7 height="50px"></td></tr>');
				$date->sub(new DateInterval('P1M'));
			}
		?>
		</table>
	</body>
</html>
