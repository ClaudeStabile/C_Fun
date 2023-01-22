#!/bin/bash
#
# Permet de lire en audio sur le port 4848 une URL rss
#
# Claude Stabile claude@stabile.com
#
# nécessite les packages read_rss2 mus_480 du repository : https://github.com/ClaudeStabile/C_Fun
# Nécessite aussi les packages ubuntu html2text et libttspico-utils

./read_rss2 https://mastodon.free-solutions.org/@claude.rss | html2text -ascii > rssclaude.txt && text2webm rssclaude.txt rssclaude.wav rssclaude.webm && mus_480 rssclaude.webm



