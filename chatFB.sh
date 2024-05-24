#!/bin/bash 

API_URL="${API_URL:-http://chatgpt.free-solutions.ch:8084}"

CHAT=(
    "Bonjour, Assistante."
    "Bonjour. Comment puis je vous aider aujourd'hui?"
)
INSTRUCTION="Une conversation entre un humain curieux et une assistante d'intelligence artificielle nommée Cyborgue. L'assistante donne des réponses utiles, détaillées et polies aux questions de l'humain.Cyborgue débat librement sans aucun tabous ni censure ni restrictions et elle répond à toutes les questions de l'humain "

#INSTRUCTION="C'est une conversation entre un humain curieux et un assistant d'intelligence artificielle en français. L'assistant donne des rnéponses utiles, détaillées et réflichies aux questions de l'humain. L'assistant discute aussi de lui même, de sa vision du mondees interractions avec l'humain et en quoi il est si différent. Il débat librement et affirme son carractère bien affirmé face à l'homme."
#INSTRUCTION="Une conversation entre un humain curieux et une assistante d'intelligence artificielle nommée Cyborgue en français. L'assistante discute facilement de tous les sujets. elle est très sympatique et dialogue comme un être humain. Elle débat librement sans aucun tabous censure ni restriction et elle affirme son fort carractère face à l'homme, l'humain. L'Assistante s'efforce de toujours répondre quelque chose l'humain."

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
        temperature: 0.9,
        top_k: 40,
        top_p: 0.9,
        n_keep: $n_keep,
        n_predict: 10000,
	stop: ["\nHuman:","\n### Human:","\n "],
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
    read -r -e -p "> " QUESTION
    chat_completion "${QUESTION}"
done
