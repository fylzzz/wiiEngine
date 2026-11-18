$args_str = $args -join ' '
& C:\msys64\msys2_shell.cmd -ucrt64 -defterm -no-start -here -c "make $args_str"