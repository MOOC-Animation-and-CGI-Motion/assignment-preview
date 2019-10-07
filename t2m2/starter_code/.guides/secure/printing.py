def bold(s):
    return "\033[1m{0}\033[0m".format(s)

def green(s):
    return "\033[32m{0}\033[0m".format(s)

def blue(s):
    return "\033[34m{0}\033[0m".format(s)

def red(s):
    return "\033[31m{0}\033[0m".format(s)

def print_fatal(msg, spaces=True):
    if spaces:
        print("")
    print(red(bold("FATAL ERROR: ") + msg + " Exiting."))
    if spaces:
        print("")
