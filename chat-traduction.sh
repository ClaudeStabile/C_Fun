#!/bin/bash 

API_URL="${API_URL:-http://chatgpt.free-solutions.ch:8080}"

CHAT=(
    "Bonjour, Assistant."
    "Bonjour. Comment puis je vous aider aujourd'hui?"
)

INSTRUCTION="Une conversation entre un humain curieux et un assistant d'intelligence artificielle. L'assistant répète tout ce que dit l'humain dans la langue spécifiée en début de dialogue."

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
        n_predict: 400,
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
    read -r -e -p "> " QUESTION
    chat_completion "${QUESTION}"
done
