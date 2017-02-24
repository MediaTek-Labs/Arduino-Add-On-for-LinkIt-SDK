@echo off
echo Build windows executable...
python setup.py install
python setup.py py2exe
pause