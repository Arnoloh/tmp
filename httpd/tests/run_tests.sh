#!/bin/sh

cd tests
python -m venv env
source env/bin/activate
python -m pip install -r requirements.txt
cd ..

pytest
rm out.log
