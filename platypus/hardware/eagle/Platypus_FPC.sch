<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="7.6.0">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S" xrefpart="1_/%S.%C%R">
<libraries>
<library name="Hirose_Bat_Head">
<packages>
<package name="16-HEADER">
<smd name="P$1" x="0" y="0" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$2" x="0.4" y="0" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$3" x="0.8" y="0" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$4" x="1.2" y="0" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$5" x="1.6" y="0" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$6" x="2" y="0" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$7" x="2.4" y="0" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$8" x="2.8" y="0" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$9" x="2.8" y="2.02" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$10" x="2.4" y="2.02" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$11" x="2" y="2.02" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$12" x="1.6" y="2.02" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$13" x="1.2" y="2.02" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$14" x="0.8" y="2.02" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$15" x="0.4" y="2.02" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$16" x="0" y="2.02" dx="0.23" dy="0.74" layer="1"/>
<smd name="P$17" x="-0.58" y="0.29" dx="0.44" dy="0.59" layer="1"/>
<smd name="P$18" x="3.38" y="0.29" dx="0.44" dy="0.59" layer="1"/>
<smd name="P$19" x="3.38" y="1.72" dx="0.44" dy="0.59" layer="1"/>
<smd name="P$20" x="-0.57" y="1.72" dx="0.44" dy="0.59" layer="1"/>
<wire x1="-0.4" y1="2" x2="-0.4" y2="1.6" width="0" layer="21"/>
<wire x1="-0.4" y1="1.6" x2="-0.7" y2="1.6" width="0" layer="21"/>
<wire x1="-0.7" y1="1.6" x2="-0.7" y2="2" width="0" layer="21"/>
<wire x1="-0.7" y1="2" x2="-0.95" y2="2" width="0" layer="21"/>
<wire x1="-1.1" y1="1.85" x2="-1.1" y2="0.15" width="0" layer="21"/>
<wire x1="-0.7" y1="0" x2="-0.7" y2="0.45" width="0" layer="21"/>
<wire x1="-0.7" y1="0.45" x2="-0.4" y2="0.45" width="0" layer="21"/>
<wire x1="-0.4" y1="0.45" x2="-0.4" y2="0" width="0" layer="21"/>
<wire x1="-0.4" y1="0" x2="3.2" y2="0" width="0" layer="21"/>
<wire x1="3.2" y1="0" x2="3.2" y2="0.45" width="0" layer="21"/>
<wire x1="3.2" y1="0.45" x2="3.5" y2="0.45" width="0" layer="21"/>
<wire x1="3.5" y1="0.45" x2="3.5" y2="0" width="0" layer="21"/>
<wire x1="3.9" y1="0.15" x2="3.9" y2="1.85" width="0" layer="21"/>
<wire x1="3.5" y1="2" x2="3.5" y2="1.55" width="0" layer="21"/>
<wire x1="3.5" y1="1.55" x2="3.2" y2="1.55" width="0" layer="21"/>
<wire x1="3.2" y1="1.55" x2="3.2" y2="2" width="0" layer="21"/>
<wire x1="3.2" y1="2" x2="-0.4" y2="2" width="0" layer="21"/>
<wire x1="-0.7" y1="0" x2="-0.95" y2="0" width="0" layer="21"/>
<wire x1="3.5" y1="0" x2="3.75" y2="0" width="0" layer="21"/>
<wire x1="3.5" y1="2" x2="3.75" y2="2" width="0" layer="21"/>
<wire x1="3.75" y1="2" x2="3.9" y2="1.85" width="0" layer="21"/>
<wire x1="3.75" y1="0" x2="3.9" y2="0.15" width="0" layer="21"/>
<wire x1="-0.95" y1="2" x2="-1.1" y2="1.85" width="0" layer="21"/>
<wire x1="-0.95" y1="0" x2="-1.1" y2="0.15" width="0" layer="21"/>
<wire x1="-0.15" y1="1.65" x2="2.95" y2="1.65" width="0" layer="21"/>
<wire x1="2.95" y1="1.65" x2="2.95" y2="0.35" width="0" layer="21"/>
<wire x1="2.95" y1="0.35" x2="-0.15" y2="0.35" width="0" layer="21"/>
<wire x1="-0.15" y1="0.35" x2="-0.15" y2="1.65" width="0" layer="21"/>
<wire x1="-0.05" y1="1.6" x2="2.85" y2="1.6" width="0" layer="21"/>
<wire x1="2.85" y1="1.6" x2="2.9" y2="1.55" width="0" layer="21"/>
<wire x1="2.9" y1="1.55" x2="2.9" y2="0.45" width="0" layer="21"/>
<wire x1="2.9" y1="0.45" x2="2.85" y2="0.4" width="0" layer="21"/>
<wire x1="2.85" y1="0.4" x2="-0.05" y2="0.4" width="0" layer="21"/>
<wire x1="-0.05" y1="0.4" x2="-0.1" y2="0.45" width="0" layer="21"/>
<wire x1="-0.1" y1="0.45" x2="-0.1" y2="1.55" width="0" layer="21"/>
<wire x1="-0.1" y1="1.55" x2="-0.05" y2="1.6" width="0" layer="21"/>
</package>
</packages>
<symbols>
<symbol name="BM10B(0.8)-16DP-0.4V(51)">
<pin name="P$1" x="-10.16" y="10.16" length="middle"/>
<pin name="P$2" x="-10.16" y="7.62" length="middle"/>
<pin name="P$3" x="-10.16" y="5.08" length="middle"/>
<pin name="P$4" x="-10.16" y="2.54" length="middle"/>
<pin name="P$5" x="-10.16" y="0" length="middle"/>
<pin name="P$6" x="-10.16" y="-2.54" length="middle"/>
<pin name="P$7" x="-10.16" y="-5.08" length="middle"/>
<pin name="P$8" x="-10.16" y="-7.62" length="middle"/>
<pin name="P$9" x="15.24" y="-7.62" length="middle" rot="R180"/>
<pin name="P$10" x="15.24" y="-5.08" length="middle" rot="R180"/>
<pin name="P$11" x="15.24" y="-2.54" length="middle" rot="R180"/>
<pin name="P$12" x="15.24" y="0" length="middle" rot="R180"/>
<pin name="P$13" x="15.24" y="2.54" length="middle" rot="R180"/>
<pin name="P$14" x="15.24" y="5.08" length="middle" rot="R180"/>
<pin name="P$15" x="15.24" y="7.62" length="middle" rot="R180"/>
<pin name="P$16" x="15.24" y="10.16" length="middle" rot="R180"/>
<wire x1="-5.08" y1="12.7" x2="10.16" y2="12.7" width="0.254" layer="94"/>
<wire x1="10.16" y1="12.7" x2="10.16" y2="-10.16" width="0.254" layer="94"/>
<wire x1="10.16" y1="-10.16" x2="-5.08" y2="-10.16" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-10.16" x2="-5.08" y2="12.7" width="0.254" layer="94"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="BM10B(0.8)-16DP-0.4V(51)">
<gates>
<gate name="G$1" symbol="BM10B(0.8)-16DP-0.4V(51)" x="-2.54" y="0"/>
</gates>
<devices>
<device name="" package="16-HEADER">
<connects>
<connect gate="G$1" pin="P$1" pad="P$1"/>
<connect gate="G$1" pin="P$10" pad="P$10"/>
<connect gate="G$1" pin="P$11" pad="P$11"/>
<connect gate="G$1" pin="P$12" pad="P$12"/>
<connect gate="G$1" pin="P$13" pad="P$13"/>
<connect gate="G$1" pin="P$14" pad="P$14"/>
<connect gate="G$1" pin="P$15" pad="P$15"/>
<connect gate="G$1" pin="P$16" pad="P$16"/>
<connect gate="G$1" pin="P$2" pad="P$2"/>
<connect gate="G$1" pin="P$3" pad="P$3"/>
<connect gate="G$1" pin="P$4" pad="P$4"/>
<connect gate="G$1" pin="P$5" pad="P$5"/>
<connect gate="G$1" pin="P$6" pad="P$6"/>
<connect gate="G$1" pin="P$7" pad="P$7"/>
<connect gate="G$1" pin="P$8" pad="P$8"/>
<connect gate="G$1" pin="P$9" pad="P$9"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="Platy_Bat_Pad">
<packages>
<package name="FPC-6">
<smd name="P$1" x="0" y="0" dx="3.5" dy="3.5" layer="1"/>
<smd name="P$2" x="5" y="0" dx="2.5" dy="3.5" layer="1"/>
<smd name="P$3" x="10" y="0" dx="3.5" dy="3.5" layer="1"/>
</package>
</packages>
<symbols>
<symbol name="PLATY_BAT_PAD">
<pin name="VCC" x="0" y="0" length="middle" rot="R270"/>
<pin name="TEMP" x="5.08" y="0" length="middle" rot="R270"/>
<pin name="GND" x="10.16" y="0" length="middle" rot="R270"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="PLATY_BAT_PAD">
<gates>
<gate name="G$1" symbol="PLATY_BAT_PAD" x="0" y="5.08"/>
</gates>
<devices>
<device name="" package="FPC-6">
<connects>
<connect gate="G$1" pin="GND" pad="P$3"/>
<connect gate="G$1" pin="TEMP" pad="P$2"/>
<connect gate="G$1" pin="VCC" pad="P$1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="Platy_Charger_Con">
<packages>
<package name="2-PAD">
<smd name="P$1" x="0" y="0" dx="3.5" dy="5" layer="16"/>
<smd name="P$2" x="4" y="0" dx="3.5" dy="5" layer="16"/>
</package>
</packages>
<symbols>
<symbol name="PLATY_CHARGER_CON">
<pin name="P$1" x="0" y="5.08" length="middle" rot="R270"/>
<pin name="P$2" x="2.54" y="5.08" length="middle" rot="R270"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="PLATY_CHARGER_CON">
<gates>
<gate name="G$1" symbol="PLATY_CHARGER_CON" x="0" y="0"/>
</gates>
<devices>
<device name="" package="2-PAD">
<connects>
<connect gate="G$1" pin="P$1" pad="P$1"/>
<connect gate="G$1" pin="P$2" pad="P$2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="supply1">
<description>&lt;b&gt;Supply Symbols&lt;/b&gt;&lt;p&gt;
 GND, VCC, 0V, +5V, -5V, etc.&lt;p&gt;
 Please keep in mind, that these devices are necessary for the
 automatic wiring of the supply signals.&lt;p&gt;
 The pin name defined in the symbol is identical to the net which is to be wired automatically.&lt;p&gt;
 In this library the device names are the same as the pin names of the symbols, therefore the correct signal names appear next to the supply symbols in the schematic.&lt;p&gt;
 &lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
