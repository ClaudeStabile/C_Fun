#!/bin/bash
#
# Permet de lire en audio sur le port 4848 une URL rss
#
# Claude Stabile claude@stabile.com
#
# nécessite les packages read_rss2 mus_480 du repository : https://github.com/ClaudeStabile/C_Fun
# Nécessite aussi les packages ubuntu html2text et libttspico-utils
# Vérification des arguments
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 url_rss"
    echo "Exemple: $0 http://feeds.feedburner.com/Example"
    exit 1
fi

./read_rss2 $1 | html2text -ascii > rssclaude.txt && text2webm rssclaude.txt rssclaude.wav rssclaude.webm && mus_8080 rssclaude.webm



