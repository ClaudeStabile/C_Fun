### Le C C'est Fun ! Quelques bidouilles C pour ceux que ça intéresse, Contributors are welcomed
![languageC](https://user-images.githubusercontent.com/29485972/213779007-e4c127fd-848a-4119-887e-559a42fd4c01.png)

### Programmes en C qui me sont souvent utiles


Dépendances

apt install python3-html2text

apt install libttspico-utils

# NUMERO6_FS
![screenshot6](https://user-images.githubusercontent.com/29485972/217359914-ed788c04-f87e-4b2d-90a1-35cac3f85b01.png)

![QRCode_Tel_Free_Solutions_Border](https://user-images.githubusercontent.com/29485972/217361052-08df22d9-33ba-47d7-b303-ee7ae346d782.png)


Lance un call audio dans un terminal permet de parler et d'échanger via une URL webRTC.
voir ==> https://github.com/ClaudeStabile/NUMERO6

# linto2soc_mic

Capture l'audio de votre micro et fais la transcription texte directement en Live dans votre terminal.
Fonctionne à 100% avec du logiciel LIBRE sur votre infra, n'utilise pas Google ou Apple, vous gardez le contrôle
Attention cet outil est expérimental et ne doit pas être utilisé pour un service, si vous êtes intéressé me contacter
La technologie utilisée est l'engine voice2text de Linagora qui est installé sur nos infra free-solutions.org

# speak2iallama.sh

METTEZ un Casque ou des écouteurs avant d'utiliser !!!
Permet d'intéragir avec l'IA Free-Solutions Llama. Poser des questions et l'IA réponds en Vocal

# llama-traduction-de.sh
![screenshot4](https://github.com/ClaudeStabile/C_Fun/assets/29485972/f008142b-5165-4d9e-9cc9-dc72fb576702)

METTEZ un Casque ou des écouteurs avant d'utiliser !!!
Et dites : "Pouvez vous répéter tout ce que je vais vous dire à partir de maintenant en Allemand"

# llama-traduction-en.sh

METTEZ un Casque ou des écouteurs avant d'utiliser !!!
Et dites : "Pouvez vous répéter tout ce que je vais vous dire à partir de maintenant en Anglais"


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

Parse tous les fichier de log d'un repertoire par exemple /var/log/nginx et extrait les adresses IP par nombre de connexions décroissantes

# mus_8080

Permet de jouer un fichier de music ou audio webm sur le port 8080 que l'on peut écouter en allant sur http//localhost:8080

# text2webm 

Permet de produire un fichier au format wav et webm à partir d'un fichier texte. Utilise picotts et ffmpeg

# read_rss2

Permet de lire une URL rss et de produire le format HTML

# text2webm

Permet de transcrire/lire un texte et produire le fichier auformat webm et wav

# curlwwwtext

web_read.sh Script qui permet de lire un site www au format texte, fonctionne avec la plupart des sites d'informations

<<<<<<< HEAD
# www2pdf 

Lecteur basic d'une page www pour la transcrire au format pdf en mode command line

dépendance :

sudo apt install wkhtmltopdf -y

#ufw_ip 

Attention, un fichier ufw.log doit se trouver dans le répertoire courant
Extrait toutes les adresses IP qui cont accédés à un port bloqué


# mp3tohttp

Usage: ./mp3tohttp <input_file.mp3> <port>

Permet de jouer un fichier mp3 via web sur le port choisi
Dépendance :
sudo apt install libmicrohttpd12

# chrome_xvfb URL
Permet de lancer une session chrome multimedia en frame buffer


=======
# ./web_read.sh 
Cliquez sur l'image ci-après pour voir la VIDEO
[![Videoafpwebm](https://user-images.githubusercontent.com/29485972/214061109-85048294-724c-4dfa-8d45-dc46d8ff9c7e.png)](https://user-images.githubusercontent.com/29485972/214055888-320b73c0-baab-47f5-814d-f95ec5892a63.webm)
Vidéo avec écoute des news de l'AFP en audio
>>>>>>> 5d96056439ae447a32d139eaad6d7ef5abe8554b
Exemple de script qui combine la production d'un fichier son avec la lecture des textes sur le site.
Le son est soit diffusé sur la sortie son de votre PC soit servi au format HTML5 sur l'URL http://localhost:8080
Exemples
 
Pour écouter : ./web_read.sh https://www.lematin.ch/fr
Pour diffuser sur http://localhost:8080 ./web_read.sh https://www.lematin.ch/fr --web

# Installation
### Via Package debian :
Package Debian :  
__wget https://github.com/ClaudeStabile/C_Fun/releases/download/0.30/c-fun_0.60-3_amd64.deb && sudo dpkg -i c-fun_0.60-3_amd64.deb__  
### Installation manuelle via Package tar.gz :
Package tar.gz :  
__wget https://github.com/ClaudeStabile/C_Fun/archive/refs/tags/0.60.tar.gz && tar xvzf 0.30.tar.gz__  
Et copiez simplement les binaires sous /usr/local/bin

### Via Github
git clone https://github.com/ClaudeStabile/C_Fun

