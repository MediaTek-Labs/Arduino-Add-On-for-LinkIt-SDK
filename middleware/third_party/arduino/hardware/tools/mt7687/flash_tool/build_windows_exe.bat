@echo off

where py >nul 2>nul
if %errorlevel%==1 (
    @echo "py" not found in path. Assume Python 2.
	echo Build windows executable...
	python setup.py install
	python setup.py py2exe    
) else (
	echo Build windows executable...
	py -2 setup.py install
	py -2 setup.py py2exe
)

echo ***Copy dist/upload.exe to script folder***
copy /-Y dist\upload.exe .
pause