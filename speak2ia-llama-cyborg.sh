#!/bin/bash 
killall ./tts-FS-silent
sleep 1
touch iaout.txt
cat /dev/null>iaout.txt
tail -f iaout.txt  | ./tts-FS-silent http://chatgpt.free-solutions.ch:5002/api/tts  &
./lintomic_local |  ./chat_droid-albert.sh | tee iaout.txt | sendxmpp -r "Cyborgue" -t -i -u fsbot1@www.free-solutions.org -p fsbotfsbot --chatroom llama@conference.www.free-solutions.org
