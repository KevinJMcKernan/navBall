// F.S.Hill Jr. "Computer Graphics Using Open-GL"
#include "RGBpixmap.h"

typedef unsigned short ushort;
typedef unsigned long ulong;
fstream inf; 

// BMP uses little-endian short.
ushort getShort() {
		char ic;
		ushort ip;
		inf.get(ic);
		ip = ic; 
		inf.get(ic);
		ip |= ((ushort)ic << 8); 
		return ip;
}

// BMP uses little-endian longs.
ulong getLong() {
		ulong ip = 0;
		char ic = 0;
		unsigned char uc = ic;
		inf.get(ic); uc = ic; ip = uc;
		inf.get(ic); uc = ic; ip |=((ulong)uc << 8);
		inf.get(ic); uc = ic; ip |=((ulong)uc << 16);
		inf.get(ic); uc = ic; ip |=((ulong)uc << 24);
		return ip;
	}

int RGBpixmap:: readBMPFile(string fname, bool hasAlpha) 
{  
	// Read into memory an mRGB image from an uncompressed BMP file.
	inf.open(fname.c_str(), ios::in|ios::binary); // must read binary!
	if(!inf) { cout << " can't open file: " << fname << endl; return 0; }
	int k, row, col, numPadBytes, nBytesInRow;

	// read header information
	char ch1, ch2;
	inf.get(ch1);
	inf.get(ch2);						//type: always 'BM'
	ulong fileSize      = getLong();
	ushort reserved1    = getShort();	// always 0
	ushort reserved2    = getShort();	// always 0 
	ulong offBits       = getLong();	// offset to image - unreliable
	ulong headerSize    = getLong();	// always 40
	ulong numCols       = getLong();	// number of columns in image
	ulong numRows       = getLong();	// number of rows in image
	ushort planes       = getShort();	// always 1 
	ushort bitsPerPixel = getShort();	//8 or 24; allow 24 here
	ulong compression   = getLong();	// must be 0 for uncompressed 
	ulong imageSize     = getLong();	// total bytes in image 
	ulong xPels         = getLong();	// always 0 
	ulong yPels         = getLong();	// always 0 
	ulong numLUTentries = getLong();	// 256 for 8 bit, otherwise 0 
	ulong impColors     = getLong();	// always 0 

	if(bitsPerPixel != 24) 
	{
		// error - must be a 24 bit uncompressed image
		cout << "not a 24 bit/pixelimage, or is compressed!\n";
		inf.close();
		return 0;
	} 

	nBytesInRow = ((3 * numCols + 3)/4) * 4;
	numPadBytes = nBytesInRow - 3 * numCols; 
	nRows = numRows; 
	nCols = numCols;
	pixel = new mRGB[nRows * nCols]; 
	if (!pixel)
		return 0; 

	long count = 0;
	char dum;
	for(row = 0; row < nRows; row++) 
	{
		for(col = 0; col < nCols; col++)
		{
			char r,g,b;
			inf.get(b);
			inf.get(g);
			inf.get(r); 
			pixel[count].r = r; 
			pixel[count].g = g;
			pixel[count].b = b;
			if ( (hasAlpha) && (r==-1) && (g==-1) && (b==-1) )
				pixel[count++].a = 0;
			else
				pixel[count++].a = 255;
		}
   		for(k = 0; k < numPadBytes ; k++) 
			inf >> dum;
	}
	inf.close();
	return 1; 
}

void RGBpixmap :: setTexture(GLuint textureName)
{
	glBindTexture(GL_TEXTURE_2D,textureName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nCols, nRows,0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
}
