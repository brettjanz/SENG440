main:
	arm-linux-gcc -static -o main.exe main.c
	lftp -c "open user1:q6coHjd7P@arm; put main.exe -o brettliammain.exe; chmod +x brettliammain.exe"
	(sleep 1; echo user1; sleep 1; echo q6coHjd7P; sleep 1; echo ./brettliammain.exe; sleep 5;) | telnet arm

clean:
	rm main.exe
