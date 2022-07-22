main:
	arm-linux-gcc -o main.exe main.c
	lftp -c "open user1:q6coHjd7P@arm; mkdir brettliam; put -c -O brettliam main.exe; put -c -O brettliam test.wav; chmod +x brettliam/main.exe;"
	(sleep 1; echo user1; sleep 1; echo q6coHjd7P; sleep 1; echo ./brettliam/main.exe brettliam/test.wav brettliam/decompressed.wav; sleep 30;) | telnet arm

clean:
	rm main.exe
	rm decompressed.wav
	(sleep 1; echo user1; sleep 1; echo q6coHjd7P; sleep 1; echo rm -rf brettliam; sleep 1;) | telnet arm