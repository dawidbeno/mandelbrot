#! /bin/bash

# Hellou
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
	echo "	$0 [-h]"
	echo ""
	echo "DESCRIPTION"
	echo "	Program vykresluje Mandelbrotov fraktál do obrázku."
	echo ""
	echo "	Options:"
	echo "		-h 	Vypise help"
	echo " 		-B 	Bez pouzitia instrukcii"
	echo " 		-S 	Pouzitie SSE instrukcii"
	echo "		-A 	Pouzitie AVX instrukcii"
	echo ""

	exit 0
fi

BASIC=0
SSE=0
AVX=0

TEST=0

# Parsovanie prepinacov
while [ $# -gt 0 ]; do
	if [ "$1" = "-test" ]; then
		echo "Run test"
		TEST=1
	fi
	if [ "$1" = "-A" ]; then
		echo "Run mandelbrot with AVX"
		AVX=1
	fi
	if [ "$1" = "-S" ]; then
		echo "Run mandelbrot with SSE"
		SSE=1
	fi
	if [ "$1" = "-B" ]; then
		echo "Run mandelbrot Basic"
		BASIC=1
		bash -c "./mandelEXE -B"
	fi
	shift
done



