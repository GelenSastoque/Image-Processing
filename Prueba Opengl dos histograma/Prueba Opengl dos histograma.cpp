/*PROCESAMIENTO DE UNA IMAGEN: BRILLO,CONTRASTE,CONTRASTE AUTOMATICO, NEGATIVO
Realizado por:
- Valeria Barrera Serrano
- Fernanda Isabel Corredor
- Gelen Adriana Sastoque Cortes*/
#pragma once
#ifdef _WIN32
#include "glut.h"
#elif defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>

using namespace std;
#pragma pack(push, 1)
int window;

#define _MATH_DEFINES_DEFINED
#define M_PI 3.14159265358979
#define Euler 2.7182

struct BMPFileHeader {
	uint16_t Tipo{ 0x4D42 };          // File type always BM which is 0x4D42 (stored as hex uint16_t in little endian)
	uint32_t Size{ 0 };               // Size of the file (in bytes)
	uint16_t reserved1{ 0 };               // Reserved, always 0
	uint16_t reserved2{ 0 };               // Reserved, always 0
	uint32_t offset_data{ 0 };             // Start position of pixel data (bytes from the beginning of the file)
};

struct BMPInfoHeader {
	uint32_t size{ 0 };                      // Size of this header (in bytes)
	int32_t width{ 0 };                      // width of bitmap in pixels
	int32_t height{ 0 };                     // width of bitmap in pixels
											 //       (if positive, bottom-up, with origin in lower left corner)
											 //       (if negative, top-down, with origin in upper left corner)
	uint16_t planes{ 1 };                    // No. of planes for the target device, this is always 1
	uint16_t bit_count{ 0 };                 // No. of bits per pixel
};
BMPFileHeader file_header;
BMPInfoHeader info_header;

void pixel(float cx, float cy, float red, float green, float blue)
{
	if (red > 255.0)
	{
		red = 255.0;
	}
	if (red < 0.0)
	{
		red = 0.0;
	}
	if (green > 255.0)
	{
		green = 255.0;
	}
	if (green < 0.0)
	{
		green = 0.0;
	}
	if (blue > 255.0)
	{
		blue = 255.0;
	}
	if (blue < 0.0)
	{
		blue = 0.0;
	}

	//Definir tamaño del pixel
	glPointSize(1);
	//Definir color del pixel
	glColor3f(red / 255, green / 255, blue / 255);
	//Establecemos la primitiva punto
	glBegin(GL_POINTS);
	//Establecemos la posicion del pixel
	glVertex2f(cx, cy);
	//Finalizamos la funcion
	glEnd();
	////Intercambiamos buffers
	/*glFlush();*/
}
void linea(float x1, float y1, float x2, float y2, float r, float g, float b)
{
	/*Recibe como parametros las coordenadas de dos puntos y genera una linea entre
	ellos*/
	glColor3f(r, g, b);
	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
	glFlush();
}
float kb, kc, kca, kn, gamma;
struct BMP {

	std::vector<uint8_t> data;
	BMP(const char* nombreimg) {
		leerimg(nombreimg);
	}
	void leerimg(const char* nombreimg) {
		std::ifstream img{ nombreimg, std::ios_base::binary };
		if (img) {
			img.read((char*)&file_header, sizeof(file_header));
			img.read((char*)&info_header, sizeof(info_header));
			// Jump to the pixel data location
			img.seekg(file_header.offset_data, img.beg);
			file_header.Size = file_header.offset_data;
			data.resize(info_header.width * info_header.height * info_header.bit_count / 8);

			// Here we check if we need to take into account row padding
			if (info_header.width % 4 == 0) {
				img.read((char*)data.data(), data.size());
				file_header.Size += static_cast<uint32_t>(data.size());
			}
			else {
				row_stride = info_header.width * info_header.bit_count / 8;
				uint32_t new_stride = make_stride_aligned(4);
				std::vector<uint8_t> padding_row(new_stride - row_stride);

				for (int y = 0; y < info_header.height; ++y) {
					img.read((char*)(data.data() + row_stride * y), row_stride);
					img.read((char*)padding_row.data(), padding_row.size());
				}
				file_header.Size += static_cast<uint32_t>(data.size()) + info_header.height * static_cast<uint32_t>(padding_row.size());
			}
		}
		else {
			throw std::runtime_error("Unable to open the input image file.");
		}
	}

