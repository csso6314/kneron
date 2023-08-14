REM Change the path to where xt-genldscripts is located
PATH=C:\usr\xtensa\XtDevTools\install\tools\RI-2019.2-win32\XtensaTools\bin

REM Choose vp6_asic as xtensa core to build the ldscripts
xt-genldscripts --xtensa-core=vp6_asic -b .
