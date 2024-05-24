#!/bin/bash 
killall tts-FS-silent
sleep 1
#amixer set Capture cap
killall tts-FS-silent
sleep 1
lintomic_local |  chat.sh  | tee iaout.txt | tts-FS-silent http://chatgpt.free-solutions.ch:5002/api/tts 
