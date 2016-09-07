# Rewind
Rewind  allows  the user to execute a command that makes a change to a system and then after a period of time, if the user has not issued the "keep" command,
will automatically execute a command to restore the system to the previous state.  It is designed for changing settings that run the risk of locking the user
out of the system - For example, altering network settings on a remote machine only accessible over SSH.  This is similar to how many OSs such as
Microsoft Windows handles changing the screen resolution.

## Supported Platforms
Rewind has been tested on Debian 8 Jessie, Debian Sid (Unstable) and FreeBSD 10.3.  It should however be compatible with most UNIX-like systems.

## Installation
### From Release File
0. Extract the release tarball and change into the directory
0. Run `./configure` followed by `make`
0. As root run `make install` to install Rewind onto your system.

### From Repository
Rewind uses Autotools (Automake and Autoconf) to generate its makefile and configure script.  To install from the repo,
change into the src directory and run `./automake.sh` to generate the appropriate files.  You can then follow steps 2 and
3 above to install it on your system.

### FreeBSD Users
When installing on FreeBSD, autoconf will attempt to install the man page to `/usr/local/share/man/` whereas FreeBSD uses the
location `/usr/local/man/`.  In order to install the man page correctly you should provide the argument 
`--mandir /usr/local/man/` to the configure script.

## Usage
Rewind has two commands, "run" and "keep".  `rewind run <CHANGE_COMMAND> <ROLLBACK_COMMAND>` will execute the change command and
if `rewind keep` is not executed within the timeout window (by default 30 seconds) the rollback command will be executed.  An
optional timeout argument (`-t n` where n is a number of seconds) can be supplied to the run command to alter the amount of time
to issue the "keep" command before the rollback command is executed.

### Examples
`rewind run "touch foo" "rm foo" -t 60`
Create a file called "foo", if the user does not execute the keep command within 60 seconds, the file will be deleted.

`rewind run "touch foo && rm bar" "rm foo && touch bar"`
Multiple commands can be supplied by joining them with "&&"

`rewind run "./change.sh" "./rollback.sh"`
Shell scripts can be supplied instead of bare commands

`rewind keep`
Keep the changes from the previous run (i.e. do not run the rollback command)
