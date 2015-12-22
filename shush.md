shush(1) -- Run a command and optionally report its output by email.
====================================================================

## SYNOPSIS

`shush` [ `-h` | `-v` ]

`shush` [ `-c` *dir* ] [ `-S` | `-s` *facility* ] [ `-vmfk` ] *name* [ *ID* ]

`shush` [ `-c` *dir* ] [ `-H` *to* ] [ `-R` *to* ] [ `-T` *to* ] `-C` *name* [ *stdout* [ *stderr* ] ]

`shush` [ `-i` | `-u` | `-r` ] [ `-c` *dir* ]

## DESCRIPTION

`shush` runs a command and optionally reports on it's status by email. By default, `shush` will not relay any script output to stdout or stderr, as everything is meant to be reported by email. Because interrupting `shush` ignores the following signals: SIGHUP, SIGINT, SIGQUIT, SIGTERM. Such signals should be sent to the running command, rather than `shush`. If you need to kill `shush`, SIGKILL may be used, but only with the knowledge that it should be an action of last resort.

Configuration is managed from a directory (`$HOME/.shush/` by default). For a command to be run using `shush`, it must have a configuration file with a unique name, as well as two optional files, suffixed with .stdout and .stderr respectively, in the configuration directory. The contents of these files are described in the Configuration section.

When the -C option is specified, `shush` will load the configuration file and send any emails, but will not run the commands. Two files can be specified, which will be fed into `shush` to produce any necessary reports. This can be useful for both checking configartion syntax and testing the report output. Additionally, if `shush` failed to properly terminate while running a command, the stdout and stderr will left in the `$TMPDIR` directory (usually `/tmp/`), allowing you to manually produce reports for the interrupted run.

`shush` is also able to manage `crontab`(5) entries via the configuration files. It will scan all files in the configuration directory for the schedule option, and will update the user's `crontab`(5) accordingly.

## OPTIONS

  * `-h`:
    Display a brief help message.

  * `-V`:
    Display the version information. Prefix with `-v` to display compile time defaults.

  * `-c` *dir*:
    Specify the directory where configurations are stored.

  * `-s` *facility*:
    Defines the syslog facility to use for logging.

  * `-S`:
    Disable syslog logging.

  * `-v`:
    Copy information log messages to the standard output.

  * `-f`:
    Fast mode: Any configured randomdelay is ignored.

  * `-m`:
    Monitor and display the command's standard output and error in real time.

  * `-k`:
    Keep the command's output log files instead of deleting them upon completion.

  * `-C`:
    Check the configuration without running any command.

  * `-H` *to*:
    Send a sample HTML report to the specified recipient(s).

  * `-R` *to*:
    Send a sample enriched report to the specified recipient(s).

  * `-T` *to*:
    Send a sample text report to the specified recipient(s).

  * `-i`:
    Use crontab(1) to install a new `crontab`(5) file for the current user's. The user must not already have a `crontab`(5) file.

  * `-u`:
    Use crontab(1) to update the current user's `crontab`(5) file which must already exist.

  * `-r`:
    Remove any entry added by the -u option from the current user's `crontab`(5).

## CONFIGURATION

`shush` configuration files consist of a main section, report section(s) and parameters The main section defines global parameters as well as defaults for reports. Each report section begins with the name of the report between brackets. Lines beginning with the character "#" are ignored. Parameters should be specified only once. If specified multiple times, all but the last occurrence will be ignored, unless noted otherwise. Parameters are defined using the following syntax:

`name=value`

or optionally:

`name@hostname=value`

The second format allows the value to only be applied to a specific hostname.

The following parameters appear only the the main section:

### Main Section

  * `command`:
    The command to run.

  * `lock`:
    If set, `shush` attempts to obtain a lock file before running the command,
    and can also specify the action to take if the lockfile already exists.
    The value format is a comma-separated list of actions. Valid actions are:

    `abort`: Immediately exit.

    `ignore`: Ignore any lockfile and run anyway.

    `loop`: Repeat the list of actions from the beginning. Mostly useful when combined with a time duration.

    a valid time duration: `shush` will wait for the specified time, continually trying to obtain the lockfile. Time durations may be specified in units of w(eeks), d(ays), h(ours), m(inutes) or s(econds). If no unit is specified, it is assumed to be minutes.

    `notify=`*<email>*: will notify the specified email that obtaining the lockfile failed.

  * `lockfile`:
    By default, shush will use a file located in the same directory as the configuration file, and named after the configuration and host names. An alternate filename may be specified using this option.

  * `lockmsg`:
    If set, this string will be used as subject for lock notification(s) mail messages.

  * `path`:
    `shush` does not modify the environment, except to set the `PATH` variable if the path option is set.

  * `randomdelay`:
    If this option is set, shush will wait up to the specified amount of time before starting the command (unless invoked with the `-f`). Valid time units are: s(econds), m(inutes), h(ours), d(ays), w(eeks). If not unit is specified, it is assumed to be minutes.

  * `schedule`:
    This defines when to run this command as a cron job in a `crontab`(5) compatible format. Multiple entries may be specified using the character ";" as separator. Entries prefixed by the character "#" will be skipped. This option is not directly used by shush to run the command, but used by the -i and -u options.

  * `sendmail`:
    This may be used to override the command used to send mail.

  * `shell`:
    By default, the Bourne shell `sh`(1) is used to run the command allowing any shell syntax to be used. An alternate shell may be defined using this option.

  * `stderr`:
    This defines how the command's standard errors are captured and reported to the user: "first", "mixed" or "last". When using "mixed", the name.stderr file is ignored.

  * `syslog`:
    This option is _only_ used by the `-i` and `-u` options and has no other effect on shush. It allows overriding the default syslog facility used for logging and defined at compile time. If left blank, this supresses the use of syslog.

  * `timeout`:
    This option allows one to control how long the command may run. It should be a comma separated list of actions. Actions are executed in the order they are provided, and shush will wait forever if the command is still running once all the actions have been executed unless the string "loop" is one of defined actions. Valid actions are:

    a valid time duration: Simply wait for the command to terminate. Time durations may be specified in units of w(eeks), d(ays), h(ours), m(inutes) or s(econds).

    a signal (either *SIGNAME* or -*SIGNUMBER*): To be sent the command's process group.

    a signal (either =*SIGNAME* or =*SIGNUMBER*): To be sent the shell used to spawn the command.

    `loop`: mark where to start again from when all actions have been executed

    `notify=`<email>: mail addresses to which a notification mail should be sent. If no unit is specified, it is assumed to be minutes.
 
