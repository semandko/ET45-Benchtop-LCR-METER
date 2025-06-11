

ET44、ET45 Benchtop LCR Meter SCPI Communication Protocol

This communication agreement provides that:
NR1: integer, e.g. 123
NR2: a decimal (fixed-point number), as in 12.3
NR3: floating point number, e.g., 12.3E+5
NRf: NR1, NR2 or NR3
NL: carriage return, integer 10



*IDN?

Query syntax SYSTem: VERSion?
The query returns 1999.0<NL>

Query grammar APERture?
The query returns {FAST|MEDium|SLOW}<NL>

Query grammar BIAS:VOLTage[:LEVel]?
The query returns <NR3><NL>
For example: Make BIas to 1500mv
BIAS:VOLTage 1500 (Notice the Spaces in between)；
The query ：BIAS:VOLTage:LEVel?
Returns ：1500

Query Grammar : COMParator[:STATe]?
Query Return : <NR1><NL>
:TOLerance:BIN？

Query grammar COMParator:TOLerance:NOMinal?
Query Return <NR3><NL>
: TOLerance:RANGe:CH

Query Grammar COMParator:TOLerance:RANGe:CH?
Query Return <value><NL >

Query Grammar FETCh?
Query Return <NR3>,<NR3><NL>

The command syntax VOLTage[:LEVel] is <numeric>
Unit mV
Query Grammar VOLTage[:LEVel]?
Query Return <NR3><NL>

Command syntax FREQuency[:CW] <numeric>
Query Grammar FREQuency[:CW]?
Query Return <NR3><NL>

FUNCtion: DEV:MODE {ON|OFF}
Query Grammar FUNCtion:DEV:MODE?
Query Return {ON|OFF}<NL>
:IMPedance:A
Query Grammar FUNCtion:IMPedance:A?
Query Return {AUTO|R|C|L|Z|DCR|ECAP}<NL>
:IMPedance:B

Command syntax FUNCtion: IMPedance: B {X | D | | Q THR | ESR}
Query Grammar FUNCtion:IMPedance:B?
Query Return {X|D|Q|THR|ESR}<NL>
:IMPedance:EUUivalent

Command syntax FUNCtion: IMPedance: EQUivalent
Query Grammar FUNCtion:IMPedance: EQUivalent?
Query Return {SERial|PALlel}<NL>
:IMPedance:RANGe:AUTO

Command syntax FUNCtion: IMPedance: RANGe: AUTO {ON | OFF | 1 | 0}
Query Grammar FUNCtion:IMPedance:RANGe:AUTO?
Query Return <NR1><NL>
:IMPedance:RANGe[:VALue]

Command syntax FUNCtion: IMPedance: RANGe VALue] [: < numeric >
Query grammar FUNCtion:IMPedance:RANGe[:VALue]?
Query return <Numeric><NL>

SYSTem: BEEPer:STATe {ON|OFF|1|0}
Query Grammar SYSTem:BEEPer:STATe?
Query Return <NR1><NL>
:LOCal
