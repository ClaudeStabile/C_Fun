#!/bin/bash
#
# Permet de lire en audio sur le port 4848 une URL rss
#
# Claude Stabile claude@stabile.com
#
# nécessite les packages curlwwwtext text2web mus_8080 du repository : https://github.com/ClaudeStabile/C_Fun
# Nécessite aussi les packages ubuntu html2text et libttspico-utils
# Vérification des arguments
export PATH=.:$PATH 
if [ "$1" == "--help" ]; then
  echo "Usage: $0 URL [--web]"
  exit 0
fi

url=$1
web=false

if [ "$2" == "--web" ]; then
  web=true
fi

# Code pour traiter l'URL ici
echo "Processing URL: $url"
echo "CTRL + C pour arrêter la diffusion"
if $web; then
  echo "Web option is set"
  curlwwwtext $1 > rssclaude.txt && text2webm rssclaude.txt rssclaude.wav rssclaude.webm && mus_8080 rssclaude.webm
else
  echo "Web option is not set"
  curlwwwtext $1 > rssclaude.txt && text2webm rssclaude.txt rssclaude.wav rssclaude.webm && aplay rssclaude.wav
fi
