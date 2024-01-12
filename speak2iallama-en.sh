#!/bin/bash 
killall tts-FS-silent
sleep 1
touch iaout.txt
cat /dev/null>iaout.txt
tail -f iaout.txt  | tts-FS-silent http://chatgpt.free-solutions.ch:5004/api/tts &
lintomic_local |  chat-en.sh | tee iaout.txt
