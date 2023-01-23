# C_Fun
![languageC](https://user-images.githubusercontent.com/29485972/213779007-e4c127fd-848a-4119-887e-559a42fd4c01.png)
[capture_video_lowres.webm](https://user-images.githubusercontent.com/29485972/214055888-320b73c0-baab-47f5-814d-f95ec5892a63.webm)

Le C C'est Fun ! Quelques bidouilles C pour ceux que ça intéresse, Contributors are welcomed

### Programmes en C qui me sont souvent utiles


Dépendances

apt install python3-html2text
apt install libttspico-utils

# www8080 

C'est un micro serveur www qui sert la page index.html du repertoire sur le port 8080, on peut passer un autre port en argument
pour voir la page allez sur http://localhost:8080

Usage ./www8080 {port} le port est optionnel par défaut c'est le 8080 qui est utilisé

# read_rss 

Permet de lire en continu un flux RSS et l'affiche sur stdout
./read_rss 
Usage: ./read_rss <rss_feed_url> [refresh_rate]


# PlayAudioWebmOnWeb 

Permet de jouer en audio un fichier webm spécifié en argument sur le port 4848.
Pour écouter il suffit d'aller sur http://localhost:4848

Usage ./PlayAudioWebmOnWeb music.webm

# ip_nginx

Parse le fichier de log access.log en argument et extrait les adresses IP par nombre de connexions décroissantes

# mus_8080

Permet de jouer un fichier de music ou audio webm sur le port 8080 que l'on peut écouter en allant sur http//localhost:8080

# text2webm 

Permet de produire un fichier au format wav et webm à partir d'un fichier texte. Utilise picotts et ffmpeg

# read_rss2

Permet de lire une URL rss et de produire le format HTML

# text2webm

Permet de transcrire/lire un texte et produire le fichier auformat webm et wav

# curlwwwtext

Permet de lire un site www au format texte, fonctionne avec la plupart des sites d'informations

# ./web_read.sh https://www.lematin.ch/fr

Exemple de script qui combine la production d'un fichier son avec la lecture des textes sur le site.
Le son est soit diffusé sur la sortie son de votre PC soit servi au format HTML5 sur l'URL http://localhost:8080
Exemples
 
Pour écouter : ./web_read.sh https://www.lematin.ch/fr
Pour diffuser sur http://localhost:8080 ./web_read.sh https://www.lematin.ch/fr --web

# Install 

Copiez simplement les binaires sous /usr/local/bin

Package : tar cvzf ../c_fun_0_0_1.tar.gz --exclude .git .


