main:
	arm-linux-gcc -o main.exe main.c
	lftp -c "open user1:q6coHjd7P@arm; mkdir brettliam; put -c -O brettliam main.exe; put -c -O brettliam test.wav; chmod +x brettliam/main.exe;"
	(sleep 1; echo user1; sleep 1; echo q6coHjd7P; sleep 1; echo ./brettliam/main.exe brettliam/test.wav brettliam/decompressed.wav; sleep 30;) | telnet arm

opt:
	arm-linux-gcc -o main_optimized.exe main_optimized.c
	lftp -c "open user1:q6coHjd7P@arm; mkdir brettliam; put -c -O brettliam main_optimized.exe; put -c -O brettliam test.wav; chmod +x brettliam/main_optimized.exe;"
	(sleep 1; echo user1; sleep 1; echo q6coHjd7P; sleep 1; echo ./brettliam/main_optimized.exe brettliam/test.wav brettliam/opt_decompressed.wav; sleep 30;) | telnet arm

all:
	arm-linux-gcc -o main.exe main.c
	arm-linux-gcc -o main_optimized.exe main_optimized.c
	lftp -c "open user1:q6coHjd7P@arm; mkdir brettliam; put -c -O brettliam main_optimized.exe; put -c -O brettliam main.exe; put -c -O brettliam test.wav; chmod +x brettliam/main_optimized.exe; chmod +x brettliam/main.exe;"
	(sleep 1; echo user1; sleep 1; echo q6coHjd7P; sleep 1; echo "Main"; echo ./brettliam/main.exe brettliam/test.wav brettliam/decompressed.wav; sleep 10; echo "Optimized"; echo ./brettliam/main_optimized.exe brettliam/test.wav brettliam/opt_decompressed.wav; sleep 10; echo "Differences:"; echo cmp -l decompressed.wav opt_decompressed.wav; sleep 30;) | telnet arm
	
clean:
	(sleep 1; echo user1; sleep 1; echo q6coHjd7P; sleep 1; echo rm -rf brettliam; sleep 1;) | telnet arm