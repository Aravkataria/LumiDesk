@echo off
echo Starting LumiDesk Server...

:: 1. Tell Windows where Conda is located
:: (Change "miniconda3" to "anaconda3" if you installed the full Anaconda version)
call "DELL\miniconda3\Scripts\activate.bat" "DELL\miniconda3"

:: 2. Activate your specific environment
call conda activate spotifyoled

:: 3. Run the server
python app.py

pause