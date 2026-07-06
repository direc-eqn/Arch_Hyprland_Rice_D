# Created by newuser for 5.9.1
unsetopt PROMPT_SP

autoload -Uz compinit promptinit
compinit
promptinit

# Starship
export STARSHIP_CONFIG="$HOME/.config/starship/starship.toml"
eval "$(starship init zsh)"

 # SSH-Agent
export SSH_AUTH_SOCK="$XDG_RUNTIME_DIR/ssh-agent.socket" 


# Wrapper that provides the ability to change the current working directory when exiting Yazi
export EDITOR="nvim"
function y() {
	local tmp="$(mktemp -t "yazi-cwd.XXXXXX")" cwd
	command yazi "$@" --cwd-file="$tmp"
	IFS= read -r -d '' cwd < "$tmp"
	[ "$cwd" != "$PWD" ] && [ -d "$cwd" ] && builtin cd -- "$cwd"
	command rm -f -- "$tmp"
}

# Source the desired files from your
source <(fzf --zsh)

# Use fd instead of fzf
export FZF_DEFAULT_COMMAND="fd --hidden --strip-cwd-prefix --exclude .git"
export FZF_CTRL_T_COMMAND="$FZF_DEFAULT_COMMAND"
export FZF_ALT_C_COMMAND="fd --type=d --hidden --strip-cwd-prefix --exclude .git"

# Use fd for listing path candidates
# - The first arugment to the fuction ($1) is the base path to start traversal
# - See the source code (completion.{bash,zsh} for the details

_fzf_compgen_path() {
    fd --hidden --exclude .git . "$1"
}

_fzf_compgen_dir(){
    fd --type=d --hidden --exclude .git . "$1"
}

# Set up previews with fzf
export FZF_CTRL_T_OPTS="--preview 'bat -n --color=always --line-range :500 {}'"
export FZF_ALT_C_OPTS="--preview 'eza --tree --color=always {} | head -200'"

_fzf_comprun() {
    local command=$1
    shift

    case "$command" in 
        cd)             fzf --preview 'eza --tree --color=always {} | head -200'                    "$@" ;;
        export|unset)   fzf --preview "eval 'eza 'echo \$' {}"                                      "$@" ;;
        ssh)            fzf --preview 'dig {}'                                                      "$@" ;;
        *)              fzf --preview "--preview 'bat -n --color=always ==ling-range :500 {}'"      "$@" ;;
    esac
}

# Alias list
alias ls="eza --color=always --git --no-filesize --icons=always --no-time --no-user --no-permissions"
alias cd="z"

# Zoxide 
eval "$(zoxide init zsh)"
