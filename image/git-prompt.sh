if test -f ~/.config/git/git-prompt.sh
then
  . ~/.config/git/git-prompt.sh
else
  PS1='\[\033]0;\u@\h:$PWD\007\]' # set window title
  PS1="$PS1"'\n'                  # new line
  PS1="$PS1"'\[\033[32m\]\u@\h'   # green user@host
  PS1="$PS1"'\[\033[35m\]:'       # purple :
  PS1="$PS1"'\[\033[33m\]\w'      # brownish yellow working directory
  if test -f "/etc/bash_completion.d/git"
  then
    . "/etc/bash_completion.d/git"
    GIT_EXEC_PATH="$(git --exec-path 2>/dev/null)"
    COMPLETION_PATH="${GIT_EXEC_PATH%/libexec/git-core}"
    COMPLETION_PATH="${COMPLETION_PATH%/lib/git-core}"
    COMPLETION_PATH="$COMPLETION_PATH/share/git-core/contrib/completion"
    if test -f "$COMPLETION_PATH/git-prompt.sh"
    then
      . "/usr/share/git-core/contrib/completion/git-prompt.sh"
      PS1="$PS1"'\[\033[36m\]'  # change color to cyan
      PS1="$PS1"'`__git_ps1`'   # bash function
    fi
  fi
  PS1="$PS1"'\[\033[0m\]'        # change color
  PS1="$PS1"'\n'                 # new line
  PS1="$PS1"'$ '                 # prompt: always $
fi