	void Infocolorpix() {
		uint32_t channels = info_header.bit_count / 8;
		for (uint32_t y = 0; y < 100; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				int B = data[channels * (y * info_header.width + x) + 0];
				int R = data[channels * (y * info_header.width + x) + 2];
				int G = data[channels * (y * info_header.width + x) + 1];
				std::cout << "[R:" << R << " ";
				std::cout << "G:" << G << " ";
				std::cout << "B:" << B << " ]\t";
			}
			std::cout << std::endl;
		}

	}

	void dibujarimg() {
		float r, g, b;
		int pos = 0, lim = 0;
		uint32_t channels = info_header.bit_count / 8;
		for (uint32_t y = 0; y < info_header.height; y++) {
			pos = 0;
			for (uint32_t x = lim; x < lim + (info_header.width * 3); x += 3)
			{
				b = data[x];
				g = data[x + 1];
				r = data[x + 2];
				pos++;
				pixel(pos, y, r, g, b);
			}
			lim = lim + (info_header.width * 3);
		}
		glFlush();
	}

	void subirbrillo()
	{
		float r, g, b, rb, gb, bb;

		int pos = 0, lim = 0;
		for (uint32_t y = 0; y < info_header.height; y++) {
			pos = 0;
			for (uint32_t x = lim; x < lim + (info_header.width * 3); x += 3)
			{
				b = data[x];
				bb = b + kb;
				g = data[x + 1];
				gb = g + kb;
				r = data[x + 2];
				rb = r + kb;
				pos++;
				pixel(pos, y, rb, gb, bb);
			}
			lim = lim + (info_header.width * 3);
		}
		glFlush();
	}
	float sumab = 0, sumar = 0, sumag = 0;
	float miur, miug, miub;
	void contrastesuma()
	{
		float r, g, b, rc, gc, bc;

		int pos = 0, lim = 0;

		uint32_t channels = info_header.bit_count / 8;
		for (uint32_t y = 0; y < info_header.height; y++) {
			for (uint32_t x = lim; x < lim + (info_header.width * 3); x += 3)
			{
				b = data[x];
				sumab = sumab + b;
				g = data[x + 1];
				sumag = sumag + g;
				r = data[x + 2];
				sumar += r;
			}
			lim = lim + (info_header.width * 3);
		}
		miur = sumar / (info_header.height * info_header.width);
		miug = sumag / (info_header.height * info_header.width);
		miub = sumab / (info_header.height * info_header.width);


	}
	void contraste()
	{
		float r, g, b, rc, gc, bc;
		int pos = 0, lim = 0;
		contrastesuma();
		for (uint32_t y = 0; y < info_header.height; y++) {
			pos = 0;
			for (uint32_t x = lim; x < lim + (info_header.width * 3); x += 3)
			{
				b = data[x];
				bc = (b - miub) * kc + miub;
				g = data[x + 1];
				gc = (g - miug) * kc + miug;
				r = data[x + 2];
				rc = (r - miur) * kc + miur;
				pos++;
				pixel(pos, y, rc, gc, bc);
			}
			lim = lim + (info_header.width * 3);
		}
		glFlush();
	}
	float maxb = 0, minb = 0, maxg = 0, ming = 0, maxr = 0, minr = 0;
	void contrasteautomaxmin()
	{
		float r, g, b;
		int pos = 0, lim = 0;
		for (uint32_t y = 0; y < info_header.height; y++) {
			pos = 0;
			for (uint32_t x = lim; x < lim + (info_header.width * 3); x += 3)
			{
				b = data[x];
				if (b > maxb)
				{
					maxb = b;
				}
				g = data[x + 1];
				if (g > maxg)
				{
					maxg = g;
				}
				r = data[x + 2];
				if (r > maxr)
				{
					maxr = r;
				}
			}
			lim = lim + (info_header.width * 3);
		}
		glFlush();
	}
	void contrasteauto()
	{
		float r, g, b, rca, gca, bca;
		int pos = 0, lim = 0;
		contrasteautomaxmin();
		uint32_t Lc = info_header.bit_count / 3;
		uint32_t L = pow(2, Lc);
		for (uint32_t y = 0; y < info_header.height; y++) {
			pos = 0;
			for (uint32_t x = lim; x < lim + (info_header.width * 3); x += 3)
			{
				b = data[x];
				bca = ((b - minb) / (maxb - minb)) * (L - 1);
				g = data[x + 1];
				gca = ((g - ming) / (maxg - ming)) * (L - 1);
				r = data[x + 2];
				rca = ((r - minr) / (maxr - minr)) * (L - 1);
				pos++;
				pixel(pos, y, rca, gca, bca);
			}
			lim = lim + (info_header.width * 3);
		}
		glFlush();
	}
	void negativo()
	{
		float r, g, b, rn, gn, bn;
		int pos = 0, lim = 0;
		uint32_t Lc = info_header.bit_count / 3;
		uint32_t L = pow(2, Lc);
		for (uint32_t y = 0; y < info_header.height; y++) {
			pos = 0;
			for (uint32_t x = lim; x < lim + (info_header.width * 3); x += 3)
			{
				b = data[x];
				bn = L - b;
				g = data[x + 1];
				gn = L - g;
				r = data[x + 2];
				rn = L - r;
				pos++;
				pixel(pos, y, bn, gn, rn);
			}
			lim = lim + (info_header.width * 3);
		}
		glFlush();
	}
	void trasnformacionGamma()
	{
		float r, g, b, rg, gg, bg;
		int pos = 0, lim = 0;
		for (uint32_t y = 0; y < info_header.height; y++) {
			pos = 0;
			for (uint32_t x = lim; x < lim + (info_header.width * 3); x += 3)
			{
				b = data[x];
				bg = (pow(b / 255, gamma)) * 255;
				g = data[x + 1];
				gg = (pow(g / 255, gamma)) * 255;
				r = data[x + 2];
				rg = (pow(r / 255, gamma)) * 255;
				pos++;
				pixel(pos, y, bg, gg, rg);
			}
			lim = lim + (info_header.width * 3);
		}
		glFlush();
	}
	int rh, gh, bh, rg, gg, bg;
	int vr[256] = { 0 }, vg[256] = { 0 }, vb[256] = { 0 };
	void histograma() {

		int pos = 0, lim = 0;
		for (uint32_t y = 0; y < info_header.height; y++) {
			pos = 0;
			for (uint32_t x = lim; x < lim + (info_header.width * 3); x += 3)
			{
				bh = data[x];
				vb[bh]++;
				gh = data[x + 1];
				vg[gh]++;
				rh = data[x + 2];
				vr[rh]++;
				pos++;
			}
			lim = lim + (info_header.width * 3);
		}
	}
	void pintarhisto()
	{
		histograma();
		int ini;
		//Para canal b
		for (int i = 0; i < 256; i++)
		{
			ini = vb[i];
			linea(i, ini, i + 1, vb[i + 1], 0, 0, 1);
		}
		//Para canal r
		for (int i = 0; i < 256; i++)
		{
			ini = vr[i];
			linea(i, ini, i + 1, vr[i + 1], 1, 0, 0);
		}
		//Para canal g
		for (int i = 0; i < 256; i++)
		{
			ini = vg[i];
			linea(i, ini, i + 1, vg[i + 1], 0, 1, 0);
		}
		linea(0, 0, 256, 0, 1, 1, 1);
		linea(0, 0, 0, 20000, 1, 1, 1);

	}
	double vbe[256] = { 0 }, vge[256] = { 0 }, vre[256] = { 0 }, vpp[256] = { 0 };
	void histequalizado()
	{
		histograma();
		double size = info_header.height * info_header.width;
		cout << "size: " << size << endl;
		int ini = 0, ini2 = 0, ini3 = 0, ini4 = 0, ini5 = 0, ini6 = 0;
		//Para canal b
		float acum = 0.0, acum2 = 0.0, acum3 = 0.0, acum4 = 0.0, acum5 = 0.0, acum6 = 0.0;
		for (int i = 0; i < 256; i++)
		{
			double vecaux[256] = { 0 }, vecaux2[256] = { 0 };
			int bp = vb[i], bp2 = vb[i + 1];
			vecaux[i] = (bp / size);
			vecaux2[i] = (bp2 / size);
			double baux = vecaux[i]; double baux2 = vecaux2[i];
			acum = acum + baux;
			acum2 = acum2 + baux2;
			vbe[i] = round(acum * 255.0);
			vbe[i + 1] = round(acum2 * 255.0);
			ini = vbe[i];
			ini2 = vbe[i + 1];
			linea(i, ini, i + 1, ini2, 0, 0, 1);

		}

		//Para canal r
		for (int i = 0; i < 256; i++)
		{
			double vecaux[256] = { 0 }, vecaux2[256] = { 0 };
			int rp = vr[i]; int rp2 = vr[i + 1];
			vecaux[i] = (rp / size);
			vecaux2[i] = (rp2 / size);
			double baux = vecaux[i]; double baux2 = vecaux2[i];
			acum3 = acum3 + baux;
			acum4 = acum4 + baux2;
			vre[i] = round(acum3 * 255.0);
			vre[i + 1] = round(acum4 * 255.0);
			ini3 = vre[i];
			ini4 = vre[i + 1];
			linea(i, ini3, i + 1, ini4, 1, 0, 0);
		}
		//Para canal g
		for (int i = 0; i < 256; i++)
		{
			double vecaux[256] = { 0 }, vecaux2[256] = { 0 };

			int gp = vg[i], gp2 = vg[i + 1];
			vecaux[i] = (gp / size);
			vecaux2[i] = (gp2 / size);
			double baux = vecaux[i], baux2 = vecaux2[i];
			acum5 = acum5 + baux;
			acum6 = acum6 + baux2;
			vge[i] = round(acum5 * 255.0);
			vge[i + 1] = round(acum6 * 255.0);
			ini5 = vge[i];
			ini6 = vge[i + 1];
			linea(i, ini5, i + 1, ini6, 0, 1, 0);
		}
		linea(0, 0, 256, 0, 1, 1, 1);
		linea(0, 0, 0, 60, 1, 1, 1);
	}
	std::vector<uint8_t> red, green, blue;
	int** rojo; int** verde; int** azul; int** resr; int** resv; int** resa;
	void crearvectorrgb()
	{
		red.resize(info_header.width * info_header.height);
		green.resize(info_header.width * info_header.height);
		blue.resize(info_header.width * info_header.height);
		int pos = 0, lim = 0, lim2 = 0, b, g, r, acum = 0, suma = 0;
		for (uint32_t y = 0; y < info_header.height; y++) {
			pos = 0;
			for (uint32_t x = lim; x < lim + (info_header.width * 3); x += 3)
			{
				b = data[x];
				blue[acum] = b;
				g = data[x + 1];
				green[acum] = g;
				r = data[x + 2];
				red[acum] = r;
				pos++;
				acum++;
			}

			lim = lim + (info_header.width * 3);
		}
	}
	void Filtros(int filas, int col, float** fil, bool sobel, int opchv)
	{
		crearvectorrgb();
		//Variables
		int resultado;
		float auxfil[3][3];

		//creacion matrices dinámicas
		//rojo
		int** rojo = new int* [info_header.height];
		int** resr = new int* [info_header.height];
		//verde
		int** verde = new int* [info_header.height];
		int** resv = new int* [info_header.height];
		//azul
		int** azul = new int* [info_header.height];
		int** resa = new int* [info_header.height];

		//variables para concolucion
		int  mm, nn, ii, jj;
		int acumulador, acumuladorg, acumuladorb;
		int mitad = (filas * col) / 2;

		float** vfiltro2 = new float* [filas];
		for (int i = 0; i < filas; i++) {
			vfiltro2[i] = new float[filas];
		}
		for (int i = 0; i < filas; i++) {//filas
			for (int j = 0; j < col; j++) {//columnas
				vfiltro2[i][j] = fil[i][j];
			}
		}


		for (int i = 0; i < info_header.height; i++) {
			rojo[i] = new int[info_header.width];
			resr[i] = new int[info_header.width];
			//verde
			verde[i] = new int[info_header.width];
			resv[i] = new int[info_header.width];
			//azul
			azul[i] = new int[info_header.width];
			resa[i] = new int[info_header.width];
		}

		for (int i = 0; i < info_header.width; i++) {//filas
			for (int j = 0; j < info_header.height; j++) {//columnas
				//rojo
				rojo[i][j] = red[i * info_header.width + j];
				//verde
				verde[i][j] = green[i * info_header.width + j];
				//azul
				azul[i][j] = blue[i * info_header.width + j];
			}
		}

		//CONVOLUCIÓN
		for (int i = 0; i < info_header.width; i++) // Filas
		{
			for (int j = 0; j < info_header.height; j++) // Columnas
			{
				// Variable acumuladora
				acumulador = 0;
				acumuladorg = 0;
				acumuladorb = 0;

				for (int m = 0; m < filas; m++) // Filas del Kernel
				{
					mm = filas - 1 - m; // Indice de la fila del kernel alrevez

					for (int n = 0; n < col; n++) // Columnas del kernel
					{
						nn = col - 1 - n; // Indice de la columna del kernel alrevez
						ii = i + (m - mitad);
						jj = j + (n - mitad);

						// validar limites de la imagen 00000
						if (ii >= 0 && ii < info_header.width && jj >= 0 && jj < info_header.height)
						{
							acumulador += rojo[ii][jj] * vfiltro2[mm][nn]; //R
							acumuladorg += verde[ii][jj] * vfiltro2[mm][nn];//G
							acumuladorb += azul[ii][jj] * vfiltro2[mm][nn];//B
						}
					}
				}
				resr[i][j] = acumulador;
				resv[i][j] = acumuladorg;
				resa[i][j] = acumuladorb;
			}
		}

		//Pintar imagen
		for (int i = 0; i < info_header.height; i++)
		{
			for (int j = 0; j < info_header.width; j++)
			{
					pixel(i, j, resr[j][i], resv[j][i], resa[j][i]);
			}
		}
		glFlush();
	}
	void Sobel() {
		crearvectorrgb();
		int resultado;
		//creacion matrices dinámicas
		//rojo
		int** rojo = new int* [info_header.height];
		int** versoberR = new int* [info_header.height];
		int** horsoberR = new int* [info_header.height];
		int** RessobelR = new int* [info_header.height];
		//verde
		int** verde = new int* [info_header.height];
		int** versoberG = new int* [info_header.height];
		int** horsoberG = new int* [info_header.height];
		int** RessobelG = new int* [info_header.height];
		//azul
		int** azul = new int* [info_header.height];
		int** versoberB = new int* [info_header.height];
		int** horsoberB = new int* [info_header.height];
		int** RessobelB = new int* [info_header.height];

		float sobelVert[3][3] = { {-1.0, 0.0, 1.0},{-2.0, 0.0, 2.0},{-1.0, 0.0, 1.0} };
		float sobelHor[3][3] = { {-1.0,-2.0,-1.0},{0.0,0.0,0.0},{1.0,2.0,1.0} };
		for (int i = 0; i < info_header.height; i++) {
			//Rojo
			rojo[i] = new int[info_header.width];
			versoberR[i] = new int[info_header.width];
			horsoberR[i] = new int[info_header.width];	
			RessobelR[i] = new int[info_header.width];
			//verde
			verde[i] = new int[info_header.width];
			versoberG[i] = new int[info_header.width];
			horsoberG[i] = new int[info_header.width];
			RessobelG[i] = new int[info_header.width];
			//azul
			azul[i] = new int[info_header.width];
			versoberB[i] = new int[info_header.width];
			horsoberB[i] = new int[info_header.width];
			RessobelB[i] = new int[info_header.width];
		}
		for (int i = 0; i < info_header.width; i++) {//filas
			for (int j = 0; j < info_header.height; j++) {//columnas
				//rojo
				rojo[i][j] = red[i * info_header.width + j];
				//verde
				verde[i][j] = green[i * info_header.width + j];
				//azul
				azul[i][j] = blue[i * info_header.width + j];
			}
		}
		//variables para convolucion
		int  mm1, nn1, ii1, jj1;
		int acumulador1, acumuladorg1, acumuladorb1;
		int  mm2, nn2, ii2, jj2;
		int acumulador2, acumuladorg2, acumuladorb2;
		int mitad = 9 / 2;

		//CONVOLUCIÓN 1 
		for (int i = 0; i < info_header.width; i++) // Filas
		{
			for (int j = 0; j < info_header.height; j++) // Columnas
			{
				// Variable acumuladora
				acumulador1 = 0;
				acumuladorg1 = 0;
				acumuladorb1 = 0;

				for (int m = 0; m < 3; m++) // Filas del Kernel
				{
					mm1 = 3 - 1 - m; // Indice de la fila del kernel alrevez

					for (int n = 0; n < 3; n++) // Columnas del kernel
					{
						nn1 = 3 - 1 - n; // Indice de la columna del kernel alrevez
						ii1 = i + (m - mitad);
						jj1 = j + (n - mitad);

						// validar limites de la imagen 00000
						if (ii1 >= 0 && ii1 < info_header.width && jj1 >= 0 && jj1 < info_header.height)
						{
							acumulador1 += rojo[ii1][jj1] * sobelVert[mm1][nn1]; //R
							acumuladorg1 += verde[ii1][jj1] * sobelVert[mm1][nn1];//G
							acumuladorb1 += azul[ii1][jj1] * sobelVert[mm1][nn1];//B
						}
					}
				}
				versoberR[i][j] = acumulador1;
				versoberG[i][j] = acumuladorg1;
				versoberB[i][j] = acumuladorb1;
			}
		}
		//CONVOLUCIÓN 2 
		for (int i = 0; i < info_header.width; i++) // Filas
		{
			for (int j = 0; j < info_header.height; j++) // Columnas
			{
				// Variable acumuladora
				acumulador2 = 0;
				acumuladorg2 = 0;
				acumuladorb2 = 0;

				for (int m = 0; m < 3; m++) // Filas del Kernel
				{
					mm2 = 3 - 1 - m; // Indice de la fila del kernel alrevez

					for (int n = 0; n < 3; n++) // Columnas del kernel
					{
						nn2 = 3 - 1 - n; // Indice de la columna del kernel alrevez
						ii2 = i + (m - mitad);
						jj2 = j + (n - mitad);

						// validar limites de la imagen 00000
						if (ii2 >= 0 && ii2 < info_header.width && jj2 >= 0 && jj2 < info_header.height)
						{
							acumulador2 += rojo[ii1][jj1] * sobelHor[mm2][nn2]; //R
							acumuladorg2 += verde[ii1][jj1] * sobelHor[mm2][nn2];//G
							acumuladorb2 += azul[ii1][jj1] * sobelHor[mm2][nn2];//B
						}
					}
				}
				horsoberR[i][j] = acumulador2;
				horsoberG[i][j] = acumuladorg2;
				horsoberB[i][j] = acumuladorb2;
			}
		}

		for (int i = 0; i < info_header.width; i++) {//filas
			for (int j = 0; j < info_header.height; j++) {//columnas
				if (versoberR[i][j] > horsoberR[i][j])
				{
					RessobelR[i][j] = versoberR[i][j];
				}
				else
				{
					RessobelR[i][j] =horsoberR[i][j];
				}
				if (versoberG[i][j] > horsoberG[i][j])
				{
					RessobelG[i][j] = versoberG[i][j];
				}
				else
				{
					RessobelG[i][j] = horsoberG[i][j];
				}
				if (versoberB[i][j] > horsoberB[i][j])
				{
					RessobelB[i][j] = versoberB[i][j];
				}
				else
				{
					RessobelB[i][j] = horsoberG[i][j];
				}
			}
		}
		//Pintar imagen
		for (int i = 0; i < info_header.height; i++)
		{
			for (int j = 0; j < info_header.width; j++)
			{
				pixel(j, i, RessobelR[i][j], RessobelR[i][j], RessobelR[i][j]);
			}
		}
		glFlush();
	}
	void filtrosminmax(float fil[3][3], int opcionfil) {
		crearvectorrgb();
		int resultado;
		float auxfil[3][3];

		//creacion matrices dinamicas
		int** rojo = new int* [info_header.height];
		int** resr = new int* [info_header.height];
		//verde
		int** verde = new int* [info_header.height];
		int** resv = new int* [info_header.height];
		//azul
		int** azul = new int* [info_header.height];
		int** resa = new int* [info_header.height];

		int acumuladorR, acumuladorG, acumuladorB;
		int minormaxR, minormaxG, minormaxB;
		int mitad = 9 / 2, mm, nn, ii, jj;


		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				auxfil[i][j] = fil[i][j];
			}
		}
		float** vfiltro2 = new float* [3];
		for (int i = 0; i < 3; i++) {
			vfiltro2[i] = new float[3];
		}
		for (int i = 0; i < 3; i++) {//filas
			for (int j = 0; j < 3; j++) {//columnas
				vfiltro2[i][j] = auxfil[i][j];
				/*cout << "matriz azul [" << i << "][" << j << "]" << azul[i][j]<<endl;*/
			}
		}


		for (int i = 0; i < info_header.height; i++) {
			rojo[i] = new int[info_header.width];
			resr[i] = new int[info_header.width];
			//verde
			verde[i] = new int[info_header.width];
			resv[i] = new int[info_header.width];
			//azul
			azul[i] = new int[info_header.width];
			resa[i] = new int[info_header.width];
		}

		for (int i = 0; i < info_header.width; i++) {//filas
			for (int j = 0; j < info_header.height; j++) {//columnas
				rojo[i][j] = red[i * info_header.width + j];
				verde[i][j] = green[i * info_header.width + j];
				azul[i][j] = blue[i * info_header.width + j];
			}
		}

		for (int i = 0; i < info_header.width; i++) // Filas
		{
			for (int j = 0; j < info_header.height; j++) // Columnas
			{
				// Variable acumuladora
				acumuladorR = 0;
				acumuladorG = 0;
				acumuladorB = 0;


				//maximos
				if (opcionfil == 1) {
					minormaxR = 0;
					minormaxG = 0;
					minormaxB = 0;
				}
				//minimos
				else if (opcionfil == 2) {
					minormaxR = 255;
					minormaxG = 255;
					minormaxB = 255;
				}


				for (int m = 0; m < 3; m++) // Filas del Kernel
				{
					mm = 3 - 1 - m; // Indice de la fila del kernel alrevez

					for (int n = 0; n < 3; n++) // Columnas del kernel
					{
						nn = 3 - 1 - n; // Indice de la columna del kernel alrevez
						ii = i + (m - mitad);
						jj = j + (n - mitad);

						// validar limites de la imagen 00000
						if (ii >= 0 && ii < info_header.width && jj >= 0 && jj < info_header.height)
						{
							if (opcionfil == 1) {
								acumuladorR = rojo[ii][jj] * vfiltro2[mm][nn];
								if (acumuladorR > minormaxR)
								{
									minormaxR = acumuladorR;
								}
								acumuladorG = verde[ii][jj] * vfiltro2[mm][nn];
								if (acumuladorG > minormaxG)
								{
									minormaxG = acumuladorG;
								}
								acumuladorB = azul[ii][jj] * vfiltro2[mm][nn];
								if (acumuladorB > minormaxB)
								{
									minormaxB = acumuladorB;
								}
							}
							else if (opcionfil == 2) {
								acumuladorR = rojo[ii][jj] * vfiltro2[mm][nn];
								if (acumuladorR < minormaxR)
								{
									minormaxR = acumuladorR;

								}
								acumuladorG = verde[ii][jj] * vfiltro2[mm][nn];
								if (acumuladorG < minormaxG)
								{
									minormaxG = acumuladorG;
								}
								acumuladorB = azul[ii][jj] * vfiltro2[mm][nn];
								if (acumuladorB < minormaxB)
								{
									minormaxB = acumuladorB;
								}
							}
						}
					}
				}

				resr[i][j] = minormaxR;
				resv[i][j] = minormaxG;
				resa[i][j] = minormaxB;

			}
		}

		//Pintar imagen
		for (int i = 0; i < info_header.height; i++)
		{
			for (int j = 0; j < info_header.width; j++)
			{
				pixel(i, j, resr[j][i], resv[j][i], resa[j][i]);
			}
		}
		glFlush();
	}
	void filtroGaussiano(float sigma, int fil, int col) {
		float** filtrogaussdin = new float* [col];
		for (int i = 0; i < fil; i++) {
			filtrogaussdin[i] = new float[fil];
		}
		double r, s = 2.0 * pow(sigma, 2);
		double sum = 0.0;
		for (int x = 0; x < fil; x++) {

			for (int y = 0; y < col; y++) {
				r = pow(x, 2) + pow(y, 2);
				filtrogaussdin[x][y] = (exp(-(r) / s)) / (M_PI * s);
				sum += filtrogaussdin[x][y];
			}
		}
		for (int i = 0; i < fil; i++)
			for (int j = 0; j < col; j++)
				filtrogaussdin[i][j] /= sum;

		Filtros(fil, col, filtrogaussdin, false, 0);
	}

	vector<int> getRow(int rowIndex)
	{
		vector<int> currow;

		// 1st element of every row is 1
		currow.push_back(1);

		// Check if the row that has to be returned is the first row
		if (rowIndex == 0)
		{
			return currow;
		}

		// Generate the previous row
		vector<int> prev = getRow(rowIndex - 1);

		for (int i = 1; i < prev.size(); i++)
		{
			// Generate the elements of the current row by the help of the previous row
			int curr = prev[i - 1] + prev[i];
			currow.push_back(curr);
		}
		currow.push_back(1);

		// Return the row
		return currow;
	}
	void filtrobinomial(int n) {
		//cout << "holi" << endl;
		vector<int> arr = getRow(n);
		int** marr = new int* [n];
		float** filtro = new float* [n];
		int sum = 0, sum2 = 0;
		for (int i = 0; i <= n; i++) {
			marr[i] = new int[n];
			filtro[i] = new float[n];
		}

		for (int i = 0; i < 1; i++) {//filas
			for (int j = 0; j <= n; j++) {//columnas
				marr[i][j] = arr[j];
				cout << marr[i][j] << " ";
			}
		}

		for (int i = 1; i <= n; i++) {//filas
			for (int j = 0; j <= n; j++) {//columnas
				if (j == 0) {
					marr[i][j] = arr[i];
				}
				else {
					marr[i][j] = marr[0][j] * marr[i][0];
				}
			}
		}
		for (int i = 0; i <= n; i++) {//filas
			for (int j = 0; j <= n; j++) {//columnas
				//cout << marr[i][j] << " ";
				sum += marr[i][j];
			}
			//cout << endl;
		}
		for (int i = 0; i <= n; i++) {//filas
			for (int j = 0; j <= n; j++) {//columnas
				filtro[i][j] = marr[i][j] * pow(sum, -1);
			}
		}
		Filtros(n, n, filtro, false, 0);
	}
	void filtrodeteccionbordes() {

		int opdeteccion = 0; string sdeteccion;
		float** filtrobordesdin = new float* [3];
		float** filtrobordesdin2 = new float* [3];
		float** filtrobordesdin3 = new float* [3];

		float filtrobordes[3][3] = { {0,1,0},{1,-4,1},{0,1,0} };
		float filtrobordes2[3][3] = { {1,1,1},{1,-8,1},{1,1,1} };
		float filtrobordes3[3][3] = { {1, 1, 1},{2, -10, 2},{1, 1, 1} };
		//float filtrobordes3[3][3] = { {-1, 0, 1},{-2, 0, 2},{-1, 0, 1} };

		for (int i = 0; i < 3; i++) {
			filtrobordesdin[i] = new float[3];
			filtrobordesdin2[i] = new float[3];
			filtrobordesdin3[i] = new float[3];
			for (int j = 0; j < 3; j++) {
				filtrobordesdin[i][j] = filtrobordes[i][j];
				filtrobordesdin2[i][j] = filtrobordes2[i][j];
				filtrobordesdin3[i][j] = filtrobordes3[i][j];
			}
		}
		cout << "Elige entre estas 3 opciones de filtros para detectar bordes" << endl;
		cout << "1. Filtro Laplaciano" << endl;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				cout << filtrobordesdin[i][j] << " ";
			}
			cout << endl;
		}
		cout << "2. Filtro Laplaciano alternativo con bordes en 1" << endl;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				cout << filtrobordesdin2[i][j] << " ";
			}
			cout << endl;
		}
		cout << "3. Otro filtro deteccion de bordes" << endl;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				cout << filtrobordesdin3[i][j] << " ";
			}
			cout << endl;
		}

		do {
			cout << "Digite la opcion: ";
			cin >> opdeteccion;
			cout << "\n";

			switch (opdeteccion) {
			case 1:
				Filtros(3, 3, filtrobordesdin, false, 0);
				break;
			case 2:
				Filtros(3, 3, filtrobordesdin2, false, 0);
				break;
			case 3:
				Filtros(3, 3, filtrobordesdin3, false, 0);
				break;
			}
			cout << "\nQuiere probar con otra matriz?: ";
			cin >> sdeteccion;
			cout << "\n";

		} while (sdeteccion == "Si" || sdeteccion == "si" || sdeteccion == "SI");

	}

