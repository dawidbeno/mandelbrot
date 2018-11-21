#! /bin/bash

# Vypise sposob spustania ak nie je zadany ziaden pripinac
if [ $# -eq 0 ]; then
	echo "	$0 [-h]"
	exit 0
fi


# HELP
if [ "$1" = "-h" ]; then
	echo ""
	echo "NAME"
	echo "	$0 - Mandelbrot fractal"
	echo ""
	echo "SYNOPSIS"
	echo "	$0 [-h] [-B] [-S] [-A] [-test]"
	echo ""
	echo "DESCRIPTION"
	echo "	Program vykresluje Mandelbrotov fraktál do obrázku."
	echo ""
	echo "	Options:"
	echo "		-h 	Vypise help"
	echo " 		-B 	Vykreslenie obrazku bez pouzitia instrukcii"
	echo " 		-S 	Vykreslenie obrazku s pouzitim SSE instrukcii"
	echo "		-A 	Vykreslenie obrazku s pouzitim AVX instrukcii"
	echo "		-T 	Vykona testovanie"
	echo ""

	exit 0
fi


TEST=0

# Parsovanie prepinacov
while [ $# -gt 0 ]; do
	if [ "$1" = "-test" ]; then
		echo "Run test"
		TEST=1
	fi
	if [ "$1" = "-A" ]; then
		echo "Run mandelbrot with AVX"
		./mandelEXE -A -w
		echo $?
	fi
	if [ "$1" = "-S" ]; then
		echo "Run mandelbrot with SSE"
		./mandelEXE -S -w
		echo $?
	fi
	if [ "$1" = "-B" ]; then
		echo "Run mandelbrot Basic"
		./mandelEXE -B -w
		echo $?
	fi
	shift
done