</packages>
<symbols>
<symbol name="GND">
<wire x1="-1.905" y1="0" x2="1.905" y2="0" width="0.254" layer="94"/>
<text x="-2.54" y="-2.54" size="1.778" layer="96">&gt;VALUE</text>
<pin name="GND" x="0" y="2.54" visible="off" length="short" direction="sup" rot="R270"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="GND" prefix="GND">
<description>&lt;b&gt;SUPPLY SYMBOL&lt;/b&gt;</description>
<gates>
<gate name="1" symbol="GND" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="U$1" library="Hirose_Bat_Head" deviceset="BM10B(0.8)-16DP-0.4V(51)" device=""/>
<part name="U$2" library="Platy_Bat_Pad" deviceset="PLATY_BAT_PAD" device=""/>
<part name="U$3" library="Platy_Charger_Con" deviceset="PLATY_CHARGER_CON" device=""/>
<part name="GND1" library="supply1" deviceset="GND" device=""/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="U$1" gate="G$1" x="66.04" y="60.96"/>
<instance part="U$2" gate="G$1" x="88.9" y="63.5"/>
<instance part="U$3" gate="G$1" x="43.18" y="63.5" rot="R180"/>
<instance part="GND1" gate="1" x="99.06" y="48.26"/>
</instances>
<busses>
</busses>
<nets>
<net name="N$1" class="0">
<segment>
<wire x1="55.88" y1="71.12" x2="55.88" y2="68.58" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="P$1"/>
<pinref part="U$1" gate="G$1" pin="P$2"/>
<pinref part="U$1" gate="G$1" pin="P$3"/>
<wire x1="55.88" y1="68.58" x2="55.88" y2="66.04" width="0.1524" layer="91"/>
<junction x="55.88" y="68.58"/>
<pinref part="U$1" gate="G$1" pin="P$4"/>
<wire x1="55.88" y1="66.04" x2="55.88" y2="63.5" width="0.1524" layer="91"/>
<junction x="55.88" y="66.04"/>
<wire x1="88.9" y1="63.5" x2="88.9" y2="76.2" width="0.1524" layer="91"/>
<wire x1="88.9" y1="76.2" x2="55.88" y2="76.2" width="0.1524" layer="91"/>
<wire x1="55.88" y1="76.2" x2="55.88" y2="71.12" width="0.1524" layer="91"/>
<junction x="55.88" y="71.12"/>
<pinref part="U$2" gate="G$1" pin="VCC"/>
</segment>
</net>
<net name="N$2" class="0">
<segment>
<pinref part="U$1" gate="G$1" pin="P$5"/>
<pinref part="U$1" gate="G$1" pin="P$6"/>
<wire x1="55.88" y1="60.96" x2="55.88" y2="58.42" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="P$7"/>
<wire x1="55.88" y1="58.42" x2="55.88" y2="55.88" width="0.1524" layer="91"/>
<junction x="55.88" y="58.42"/>
<pinref part="U$1" gate="G$1" pin="P$8"/>
<wire x1="55.88" y1="55.88" x2="55.88" y2="53.34" width="0.1524" layer="91"/>
<junction x="55.88" y="55.88"/>
<pinref part="U$3" gate="G$1" pin="P$1"/>
<wire x1="43.18" y1="58.42" x2="43.18" y2="53.34" width="0.1524" layer="91"/>
<wire x1="43.18" y1="53.34" x2="55.88" y2="53.34" width="0.1524" layer="91"/>
<junction x="55.88" y="53.34"/>
</segment>
</net>
<net name="GND" class="0">
<segment>
<pinref part="GND1" gate="1" pin="GND"/>
<wire x1="99.06" y1="50.8" x2="99.06" y2="63.5" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="P$16"/>
<pinref part="U$1" gate="G$1" pin="P$15"/>
<wire x1="81.28" y1="71.12" x2="81.28" y2="68.58" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="P$14"/>
<wire x1="81.28" y1="68.58" x2="81.28" y2="66.04" width="0.1524" layer="91"/>
<junction x="81.28" y="68.58"/>
<pinref part="U$1" gate="G$1" pin="P$13"/>
<wire x1="81.28" y1="66.04" x2="81.28" y2="63.5" width="0.1524" layer="91"/>
<junction x="81.28" y="66.04"/>
<pinref part="U$1" gate="G$1" pin="P$12"/>
<wire x1="81.28" y1="63.5" x2="81.28" y2="60.96" width="0.1524" layer="91"/>
<junction x="81.28" y="63.5"/>
<pinref part="U$1" gate="G$1" pin="P$11"/>
<wire x1="81.28" y1="60.96" x2="81.28" y2="58.42" width="0.1524" layer="91"/>
<junction x="81.28" y="60.96"/>
<pinref part="U$1" gate="G$1" pin="P$10"/>
<wire x1="81.28" y1="58.42" x2="81.28" y2="55.88" width="0.1524" layer="91"/>
<junction x="81.28" y="58.42"/>
<pinref part="U$1" gate="G$1" pin="P$9"/>
<wire x1="81.28" y1="55.88" x2="81.28" y2="53.34" width="0.1524" layer="91"/>
<junction x="81.28" y="55.88"/>
<wire x1="99.06" y1="50.8" x2="81.28" y2="50.8" width="0.1524" layer="91"/>
<wire x1="81.28" y1="50.8" x2="81.28" y2="53.34" width="0.1524" layer="91"/>
<junction x="99.06" y="50.8"/>
<junction x="81.28" y="53.34"/>
<wire x1="40.64" y1="58.42" x2="40.64" y2="48.26" width="0.1524" layer="91"/>
<wire x1="40.64" y1="48.26" x2="81.28" y2="48.26" width="0.1524" layer="91"/>
<wire x1="81.28" y1="48.26" x2="81.28" y2="53.34" width="0.1524" layer="91"/>
<pinref part="U$3" gate="G$1" pin="P$2"/>
<pinref part="U$2" gate="G$1" pin="GND"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
