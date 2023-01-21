# C_Fun
![languageC](https://user-images.githubusercontent.com/29485972/213779007-e4c127fd-848a-4119-887e-559a42fd4c01.png)

Le C C'est Fun ! Quelques bidouilles C pour ceux que ça intéresse, Contributors are welcomed

### Programmes en C qui me sont souvent utiles

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


