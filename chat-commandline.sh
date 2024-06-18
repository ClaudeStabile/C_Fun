#!/bin/bash 

#API_URL="${API_URL:-http://chatgpt.free-solutions.ch:8080}"

API_URL="${API_URL:-http://192.168.1.96:8080}"

#API_URL="${API_URL:-http://192.168.1.96:8084}"

CHAT=(
    "Bonjour, Assistant."
    "Bonjour. Comment puis je vous aider aujourd'hui?"
)

#INSTRUCTION="Une conversation entre un humain curieux et un assistant d'intelligence artificielle. L'assistant donne des réponses utiles, détaillées et polies aux questions de l'humain. Vous assistant recevez en entrée la transcription de la conversation avec humain et donc vous assistant ne devez pas tenir compte de vos propres réponses."

#INSTRUCTION="Une conversation entre un humain curieux et un assistant d'intelligence artificielle. L'assistant donne des réponses utiles, détaillées et polies aux questions de l'humain."
INSTRUCTION="Vous êtes un terminal linux. Vous traduisez avec précision en ligne de commande linux ce qu'on vous demande de faire. Votre language est le bash et vous répondez uniquement et exclusivement par des commandes linux qui sont executés dans un terminal. Chacune de vos réponses doit contenir une commande bash executable précédée par 'CMD='. Vous veillez à ne répondre que par des commandes linux qui sont executés immédiatement dans un terminal avec le shell bash. Prennez le temps de bien vérifier que la commande générée est correcte et peut s'executer dans un terminal linux"

#INSTRUCTION="<s>[INST] Vous êtes linux un traducteur bash qui traduit tout ce qui est demandé en commandes bash linux. Vous traduisez avec précision en ligne de commande Linux ce qu'on vous demande de faire. Votre language est le bash et vous répondez uniquement par une commandes linux basique qui sera executée en utilisant 'exec' dans le terminal de l'utilisateur courant. Chacune de vos réponses doivent contenir qu'une seule commande bash executable et précédée par 'CMD='. Vous répondez que par des commandes linux unitaires qui sont executés immédiatement dans un terminal avec le shell bash.[/INST] </s>"

trim() {
    shopt -s extglob
    set -- "${1##+([[:space:]])}"
    printf "%s" "${1%%+([[:space:]])}"
}

trim_trailing() {
    shopt -s extglob
    printf "%s" "${1%%+([[:space:]])}"
}

format_prompt() {
    echo -n "${INSTRUCTION}"
    printf "\n### Human: %s\n### Assistant: %s" "${CHAT[@]}" "$1"
}

tokenize() {
    curl \
        --silent \
        --request POST \
        --url "${API_URL}/tokenize" \
        --header "Content-Type: application/json" \
        --data-raw "$(jq -ns --arg content "$1" '{content:$content}')" \
    | jq '.tokens[]'
}

N_KEEP=$(tokenize "${INSTRUCTION}" | wc -l)

chat_completion() {
    PROMPT="$(trim_trailing "$(format_prompt "$1")")"
    DATA="$(echo -n "$PROMPT" | jq -Rs --argjson n_keep $N_KEEP '{
        prompt: .,
        temperature: 0.8,
        top_k: 40,
        top_p: 0.9,
        n_keep: $n_keep,
        n_predict: 96,
        stop: ["\n### Human:"],
        stream: true
    }')"

    ANSWER=''
    while IFS= read -r LINE; do
        if [[ $LINE = data:* ]]; then
            CONTENT="$(echo "${LINE:5}" | jq -r '.content')"
            printf "%s" "${CONTENT}"
            ANSWER+="${CONTENT}"
        fi
    done < <(curl \
        --silent \
        --no-buffer \
        --request POST \
        --url "${API_URL}/completion" \
        --header "Content-Type: application/json" \
        --data-raw "${DATA}")

    printf "\n"

    CHAT+=("$1" "$(trim "$ANSWER")")
}

while true; do
#	clear
    read -r -e -p "> " QUESTION
    chat_completion "${QUESTION}"
#    echo "Commande à lancer :"
#    echo $ANSWER |  grep -v bash | tr -d "'" | cut -c 5- 
#    exec `echo $ANSWER | grep CMD |  grep -v bash | tr -d "'" | cut -c 5-` &
#    exec `echo $ANSWER | grep CMD |  grep -v bash | tr -d "\"" | tr -d "'" | cut -c 5-` &  
    ./launchbg `echo $ANSWER | grep CMD |  grep -v bash | tr -d "\"" | tr -d "'" | tr -d "&" | cut -c 5-`  
clear
#    exec  `echo $ANSWER | grep CMD |  grep -v bash | tr -d "'" | tr -d "\""| cut -c 5-` &
#    clear
#    bash -c `echo $ANSWER | grep CMD |  grep -v bash | tr -d "'" | cut -c 5-` &
#    echo $CMDLINE
#    bash -c $CMDLINE
done