### Any section

  * `to`, `cc`, `bcc`:
    Where to send the mail report.

  * `subject`:
    Subject of the mail report.

  * `header`:
    Additional mail header(s). Note that this parameter may be repeated to specify multiple headers. However, only headers from the report (if specified) or from the main section will be used for a given report.

  * `hostprefix`:
    By default, specified subjects are prefixed with the host name between brackets. This option allows to customize this prefix. A positive integer indicates how many hostname components should be shown. With a negative integer, trailing components of the hostname are shown. The integer zero indicates that the prefix should be omitted.

  * `userprefix`:
    By default, specified subjects are prefixed with the username between brackets. This option allows to disable this prefix. Any non zero value indicates that the username should be shown while zero causes the prefix to be omitted.

  * `format`:
    Mail messages sending the output of the command may be sent in three different formats: "text" (the default), "enriched" text or "html".

  * `sizelimit`:
    By default, the entire output of the command is sent in mail reports. This option may be used to limit the size of the output included in a report. Note that the total size of mail sent will be greater as this limit has no effect upon mail headers. The size can be specified in units of m, k, b, c (MB, KB, Bytes). If no unit is specified, it is assumed to be KB. A limit of zero indicates that the output should not be truncated.

  * `if`:
    A report is only sent if no if condition is specified or if the specified if condition is true. The condition syntax allows for the usual logical operators (`||`, `&&`, `!`), comparison operators (`==`, `!=`, `<`, `<=`, `>`, `>=`) and basic arithmetic operators (`+`, `-`). Asides from counters defined by the configuration, the following variables may be used:

    `$exit`: If the command terminated normally, this is its exit code. Otherwise, it is negative and indicates the signal number having caused the command to terminate (e.g. -1 indicates signal number 1 caused the command to terminate).

    `$size`: Output size (in bytes), same as "$outsize + $errsize"

    `$outsize`: size (in bytes) of standard output

    `$errsize`: size (in bytes) of standard error

    `$lines`: number of lines output

    `$outlines`: number of standard output lines

    `$errlines`: number of standard error lines

    `$runtime`: command run time (in seconds)

    `$utime`: user time used by the command

    `$stime`: system time used by the command

    `$tty`: `1` if shush is run from a terminal (e.g. interactively), `0` otherwise.
    
### Handling stdout and stderr

Shush supports having two files, suffixed with `.stderr` or `.stdout`, to complement the main configuration file. Each of these files should contain a single uppercase letter, followed by a regex. Any lines matching the regex will add a counter to a variable identified by the initial letter, which can then be referenced in an `if` statement. The first successful match will cause the processing to stop. Unlike the variables listed above, no leading `$` should be used when referencing them in the `if` block.

## ENVIRONMENT VARIABLES

  * `HOME`:
    If the `-c` option is not used, shush will look for configuration files in `$HOME/.shush`.

  * `SHUSH_SENDMAIL`:
    If defined, this should point to the `sendmail`(1) binary. This variable overrides the "sendmail" configuration setting and should be used with care.

  * `TMPDIR`:
    Directory where temporary files, such as the lock files, are created.

## EXAMPLE

The following configuration runs "shush -c /etc/shush -u" daily at 9:00, updating the user (root) crontab:

    command=shush -c /etc/shush -u
    schedule=0 9 * * *
    lock=notify=root root-logs,abort
    timeout=5m,notify=root root-logs
    stderr=first
    format=text
    Subject=Crontab Daily Update
    [logs]
    to=root-logs
    [readers]
    if=$exit != 0 || $outlines != 1 || $errsize > 0 || U
    to=root
    format=rich

Assuming the configuration above was placed in a file named `update_shush_cron`, the next two files would be named `update_shush_cron.stdout` and `update_shush_cron.stderr` respectively.

The associated configuration for standard output is:

    Oshush: crontab updated\.$
    U^(.+)$

and for standard error:

    U^(.+)$

A lock will be set while running the command, and an email will be sent to "root" and "root-logs" if the lock is held by another process when shush starts. A mail will also be sent to "root" and "root-logs" if "shush -c /etc/shush -u" runs for more than 5 minutes.  Upon completion, the output will always be sent to "root-logs". Additionally, the output will be sent to "root" if the condition "$exit != 0 || $outlines != 1 || $errsize > 0 || U" is true. For it to be true, one of the following must be true: the exit code is non zero, there was output on standard error or there was output on standard output other than the line "shush: crontab updated.". Any line of output other than "shush: crontab updated." will be displayed in bold in an email sent to "root".

## SEE ALSO

`crontab`(1), `pcre`(3), `regex`(3), `sendmail`(1), `sh`(1).

## AVAILABILITY

The latest release of shush is available on github. The old version is available on http://web.taranis.org/shush/.

## AUTHOR

Christophe Kalt <shush@taranis.org> (Original author)
Stephen Muth <smuth4@gmail.com>
