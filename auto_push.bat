@echo off

echo [%date% %time%] Starting auto push...

git add .
git commit -m "Daily auto commit - %date% %time%"
git push origin main

echo [%date% %time%] Auto push completed.