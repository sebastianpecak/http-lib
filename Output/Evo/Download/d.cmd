@echo off
if /i "%1"=="d" (
	ddl -p%2 -f debug.dld
) else (
	ddl -p%2 -f release.dld
)