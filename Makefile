main:
	arm-linux-gcc -static -o main.exe main.c
	lftp -c "open user1:q6coHjd7P@arm; mirror -P 10 -R -n -x '.git' . brettliam;"
	(sleep 1; echo user1; sleep 1; echo q6coHjd7P; sleep 1; echo ./brettliam/main.exe test.wav; sleep 5;) | telnet arm

clean:
	rm main.exe
