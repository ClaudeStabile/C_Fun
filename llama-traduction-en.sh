#!/bin/bash 
echo "Dites à haute voix :"
echo "Pouvez vous répéter tout ce que je vais vous dire à partir de maintenant en Anglais"
killall tts-FS-silent
sleep 1
lintomic_local |  chat-traduction.sh | tee english_translation.txt | tts-FS-silent http://chatgpt.free-solutions.ch:5004/api/tts 
