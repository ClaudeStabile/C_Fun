#!/bin/bash 
killall tts-FS-silent
sleep 1
lintomic_local |  chatFB.sh | tee iaout.txt | tts-FS-silent http://chatgpt.free-solutions.ch:5002/api/tts 
