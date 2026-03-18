# Description

This is a minor command-line utility, vibe-coded in C with locally run LLM models.

Using:
  pi / pi-coding-agents
  ollama + gpt-oss:20b

# Purpose

- I wanted to try out vibe-coding, starting with something small.
- I wanted to see if I could do it safely and securely without leaking IP (except open source on github).
- I wanted a minimal utility to keep track of stuff. When and how often I did something, with the possibility to later summarize if something is trending up or down and if I reach goals, etc.

# Usage

Build with this command:

    $ gcc -Wall -Iinclude -o bin/counters src/*.c

Examples:

    $ ./bin/counters --set 0 breakfast
    $ ./bin/counters breakfast
    0
    $ ./bin/counters --update 1 breakfast
    $ ./bin/counters --update 1 breakfast # second breakfast
    $ ./bin/counters breakfast
    2

Help:

    $ ./bin/counters --help
    cmdline-counter
    Usage:
      app [options] [counter]
    
    Options:
      -f, --file=FILE        specify counters file (default: $COUNTERS_FILE or $HOME/.counters)
      --set=VALUE, -s VALUE  set counter to VALUE
      --update=VALUE, -u VALUE increment counter by VALUE
      --delete, -d           delete counter
      -l, --log[=FILE]       specify log file (default: $COUNTERS_LOG or $HOME/.counters.log)
      -t, --timestamp=TS     override timestamp for logs (milliseconds)
      -h, --help, -?         show this help
    
    Environment variables:
      COUNTERS_FILE          default counter file path
      COUNTERS_LOG           default log file path
      HOME                   used for default paths when env vars are unset

# License

Whatever. It's 99% vibe-coded.

