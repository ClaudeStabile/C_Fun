#!/bin/bash 
echo "METTEZ un Casque ou des écouteurs avant d'utiliser !!!"
echo "Dites :"
echo "Pouvez vous répéter tout ce que je vais vous dire à partir de maintenant en Allemand"
killall tts-FS-silent
sleep 1
lintomic_local |  chat-traduction.sh | tee german_translation.txt | tts-FS-silent http://chatgpt.free-solutions.ch:5003/api/tts 
