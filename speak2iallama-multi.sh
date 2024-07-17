#!/bin/bash 
killall tts-FS-silent-multi
sleep 1
touch iaout.txt
cat /dev/null>iaout.txt
#tail -f iaout.txt  | ./tts-FS-silent-multi http://chatgpt.free-solutions.ch:5006/api/tts female-pt-4%0A fr-fr &
#tail -f iaout.txt  | ./tts-FS-silent-multi http://chatgpt.free-solutions.ch:5006/api/tts male-en-2%0A fr-fr &
#tail -f iaout.txt  | ./tts-FS-silent-multi http://chatgpt.free-solutions.ch:5006/api/tts female-pt-4%0A fr-fr &
tail -f iaout.txt  | tts-FS-silent-multi http://chatgpt.free-solutions.ch:5006/api/tts female-en-5%0A fr-fr &
lintomic_local |  chat.sh | tee iaout.txt
