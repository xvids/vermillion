#pragma once

#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"GLu32.lib")

#include "main.h"

#include <gl/gl.h>
#include <gl/glu.h>

#define FL_NONE	0
#define FL_OUTLINE	2
#define FL_BACKDROP	4
#define FL_CENTER	8
#define FL_BOLD 16
#define FL_ALL	255

class cFont
{
public:
	void InitText( char* font , int height , int width );
	void Print( int x , int y , int r , int g , int b , int a , BYTE flags , char *strText );
	void PrintFont( int x , int y , int r , int g , int b , int a , BYTE flags , char *strText );
	int   fheight;
private:
	void	Render( int x , int y , int r , int g , int b , int a , char *string );
	short	cheight;
	int		width;
	short	cwidth[255];
	GLuint   g_FontListID;
};

extern cFont Verdana1;
extern cFont Verdana2;
extern cFont ArialBlack;