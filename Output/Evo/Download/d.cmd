@echo off
if /i "%1"=="d" (
	ddl2 -p%2 -f debug.dld
) else (
	ddl2 -p%2 -f release.dld
)