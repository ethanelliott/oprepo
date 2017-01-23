@echo off
echo Starting Server...
cd "C:\MONGO\bin"
start cmd /c mongod --dbpath C:\Users\Ethan\Desktop\node\OPrepo\data
cd C:\Users\Ethan\Desktop\node\OPrepo
nodemon npm start
pause
