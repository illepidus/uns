<?xml version="1.0" encoding="UTF-8"?> 
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:decimal-format name="nan" NaN=""/>

<xsl:template match="/*">
<HTML>
<HEAD>
  <TITLE>Отчет</TITLE>
  <STYLE>
    @media print { *{font:normal 7pt Arial!important; color:#000!important;} }
    body { font:8pt Arial; margin:1pt; padding:0px; }
    p { text-align:center; font-weight:bold; }
    table { width: 100%; border-collapse: collapse; margin: 0pt; table-layout:fixed; }
    tr:hover { background-color:#FFC; }
    th { padding:0px 4px 0px 4px; border:1px solid #DDD; font:inherit; }
    td { padding:0px 4px 0px 4px; border:1px solid #DDD; color:#000;
      white-space:nowrap; overflow:hidden; }
    .rr { text-align:right; }
    .cc { text-align:center; }
    .bb { font-weight:bold; }
    .HOUR { font-weight:bold; background-color:#EEE; }
    .FILL { }
  </STYLE>
</HEAD>
<BODY>
  <P>УНС <xsl:value-of select="@date"/><BR/>
    (рабочий день с <xsl:value-of select="FILL[1]/@time"/> 
    до <xsl:value-of select="FILL[last()]/@time"/>)
  </P>
  <TABLE>
    <TR>
      <TH></TH>
      <TH>мода</TH>
      <TH colspan="3">описание</TH>
      <TH>кол-во,шт</TH>
      <TH>норма,кг</TH>
      <TH>расход,кг</TH>
      <TH>время,сек</TH>
      <TH>ёмкость 1</TH>
      <TH>ёмкость 2</TH>
    </TR>
    <TR class="HOUR">
      <TD class="cc"><xsl:value-of select="*[1]/@time"/></TD>
      <TD class="cc"></TD>
      <TD colspan="7"><xsl:value-of select="'Остаток на начало дня'"/></TD>
      <TD class="rr"><xsl:value-of select="format-number(*[1]/GAS1, '#', 'nan')"/></TD>
      <TD class="rr"><xsl:value-of select="format-number(*[1]/GAS2, '#', 'nan')"/></TD>
    </TR>
    <xsl:call-template name="sum"><xsl:with-param name="m" select="'B05'"/></xsl:call-template>
    <xsl:call-template name="sum"><xsl:with-param name="m" select="'B2'"/></xsl:call-template>
    <xsl:call-template name="sum"><xsl:with-param name="m" select="'B024'"/></xsl:call-template>
    <xsl:call-template name="sum"><xsl:with-param name="m" select="'B3'"/></xsl:call-template>
    <xsl:call-template name="sum"><xsl:with-param name="m" select="'B6'"/></xsl:call-template>
    <xsl:call-template name="sum"><xsl:with-param name="m" select="'B12'"/></xsl:call-template>
    <xsl:call-template name="sum"><xsl:with-param name="m" select="'B19'"/></xsl:call-template>
    <xsl:call-template name="sum"><xsl:with-param name="m" select="'B24'"/></xsl:call-template>
    <TR class="bb">
      <TD class="cc"></TD>
      <TD class="cc"></TD>
      <TD colspan="3"><xsl:value-of select="'Итого во всех модах'"/></TD>
      <TD class="rr"><xsl:value-of select="count(FILL)"/></TD>
      <TD class="rr"><xsl:value-of select="''"/></TD>
      <TD class="rr"><xsl:value-of select="format-number(sum(FILL/MASS) + count(FILL) * 0.3, '#', 'nan')"/></TD>
      <TD class="rr"><xsl:value-of select="format-number(sum(FILL/TIME), '#', 'nan')"/></TD>
      <TD/>
      <TD/>
    </TR>
    <TR class="bb">
      <TD class="cc"></TD>
      <TD class="cc"></TD>
      <TD colspan="7"><xsl:value-of select="'Итого расход/потери'"/></TD>
      <TD class="rr"><xsl:value-of select="format-number(*[1]/GAS1 - *[last()]/GAS1, '#', 'nan')"/></TD>
      <TD class="rr"><xsl:value-of select="format-number(*[1]/GAS2 - *[last()]/GAS2, '#', 'nan')"/></TD>
    </TR>
    <TR class="HOUR">
      <TD class="cc"><xsl:value-of select="*[last()]/@time"/></TD>
      <TD class="cc"></TD>
      <TD colspan="7"><xsl:value-of select="'Остаток на конец дня'"/></TD>
      <TD class="rr"><xsl:value-of select="format-number(*[last()]/GAS1, '#', 'nan')"/></TD>
      <TD class="rr"><xsl:value-of select="format-number(*[last()]/GAS2, '#', 'nan')"/></TD>
    </TR>
  </TABLE>
  <P>ДЕТАЛЬНЫЙ ОТЧЕТ</P>
  <TABLE>
    <TR>
      <TH>начало</TH>
      <TH>мода</TH>
      <TH colspan="3">описание</TH>
      <TH>масса,кг</TH>
      <TH>емкость</TH>
      <TH>нап.пост</TH>
      <TH>время,сек</TH>
      <TH>ёмкость 1</TH>
      <TH>ёмкость 2</TH>
    </TR>
    <xsl:apply-templates select="*"/>
  </TABLE>
</BODY>
</HTML>
</xsl:template>

<xsl:template match="*">
  <TR class="{name()}">
    <xsl:if test="self::HOUR">
      <TD class="cc"><xsl:value-of select="@time"/></TD>
      <TD class="cc"/>
      <TD colspan="7">
        <xsl:if test="following-sibling::*[1]/self::FILL">Исходно</xsl:if>
        <xsl:if test="not(following-sibling::*[1]/self::FILL)">Простой</xsl:if>
      </TD>
    </xsl:if>
    <xsl:if test="self::FILL">
      <TD class="cc"><xsl:value-of select="@time"/></TD>
      <TD class="cc"><xsl:value-of select="MODE"/></TD>
      <TD colspan="3"><xsl:call-template name="dsc"/></TD>
      <TD class="rr"><xsl:value-of select="format-number(MASS, '#.00', 'nan')"/></TD>
      <TD class="cc"><xsl:value-of select="TANK"/></TD>
      <TD class="cc"><xsl:value-of select="substring-after(POST, '_')"/></TD>
      <TD class="rr"><xsl:value-of select="TIME"/></TD>
    </xsl:if>
    <TD class="rr"><xsl:value-of select="format-number(GAS1, '#', 'nan')"/></TD>
    <TD class="rr"><xsl:value-of select="format-number(GAS2, '#', 'nan')"/></TD>
  </TR>
</xsl:template>

<xsl:template name="sum">
  <xsl:param name="m"/>
  <xsl:variable name="f" select="FILL[MODE = $m]"/>
  <xsl:variable name="n"><xsl:if test="$f"><xsl:value-of select="count($f)"/></xsl:if></xsl:variable>  
  <xsl:variable name="s"><xsl:call-template name="dsc"><xsl:with-param name="m" select="$m"/></xsl:call-template></xsl:variable>
  <xsl:variable name="d" select="number(substring-before($s, 'кг'))"/>
  <TR>
    <TD class="cc">всего</TD>
    <TD class="cc"><xsl:value-of select="$m"/></TD>
    <TD colspan="3"><xsl:value-of select="$s"/></TD>
    <TD class="rr"><xsl:value-of select="$n"/></TD>
    <TD class="rr"><xsl:value-of select="format-number($d * $n, '#', 'nan')"/></TD>
    <TD class="rr"><xsl:value-of select="format-number(sum($f/MASS) + $n * 0.3, '#', 'nan')"/></TD>
    <TD class="rr"><xsl:value-of select="format-number(sum($f/TIME) + $n * 0, '#', 'nan')"/></TD>
    <TD/>
    <TD/>
  </TR>
</xsl:template>

<xsl:template name="dsc">
  <xsl:param name="m" select="MODE"/>
  <xsl:choose>
   <xsl:when test="$m = 'B024'">2.40кг - 04л (баллон 150/225 атм)</xsl:when>
    <xsl:when test="$m = 'B05'">0.50кг - 10л (заготовка 80А2У)</xsl:when>
     <xsl:when test="$m = 'B2'">2.00кг - 40л (заготовка 80А2У)</xsl:when>
     <xsl:when test="$m = 'B3'">3.00кг - 05л (баллон 150/225 атм)</xsl:when>
     <xsl:when test="$m = 'B6'">6.00кг - 10л (баллон 150/225 атм)</xsl:when>
    <xsl:when test="$m = 'B12'">12.0кг - 20л (баллон 150/225 атм)</xsl:when>
    <xsl:when test="$m = 'B19'">19.0кг - 40л (баллон 125/190 атм)</xsl:when>
    <xsl:when test="$m = 'B24'">24.0кг - 40л (баллон 150/225 атм)</xsl:when>
    <xsl:otherwise>нет описания</xsl:otherwise>
  </xsl:choose>
</xsl:template>

</xsl:stylesheet>
