/* This program is free software; you can redistribute it and/or modify
 * it under the terms of version 3 or later of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#ifndef TYPES3D_H
#define TYPES3D_H

typedef struct _BoardData3d BoardData3d;
typedef struct _Material Material;
typedef struct _Path Path;
typedef struct _Texture Texture;
typedef struct _GraphData GraphData;
typedef struct _OGLFont OGLFont;
typedef struct _TextureInfo TextureInfo;

#define DF_VARIABLE_OPACITY 1
#define DF_NO_ALPHA 2
#define DF_FULL_ALPHA 4

struct _Texture {
	unsigned int texID;
	int width;
	int height;
};

typedef struct _OglModel
{
	float* data;
	int dataLength;
	int modelUsesTexture;
} OglModel;

typedef struct _OglModelHolder
{
	OglModel background;
	OglModel boardBase;
	OglModel oddPoints, evenPoints;
	OglModel table;
	OglModel hinge;
	OglModel DC;
	OglModel moveIndicator;
	OglModel piece, pieceTop;
	OglModel dice;
	OglModel flagPole;
} OglModelHolder;

/* TODO: Tidy this up... */
void OglModelInit(OglModel* oglModel);
void OglModelAlloc(OglModel* oglModel);
void OglModelDraw(const OglModel* oglModel);

extern OglModel* curModel;
#define CALL_OGL(model, fun, ...) \
{ \
	curModel = &model;\
	OglModelInit(curModel);\
	fun(__VA_ARGS__);\
	OglModelAlloc(curModel);\
	fun(__VA_ARGS__); \
}

#define F_PI (float)G_PI
#define F_PI_2 (float)G_PI_2

typedef signed long  FT_Pos;

struct _OGLFont {
	unsigned int textGlyphs;	// Used in graph
	unsigned int AAglyphs;	// For Anti-aliasing
	FT_Pos advance;
	FT_Pos kern[10][10];
	float scale;
	float heightRatio;
	float height;
	float length;
	OglModel models[10];
};

typedef struct _Point3d {
	double data[3];
} Point3d;

#if defined(WIN32)
/* Need to set the callback calling convention for windows */
#define TESS_CALLBACK __stdcall
#else
#define TESS_CALLBACK
#endif

typedef struct _GArray		GArray;
typedef unsigned int GLenum;
typedef unsigned char GLboolean;

typedef struct _Contour {
	GArray* conPoints;
} Contour;

typedef struct _Vectoriser {
	GArray* contours;
	unsigned int numPoints;
} Vectoriser;

typedef struct _Tesselation {
	GArray* tessPoints;
	GLenum meshType;
} Tesselation;

typedef struct _Mesh {
	GArray* tesselations;
} Mesh;

typedef enum _displaytype {
    DT_2D, DT_3D
} displaytype;

typedef enum _lighttype {
    LT_POSITIONAL, LT_DIRECTIONAL
} lighttype;

typedef enum _PieceType {
    PT_ROUNDED, PT_FLAT
} PieceType;

typedef enum _PieceTextureType {
    PTT_TOP, PTT_BOTTOM, PTT_ALL, NUM_TEXTURE_TYPES
} PieceTextureType;

typedef enum _TextureType {
    TT_NONE = 1, TT_GENERAL = 2, TT_PIECE = 4, TT_HINGE = 8, TT_DISABLED = 16
} TextureType;
#define TT_COUNT 3              /* 3 texture types: general, piece and hinge */

typedef int idleFunc(BoardData3d * bd3d);

/* Work out which sides of dice to draw */
typedef struct _diceTest {
    int top, bottom, side[4];
} diceTest;

typedef struct _DiceRotation {
    float xRotStart, yRotStart;
    float xRot, yRot;
    float xRotFactor, yRotFactor;
} DiceRotation;

typedef struct _EigthPoints
{	/* Used for rounded corners */
	float*** points;
	unsigned int accuracy;
} EigthPoints;

extern void freeEigthPoints(EigthPoints* eigthPoints);
void calculateEigthPoints(EigthPoints* eigthPoints, float radius, unsigned int accuracy);
//void drawCornerEigth(const EigthPoints* eigthPoints, float radius, const Texture* texture);

/* Used to calculate correct viewarea for board/fov angles */
typedef struct _viewArea {
	float top;
	float bottom;
	float width;
} viewArea;

//TODO: Sort out (remove gluNurb?)
typedef struct GLUnurbs GLUnurbsObj;
typedef struct _Flag3d {
	/* Define nurbs surface - for flag */
	GLUnurbsObj* flagNurb;
#define S_NUMPOINTS 4
#define S_NUMKNOTS (S_NUMPOINTS * 2)
#define T_NUMPOINTS 2
#define T_NUMKNOTS (T_NUMPOINTS * 2)
	/* Control points for the flag. The Z values are modified to make it wave */
	float ctlpoints[S_NUMPOINTS][T_NUMPOINTS][3];
} Flag3d;

/* Font info */

#define FONT_PITCH 24
#define FONT_SIZE (base_unit / 20.0f)
#define FONT_HEIGHT_RATIO 1.f

#define CUBE_FONT_PITCH 34
#define CUBE_FONT_SIZE (base_unit / 24.0f)
#define CUBE_FONT_HEIGHT_RATIO 1.25f

#define FONT_VERA "fonts/Vera.ttf"
#define FONT_VERA_SERIF_BOLD "fonts/VeraSeBd.ttf"

#endif