private:
	uint32_t row_stride{ 0 };
	// Add 1 to the row_stride until it is divisible with align_stride
	uint32_t make_stride_aligned(uint32_t align_stride) {
		uint32_t new_stride = row_stride;
		while (new_stride % align_stride != 0) {
			new_stride++;
		}
		return new_stride;
	}
};

BMP imagenprueba("lenna1.bmp");
void display2()
{
	imagenprueba.pintarhisto();
}
void display3()
{
	imagenprueba.histequalizado();
}
void menuhisto(int opcion)
{
	switch (opcion)
	{
	case 1:
		glutDestroyWindow(window);
		break;
	}
}
void ventanaecua()
{
	glutInitWindowSize(500, 300);
	glutInitWindowPosition(800, 100);
	window = glutCreateWindow("Histograma ecualizado");
	glutDisplayFunc(display3);
	gluOrtho2D(-10, 260, -10, 500);
	int opc = glutCreateMenu(menuhisto);
	glutAddMenuEntry("Salir", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
void ventanahisto()
{
	glutInitWindowSize(500, 300);
	glutInitWindowPosition(800, 100);
	window = glutCreateWindow("Histograma");
	glutDisplayFunc(display2);
	gluOrtho2D(-10, 260, -10, 20000);
	int opc = glutCreateMenu(menuhisto);
	glutAddMenuEntry("Salir", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

float filtroid[3][3] = { {1,1,1},{1,1,1},{1,1,1} };
float valsigma = 0.0f; int tamker = 0; string s;
void displaymenu(int op)
{

	switch (op)
	{
	case 1:
		glClear(GL_COLOR_BUFFER_BIT);
		imagenprueba.dibujarimg();
		break;
	case 2:
		//BRILLO----
		system("cls");
		glClear(GL_COLOR_BUFFER_BIT);
		cout << "\n\tDigite el nivel de brillo a aumentar: ";
		cin >> kb;
		cout << endl;
		imagenprueba.subirbrillo();
		break;
	case 3:
		//CONTRASTE
		system("cls");
		glClear(GL_COLOR_BUFFER_BIT);
		cout << "\n\tDigite el nivel de contraste a aumentar: ";
		cin >> kc;
		cout << endl;
		imagenprueba.contraste();
		break;
	case 4:
		glClear(GL_COLOR_BUFFER_BIT);
		//CONTRASTE AUTOMATICO
		imagenprueba.contrasteauto();
		break;
	case 5:
		glClear(GL_COLOR_BUFFER_BIT);
		//NEGATIVO
		imagenprueba.negativo();
		break;
	case 6:
		system("cls");
		glClear(GL_COLOR_BUFFER_BIT);
		cout << "\n\tDigite el valor de gamma (0 a 2): ";
		cin >> gamma;
		cout << endl;
		glClear(GL_COLOR_BUFFER_BIT);
		imagenprueba.trasnformacionGamma();
		break;
	case 7:
		ventanahisto();
		break;
	case 8:
		ventanaecua();
		break;
	case 10:
		glClear(GL_COLOR_BUFFER_BIT);
		imagenprueba.filtrosminmax(filtroid, 1);
		break;
	case 11:
		glClear(GL_COLOR_BUFFER_BIT);
		imagenprueba.filtrosminmax(filtroid, 2);
		break;

	case 12:
		exit(0);
		break;
	}
}

void displaymenuconvolucion(int op)
{
	float** filtroenfoquedin = new float* [3];
	float** filtropromdin = new float* [3];

	float filtroenfoque[3][3] = { {0, -1, 0},{-1, 5, -1},{0,-1,0} };
	float filtroprom[3][3] = { {0.11, 0.11, 0.11},{0.11, 0.11, 0.11},{0.11, 0.11,0.11} };

	for (int i = 0; i < 3; i++) {
		filtroenfoquedin[i] = new float[3];
		filtropromdin[i] = new float[3];
		for (int j = 0; j < 3; j++) {
			filtroenfoquedin[i][j] = filtroenfoque[i][j];
			filtropromdin[i][j] = filtroprom[i][j];
		}
	}

	switch (op)
	{
	case 1: //Deteccion bordes
		glClear(GL_COLOR_BUFFER_BIT);
		imagenprueba.filtrodeteccionbordes();
		break;
	case 2: //Enfocar
		glClear(GL_COLOR_BUFFER_BIT);
		imagenprueba.Filtros(3, 3, filtroenfoquedin, false, 0);
		break;
	case 3:
		glClear(GL_COLOR_BUFFER_BIT);
		cout << "\nDigite el nivel del triangulo de Pascal: ";
		cin >> tamker;
		imagenprueba.filtrobinomial(tamker);

		break;
	case 4:
		glClear(GL_COLOR_BUFFER_BIT);
		imagenprueba.Filtros(3, 3, filtropromdin, false, 0);

		break;

	case 5:
		glClear(GL_COLOR_BUFFER_BIT);
		system("cls");
		cout << "\nIngrese el tamanio del Kernel: ";
		cin >> tamker;
		do {
			cout << "\nIngrese el valor de sigma: ";
			cin >> valsigma;
			cout << "\n";
			imagenprueba.filtroGaussiano(valsigma, tamker, tamker);
			cout << "\nQuiere probar con otro sigma?: ";
			cin >> s;
			cout << "\n";
		} while (s == "Si" || s == "si" || s == "SI");

		break;
	case 6:
		glClear(GL_COLOR_BUFFER_BIT);
		imagenprueba.Sobel();
		break;
	}
}

void display()
{
	imagenprueba.dibujarimg();
}
//Funcion principal MAIN
int main(int argc, char** argv)
{

	const int h = info_header.height;
	const int w = info_header.width;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(w, h);
	glutInitWindowPosition(50, 100);
	glutCreateWindow("Procesamiento imagen");
	glutDisplayFunc(display);


	int opc2 = glutCreateMenu(displaymenuconvolucion);
	glutAddMenuEntry("Deteccion de bordes", 1);
	glutAddMenuEntry("Enfocar", 2);
	glutAddMenuEntry("Filtro binomial", 3);
	glutAddMenuEntry("Filtro promedio", 4);
	glutAddMenuEntry("Filtro Gussiano", 5);
	glutAddMenuEntry("Filtro Sobel", 6);

	int opcion = glutCreateMenu(displaymenu);
	glutAddMenuEntry("Imagen original", 1);
	glutAddMenuEntry("Subir/bajar brillo", 2);
	glutAddMenuEntry("Subir/bajar contraste", 3);
	glutAddMenuEntry("Contraste automatico", 4);
	glutAddMenuEntry("Negativo", 5);
	glutAddMenuEntry("Transformacion gamma", 6);
	glutAddMenuEntry("Ver histograma", 7);
	glutAddMenuEntry("Ver histograma ecualizado", 8);
	glutAddSubMenu("Filtros de Convolucion", opc2);
	glutAddMenuEntry("Filtro maximo", 10);
	glutAddMenuEntry("Filtro minimo", 11);
	glutAddMenuEntry("Salir", 12);


	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glutMainLoop();
	/*system("pause");*/
	return 0;
}
