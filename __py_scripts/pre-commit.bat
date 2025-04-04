call env\Scripts\activate.bat

set "FILES=img_to_hpp.py"

mypy --strict %FILES%
flake8 %FILES% --show-source --statistics --max-line-length 99 --ignore=E203