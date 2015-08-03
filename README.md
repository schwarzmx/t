## t - The command-line todo list manager


`t` is yet another todo list manager for your command line, heavily inspired
by another tool of the [same name](http://stevelosh.com/projects/t/). I simply
wanted to rewrite it myself and maintain it on my own.

### How To

The easiest way to use it is to alias it in your `.bash_profile` and point it to your tasks file:

```
alias t="t --file path/to/your/tasks.json"
```

That way you don't have to specify the tasks file everytime.

In order to add a task simply write the text of your task as the arguments for `t`.

```
$ t do that thing
```

Then you can query your current tasks like:

```
$ t
Current tasks:
[1] done task - Done
[2] do that thing - Todo
```

Mark your task as finished with the `-f` or `--finish` option:

```
$ t -f 2
Current tasks:
[1] done task - Done
[2] do that thing - Done
```

Or you can un-finish a task with the `-u` or `--undo` option:

```
$ t -u 2
Current tasks:
[1] done task - Done
[2] do that thing - Todo
```

You can also remove a task with the `-r` or `--remove` option:

```
$ t -r 1
Current tasks:
[2] do that thing - Done
```

### LICENSE

Licensed under MIT license.

Copyright 2015, Fernando Hernandez
