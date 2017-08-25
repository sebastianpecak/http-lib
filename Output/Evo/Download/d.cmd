@echo off
if /i "%2"=="d" (
	ddl -p%1 -f debug.dld
) else (
	ddl -p%1 -f release.dld
)